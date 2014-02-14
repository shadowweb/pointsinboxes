#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <uuid/uuid.h>

#include <vector>

using namespace std;

typedef struct swPoint
{
    double mX;
    double mY;
    uuid_t mId;
} swPoint;

typedef struct swBox
{
    swPoint mMinPoint;
    swPoint mMaxPoint;
    vector<swPoint *> mPoints;
} swBox;

class swStorage
{
public:
    swStorage();
    bool parse(char *data, size_t size);
    bool findPointsInBoxes();
    void printBoxes();
    vector<swPoint *> mPoints;
    vector<swBox *> mBoxes;
private:
    size_t parsePoint(char *data, size_t offset, size_t size);
    size_t parseBox(char *data, size_t offset, size_t size);
};

#endif