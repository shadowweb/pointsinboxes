#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <set>

#include "storage.h"

swStorage::swStorage() : mPoints(NULL), mBoxes(NULL), mPointsCount(0), mBoxesCount(0), mPointsSize(1024), mBoxesSize(32), mInited(false)
{
    if ((mPoints = (swPoint *)malloc(sizeof(swPoint) * mPointsSize)))
    {
        if ((mBoxes = (swBox *)malloc(sizeof(mBoxes) * mBoxesSize)))
            mInited = true;
    }
}

swStorage::~swStorage()
{
    if (mBoxes)
    {
        for (size_t i = 0; i < mBoxesCount; i++)
        {
            if (mBoxes[i].mPoints)
                delete mBoxes[i].mPoints;
        }
        free(mBoxes);
    }
    if (mPoints)
        free(mPoints);
}

static inline size_t skipWhiteSpace(char *data, size_t offset, size_t size)
{
    while (offset < size)
    {
        if (isspace(data[offset]))
            offset++;
        else
            break;
    }
    return offset;
}

static inline size_t readFloat(char *data, size_t offset, size_t size, double *out)
{
    size_t rtn = 0;
    if ((offset = skipWhiteSpace(data, offset, size)) < size)
    {
        size_t readOffset = offset;
        while ((offset < size) && !isspace(data[offset]))
            offset++;
        if (sscanf(&data[readOffset], "%lf", out))
            rtn = offset;
    }
    return rtn;
}

static inline size_t readUUID(char *data, size_t offset, size_t size, uuid_t uuid)
{
    size_t rtn = 0;
    if ((offset = skipWhiteSpace(data, offset, size)) < size)
    {
        size_t readOffset = offset;
        while ((offset < size) && !isspace(data[offset]))
            offset++;

        if ((offset - readOffset) == 36)
        {
            char saveCharacter = data[offset];
            data[offset] = '\0';
            if (uuid_parse(&data[readOffset], uuid) == 0)
                rtn = offset;
            data[offset] = saveCharacter;
        }
    }
    return rtn;
}

size_t swStorage::parsePoint(char *data, size_t offset, size_t size)
{
    size_t rtn = 0;
    if (data && size && mInited)
    {
        if ((offset = readFloat(data, offset, size, &(mPoints[mPointsCount].mCoordinate.mX))) && (offset < size))
        {
            if ((offset = readFloat(data, offset, size, &(mPoints[mPointsCount].mCoordinate.mY))) && (offset < size))
            {
                if ((offset = readUUID(data, offset, size, mPoints[mPointsCount].mId)) && (offset < size))
                {
                    mPointsCount++;
                    if (mPointsCount < mPointsSize)
                        rtn = offset;
                    else
                    {
                        swPoint *newPoints = (swPoint *)realloc(mPoints, sizeof(swPoint) * mPointsSize * 2);
                        if (newPoints)
                        {
                            mPoints = newPoints;
                            mPointsSize *= 2;
                            rtn = offset;
                        }
                    }
                }
            }
        }
    }
    return rtn;
}

#define swapForMax(a, b) if ((a) > (b)) { double tmp = (a); (a) = (b); (b) = tmp; }

size_t swStorage::parseBox(char *data, size_t offset, size_t size)
{
    size_t rtn = 0;
    if (data && size && mInited)
    {
        if ((offset = readFloat(data, offset, size, &(mBoxes[mBoxesCount].mMinPoint.mX))) && (offset < size))
        {
            if ((offset = readFloat(data, offset, size, &(mBoxes[mBoxesCount].mMinPoint.mY))) && (offset < size))
            {
                if ((offset = readFloat(data, offset, size, &(mBoxes[mBoxesCount].mMaxPoint.mX))) && (offset < size))
                {
                    if ((offset = readFloat(data, offset, size, &(mBoxes[mBoxesCount].mMaxPoint.mY))) && (offset < size))
                    {
                        swapForMax(mBoxes[mBoxesCount].mMinPoint.mX, mBoxes[mBoxesCount].mMaxPoint.mX);
                        swapForMax(mBoxes[mBoxesCount].mMinPoint.mY, mBoxes[mBoxesCount].mMaxPoint.mY);
                        mBoxes[mBoxesCount].mPoints = NULL;
                        mBoxesCount++;
                        if (mBoxesCount < mBoxesSize)
                            rtn = offset;
                        else
                        {
                            swBox *newBoxes = (swBox *)realloc(mBoxes, sizeof(swBox) * mBoxesSize * 2);
                            if (newBoxes)
                            {
                                mBoxes = newBoxes;
                                mBoxesSize *= 2;
                                rtn = offset;
                            }
                        }
                    }
                }
            }
        }
    }
    return rtn;
}

bool swStorage::parse(char *data, size_t size)
{
    bool rtn = false;
    if (data && size)
    {
        size_t offset = 0;
        while ((offset = skipWhiteSpace(data, offset, size)) < size)
        {
            if (data[offset] == 'p')
            {
                if ((offset = parsePoint(data, offset+5, size)))
                    continue;
            }
            else if (data[offset] == 'b')
            {
                if ((offset = parseBox(data, offset+3, size)))
                    continue;
            }
        }
        if (offset >= size)
            rtn = true;
    }
    return rtn;
}

#define BOX_MIN 0x01
#define BOX_MAX 0x02

// TODO: finish this
struct coordinateCompare
{
    bool operator() (const swCoordinate *lhs, const swCoordinate *rhs) const
    {
        swCoordinate *realLhs = lhs & ~(BOX_MIN && BOX_MAX);
        swCoordinate *realRhs = rhs & ~(BOX_MIN && BOX_MAX);
        if (realLhs < realRhs)
            return true;
        if (realLhs == realRhs)
        {
            if (lhs & BOX_MIN)
                return ((rhs & BOX_MIN)? false : true);
            else if (lhs & BOX_MAX)
                return false;
        }
        return false;
    }
};

set<swCoordinate *, coordinateCompare> s;


bool swStorage::findPointsInBoxes()
{
    set<swCoordinate *, coordinateCompare> xSet;
    set<swCoordinate *, coordinateCompare> ySet;
    return true;
}

void swStorage::printPoint(swPoint *point)
{
    if (point)
    {
        char uuidString[37] = {0};
        uuid_unparse(point->mId, uuidString);
        printf("point %f %f %s\n", point->mCoordinate.mX, point->mCoordinate.mY, uuidString);
    }
}

void swStorage::printBox(swBox *box)
{
    if (box)
    {
        printf("box %f %f %f %f\n", box->mMinPoint.mX, box->mMinPoint.mY, box->mMaxPoint.mX, box->mMaxPoint.mY);
        if (box->mPoints)
        {
            for (vector<swPoint *>::iterator it = box->mPoints->begin(); it != box->mPoints->end(); it++)
                printPoint(*it);
        }
    }
}

void swStorage::printBoxes()
{
    for (size_t i = 0; i < mPointsCount; i++)
        printPoint(&mPoints[i]);
    for (size_t i = 0; i < mBoxesCount; i++)
        printBox(&mBoxes[i]);
    return;
}
