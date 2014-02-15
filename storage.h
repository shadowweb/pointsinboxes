#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <stdint.h>
#include <uuid/uuid.h>

#include <vector>

using namespace std;

typedef struct swCoordinate
{
    double mX;
    double mY;
} swCoordinate;

typedef struct swPoint
{
    swCoordinate mCoordinate;
    uuid_t mId;
} swPoint;

typedef struct swBox
{
    swCoordinate mMinPoint;
    swCoordinate mMaxPoint;
    vector<swPoint *> *mPoints;
} swBox;

class swStorage
{
public:
    swStorage();
    ~swStorage();
    bool parse(char *data, size_t size);
    bool findPointsInBoxes();
    void printBoxes();
    swPoint *mPoints;
    swBox *mBoxes;
    size_t mPointsCount;
    size_t mBoxesCount;
    size_t mPointsSize;
    size_t mBoxesSize;
    unsigned mInited : 1;
private:
    size_t parsePoint(char *data, size_t offset, size_t size);
    size_t parseBox(char *data, size_t offset, size_t size);
    void printPoint(swPoint *point);
    void printBox(swBox *box);
};

#endif