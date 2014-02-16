#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <set>

#include "storage.h"

swStorage::swStorage() : mPoints(NULL), mBoxes(NULL), mPointsCount(0), mBoxesCount(0), mPointsSize(1024), mBoxesSize(32), mInited(false)
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
            if (mBoxes[i].mCandidates)
                delete mBoxes[i].mCandidates;
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
        // printStorage(__func__, this);
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
                        mBoxes[mBoxesCount].mCandidates = new unordered_set<swPoint *>;
                        mBoxes[mBoxesCount].mPoints = new vector<swPoint *>;
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

#define BOX_MIN 0x0000000000000001UL
#define BOX_MAX 0x0000000000000002UL

struct coordinateCompareX
{
    bool operator() (const swCoordinate *lhs, const swCoordinate *rhs) const
    {
        swCoordinate *realLhs = (swCoordinate *)((uint64_t)(lhs) & ~(BOX_MIN | BOX_MAX));
        swCoordinate *realRhs = (swCoordinate *)((uint64_t)(rhs) & ~(BOX_MIN | BOX_MAX));
        if (realLhs->mX < realRhs->mX)
            return true;
        if (realLhs->mX > realRhs->mX)
            return false;
        if ((((uint64_t)(lhs) & BOX_MIN) && !((uint64_t)(rhs) & BOX_MIN)) || ((lhs == realLhs) && ((uint64_t)(rhs) & BOX_MAX)))
            return true;
        return false;
    }
};

struct coordinateCompareY
{
    bool operator() (const swCoordinate *lhs, const swCoordinate *rhs) const
    {
        swCoordinate *realLhs = (swCoordinate *)((uint64_t)(lhs) & ~(BOX_MIN | BOX_MAX));
        swCoordinate *realRhs = (swCoordinate *)((uint64_t)(rhs) & ~(BOX_MIN | BOX_MAX));
        if (realLhs->mY < realRhs->mY)
            return true;
        if (realLhs->mY > realRhs->mY)
            return false;
        if ((((uint64_t)(lhs) & BOX_MIN) && !((uint64_t)(rhs) & BOX_MIN)) || ((lhs == realLhs) && ((uint64_t)(rhs) & BOX_MAX)))
            return true;
        return false;
    }
};

/*
static void printX(set<swCoordinate *, coordinateCompareX> &xSet)
{
    printf ("ordered X coordinates: ");
    for (set<swCoordinate *, coordinateCompareX>::iterator iter = xSet.begin(); iter != xSet.end(); iter++)
    {
        swCoordinate *coord = *iter;
        // point
        if (!((uint64_t)coord & (BOX_MIN | BOX_MAX)))
        {
            printf("%f, ", coord->mX);
        }
        // min X of the box
        else if ((uint64_t)coord & BOX_MIN)
            printf("%f(MIN), ", ((swCoordinate *)((uint64_t)(coord) & ~BOX_MIN))->mX);
        // max X of the box
        else
            printf("%f(MAX), ", ((swCoordinate *)((uint64_t)(coord) & ~BOX_MAX))->mX);
    }
    printf ("\n");
}

static void printY(set<swCoordinate *, coordinateCompareY> &ySet)
{
    printf ("ordered Y coordinates: ");
    for (set<swCoordinate *, coordinateCompareY>::iterator iter = ySet.begin(); iter != ySet.end(); iter++)
    {
        swCoordinate *coord = *iter;
        // point
        if (!((uint64_t)coord & (BOX_MIN | BOX_MAX)))
        {
            printf("%f, ", coord->mY);
        }
        // min X of the box
        else if ((uint64_t)coord & BOX_MIN)
            printf("%f(MIN), ", ((swCoordinate *)((uint64_t)(coord) & ~BOX_MIN))->mY);
        // max X of the box
        else
            printf("%f(MAX), ", ((swCoordinate *)((uint64_t)(coord) & ~BOX_MAX))->mY);
    }
    printf ("\n");
}
*/

bool swStorage::findPointsInBoxes()
{
    set<swCoordinate *, coordinateCompareX> xSet;
    set<swCoordinate *, coordinateCompareY> ySet;

    // inset boxes coordinates in the both sets
    for (size_t i = 0; i < mBoxesCount; i++)
    {
        xSet.insert((swCoordinate *)((uint64_t)(&mBoxes[i].mMinPoint) | BOX_MIN));
        ySet.insert((swCoordinate *)((uint64_t)(&mBoxes[i].mMinPoint) | BOX_MIN));
        xSet.insert((swCoordinate *)((uint64_t)(&mBoxes[i].mMaxPoint) | BOX_MAX));
        ySet.insert((swCoordinate *)((uint64_t)(&mBoxes[i].mMaxPoint) | BOX_MAX));
    }

    // identify boxes that have minimum, maximum X and Y of all boxes
    set<swCoordinate *, coordinateCompareX>::iterator xMinIter = xSet.begin();
    set<swCoordinate *, coordinateCompareX>::reverse_iterator xMaxIter = xSet.rbegin();
    set<swCoordinate *, coordinateCompareY>::iterator yMinIter = ySet.begin();
    set<swCoordinate *, coordinateCompareY>::reverse_iterator yMaxIter = ySet.rbegin();

    // swBox *boxMinX = (swBox *)(((uint64_t)(*xMinIter) & ~BOX_MIN) - offsetof(swBox, mMinPoint));
    swBox *boxMaxX = (swBox *)(((uint64_t)(*xMaxIter) & ~BOX_MAX) - offsetof(swBox, mMaxPoint));
    // swBox *boxMinY = (swBox *)(((uint64_t)(*yMinIter) & ~BOX_MIN) - offsetof(swBox, mMinPoint));
    swBox *boxMaxY = (swBox *)(((uint64_t)(*yMaxIter) & ~BOX_MAX) - offsetof(swBox, mMaxPoint));

    for (size_t i = 0; i < mPointsCount; i++)
        xSet.insert(&(mPoints[i].mCoordinate));

    set<swCoordinate *, coordinateCompareX>::iterator xIter = xMinIter;
    bool maxValueNotFound = true;
    set<swBox *> boxSet;
    while (maxValueNotFound)
    {
        swCoordinate *coord = *xIter;
        // point
        if (!((uint64_t)coord & (BOX_MIN | BOX_MAX)))
        {
            if (!boxSet.empty())
            {
                swPoint *point = (swPoint *)coord;
                for (set<swBox *>::iterator boxIter = boxSet.begin(); boxIter != boxSet.end(); boxIter++)
                    (*boxIter)->mCandidates->insert(point);
                ySet.insert(coord);
            }
        }
        // min X of the box
        else if ((uint64_t)coord & BOX_MIN)
        {
            swBox *box = (swBox *)(((uint64_t)(coord) & ~BOX_MIN) - offsetof(swBox, mMinPoint));
            boxSet.insert(box);
        }
        // max X of the box
        else
        {
            swBox *box = (swBox *)(((uint64_t)(coord) & ~BOX_MAX) - offsetof(swBox, mMaxPoint));
            boxSet.erase(box);
            if (box == boxMaxX)
                maxValueNotFound = false;
        }
        xIter++;
    }

    if (!boxSet.empty())
        printf ("!!! ERROR !!!: box set should be empty\n");

    set<swCoordinate *, coordinateCompareY>::iterator yIter = yMinIter;
    maxValueNotFound = true;
    while (maxValueNotFound)
    {
        swCoordinate *coord = *yIter;
        // point
        if (!((uint64_t)coord & (BOX_MIN | BOX_MAX)))
        {
            if (!boxSet.empty())
            {
                swPoint *point = (swPoint *)coord;
                for (set<swBox *>::iterator boxIter = boxSet.begin(); boxIter != boxSet.end(); boxIter++)
                {
                    if ((*boxIter)->mCandidates->find(point) != (*boxIter)->mCandidates->end())
                        (*boxIter)->mPoints->push_back(point);
                }
            }
        }
        // min Y of the box
        else if ((uint64_t)coord & BOX_MIN)
        {
            swBox *box = (swBox *)(((uint64_t)(coord) & ~BOX_MIN) - offsetof(swBox, mMinPoint));
            boxSet.insert(box);
        }
        // max Y of the box
        else
        {
            swBox *box = (swBox *)(((uint64_t)(coord) & ~BOX_MAX) - offsetof(swBox, mMaxPoint));
            boxSet.erase(box);
            if (box == boxMaxY)
                maxValueNotFound = false;
        }
        yIter++;
    }

    return true;
}

struct coordinateCompareXAlt
{
    bool operator() (const swCoordinate *lhs, const swCoordinate *rhs) const
    {
        return lhs->mX < rhs->mX;
    }
};

struct coordinateCompareYAlt
{
    bool operator() (const swCoordinate *lhs, const swCoordinate *rhs) const
    {
        return lhs->mY < rhs->mY;
    }
};


bool swStorage::findPointsInBoxesAlt()
{
    set<swCoordinate *, coordinateCompareXAlt> xSet;
    set<swCoordinate *, coordinateCompareYAlt> ySet;

    for (size_t i = 0; i < mPointsCount; i++)
    {
        xSet.insert(&(mPoints[i].mCoordinate));
        // ySet.insert(&(mPoints[i].mCoordinate));
    }

    for (size_t i = 0; i < mBoxesCount; i++)
    {
        set<swCoordinate *, coordinateCompareXAlt>::iterator xLowerIter, xUpperIter;
        xLowerIter = xSet.lower_bound(&(mBoxes[i].mMinPoint));
        xUpperIter = xSet.upper_bound(&(mBoxes[i].mMaxPoint));
        while (xLowerIter != xUpperIter)
        {
            ySet.insert(*xLowerIter);
            xLowerIter++;
        }

        set<swCoordinate *, coordinateCompareYAlt>::iterator yLowerIter, yUpperIter;
        yLowerIter = ySet.lower_bound(&(mBoxes[i].mMinPoint));
        yUpperIter = ySet.upper_bound(&(mBoxes[i].mMaxPoint));
        while (yLowerIter != yUpperIter)
        {
            mBoxes[i].mPoints->push_back((swPoint *)(*yLowerIter));
            yLowerIter++;
        }
        ySet.clear();
    }
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
    for (size_t i = 0; i < mBoxesCount; i++)
        printBox(&mBoxes[i]);
    return;
}
