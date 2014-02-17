#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <set>

#include "storage.h"

swStorage::swStorage(size_t points, size_t boxes) : mPoints(NULL), mBoxes(NULL), mPointsCount(0), mBoxesCount(0), mPointsSize(points), mBoxesSize(boxes), mInited(false)
{
    if ((mPoints = (swPoint *)malloc(sizeof(swPoint) * mPointsSize)))
    {
        if ((mBoxes = (swBox *)malloc(sizeof(swBox) * mBoxesSize)))
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

static inline char *skipWhiteSpace(char *ptr, char *endPtr)
{
    while (ptr < endPtr)
    {
        if (isspace(*ptr))
            ptr++;
        else
            break;
    }
    return ptr;
}

static inline char *readFloat(char *ptr, char *endPtr, double *out)
{
    char *rtn = NULL;
    if ((ptr = skipWhiteSpace(ptr, endPtr)) < endPtr)
    {
        char *readPtr = ptr;
        while ((ptr < endPtr) && !isspace(*ptr))
            ptr++;
        if (sscanf(readPtr, "%lf", out))
            rtn = ptr;
    }
    return rtn;
}

static inline char *readUUID(char *ptr, char *endPtr, uuid_t uuid)
{
    char *rtn = NULL;
    if ((ptr = skipWhiteSpace(ptr, endPtr)) < endPtr)
    {
        char *readPtr = ptr;
        while ((ptr < endPtr) && !isspace(*ptr))
            ptr++;

        if ((ptr - readPtr) == 36)
        {
            char saveCharacter = *ptr;
            *ptr = '\0';
            if (uuid_parse(readPtr, uuid) == 0)
                rtn = ptr;
            *ptr = saveCharacter;
        }
    }
    return rtn;
}

char *swStorage::parsePoint(char *ptr, char *endPtr)
{
    char *rtn = NULL;
    if (ptr && endPtr && mInited)
    {
        if ((ptr = readFloat(ptr, endPtr, &(mPoints[mPointsCount].mCoordinate.mX))) && (ptr < endPtr))
        {
            if ((ptr = readFloat(ptr, endPtr, &(mPoints[mPointsCount].mCoordinate.mY))) && (ptr < endPtr))
            {
                if ((ptr = readUUID(ptr, endPtr, mPoints[mPointsCount].mId)) && (ptr < endPtr))
                {
                    mPointsCount++;
                    if (mPointsCount < mPointsSize)
                        rtn = ptr;
                    else
                    {
                        fprintf(stderr, "point realloc\n");
                        swPoint *newPoints = (swPoint *)realloc(mPoints, sizeof(swPoint) * mPointsSize * 2);
                        if (newPoints)
                        {
                            mPoints = newPoints;
                            mPointsSize *= 2;
                            rtn = ptr;
                        }
                    }
                }
            }
        }
    }
    return rtn;
}

#define swapForMax(a, b) if ((a) > (b)) { double tmp = (a); (a) = (b); (b) = tmp; }

char *swStorage::parseBox(char *ptr, char *endPtr)
{
    char *rtn = 0;
    if (ptr && endPtr && mInited)
    {
        if ((ptr = readFloat(ptr, endPtr, &(mBoxes[mBoxesCount].mMinPoint.mX))) && (ptr < endPtr))
        {
            if ((ptr = readFloat(ptr, endPtr, &(mBoxes[mBoxesCount].mMinPoint.mY))) && (ptr < endPtr))
            {
                if ((ptr = readFloat(ptr, endPtr, &(mBoxes[mBoxesCount].mMaxPoint.mX))) && (ptr < endPtr))
                {
                    if ((ptr = readFloat(ptr, endPtr, &(mBoxes[mBoxesCount].mMaxPoint.mY))) && (ptr < endPtr))
                    {
                        swapForMax(mBoxes[mBoxesCount].mMinPoint.mX, mBoxes[mBoxesCount].mMaxPoint.mX);
                        swapForMax(mBoxes[mBoxesCount].mMinPoint.mY, mBoxes[mBoxesCount].mMaxPoint.mY);
                        mBoxes[mBoxesCount].mPoints = new vector<swPoint *>;
                        mBoxesCount++;
                        if (mBoxesCount < mBoxesSize)
                            rtn = ptr;
                        else
                        {
                            fprintf(stderr, "box realloc\n");
                            swBox *newBoxes = (swBox *)realloc(mBoxes, sizeof(swBox) * mBoxesSize * 2);
                            if (newBoxes)
                            {
                                mBoxes = newBoxes;
                                mBoxesSize *= 2;
                                rtn = ptr;
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
        char *ptr = data;
        char *endPtr = data + size;
        while ((ptr = skipWhiteSpace(ptr, endPtr)) < endPtr)
        {
            if (*ptr == 'p')
            {
                if ((ptr = parsePoint(ptr + 5, endPtr)))
                    continue;
            }
            else if (*ptr == 'b')
            {
                if ((ptr = parseBox(ptr+3, endPtr)))
                    continue;
            }
        }
        if (ptr >= endPtr)
            rtn = true;
    }
    return rtn;
}

struct coordinateCompareX
{
    bool operator() (const swCoordinate *lhs, const swCoordinate *rhs) const
    {
        return (lhs->mX < rhs->mX);
    }
};

bool swStorage::findPointsInBoxes()
{
    multiset<swCoordinate *, coordinateCompareX> xSet;

    for (size_t i = 0; i < mPointsCount; i++)
        xSet.insert(&(mPoints[i].mCoordinate));

    for (size_t i = 0; i < mBoxesCount; i++)
    {
        multiset<swCoordinate *, coordinateCompareX>::iterator xLowerIter, xUpperIter;
        xLowerIter = xSet.lower_bound(&(mBoxes[i].mMinPoint));
        xUpperIter = xSet.upper_bound(&(mBoxes[i].mMaxPoint));
        while (xLowerIter != xUpperIter)
        {
            if (((*xLowerIter)->mY >= mBoxes[i].mMinPoint.mY) && ((*xLowerIter)->mY <= mBoxes[i].mMaxPoint.mY))
                mBoxes[i].mPoints->push_back((swPoint *)(*xLowerIter));
            xLowerIter++;
        }
    }
    return true;
}

void swStorage::printPoint(swPoint *point)
{
    if (point)
    {
        char uuidString[37] = {0};
        uuid_unparse(point->mId, uuidString);
        // printf("point %f %f %s\n", point->mCoordinate.mX, point->mCoordinate.mY, uuidString);
        printf("%s\n", uuidString);
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
    for (size_t i = 0; i < mBoxesCount; i++)
        printBox(&mBoxes[i]);
    return;
}
