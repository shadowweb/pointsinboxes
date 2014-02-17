#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <stdint.h>
#include <uuid/uuid.h>

#include <unordered_set>
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
    // create storage object
    swStorage(size_t points, size_t boxes);
    // destroy storage object
    ~swStorage();
    // parse points and boxes data from the buffer
    bool parse(char *data, size_t size);
    // find points that belong to the boxes
    bool findPointsInBoxes();
    // print the boxes information to stdout
    void printBoxes();
    // check if everything inited correctly
    bool isInited() { return mInited; }
private:
    // parse point from the buffer starting from the given offset
    char *parsePoint(char *ptr, char *endPtr);
    // parse box from the buffer starting from the given offset
    char *parseBox(char *ptr, char *endPtr);
    // print point to stdout
    void printPoint(swPoint *point);
    // print box to stdout
    void printBox(swBox *box);

    swPoint *mPoints;
    swBox *mBoxes;
    size_t mPointsCount;
    size_t mBoxesCount;
    size_t mPointsSize;
    size_t mBoxesSize;
    unsigned mInited : 1;

};

#endif