#include "storage.h"

swStorage::swStorage()
{
}

static size_t skipWhiteSpace(char *data, size_t offset, size_t size)
{
    while (offset < size)
    {
        if ((data[offset] == ' ') || (data[offset] == '\t') || (data[offset] == '\n'))
            offset++;
        else
            break;
    }
    return offset;
}

size_t swStorage::parsePoint(char *data, size_t offset, size_t size)
{
    size_t rtn = 0;
    if (data && offset && size)
    {
        swPoint *point = new swPoint();
        double x = 0.0, y = 0.0;
    }
    return rtn;
}

size_t swStorage::parseBox(char *data, size_t offset, size_t size)
{
    size_t rtn = 0;
    if (data && offset && size)
    {
        swBox *point = new swBox();
        double minX = 0.0, minY = 0.0, maxX = 0.0, maxY = 0.0;
    }
    return rtn;
}

bool swStorage::parse(char *data, size_t size)
{
    bool rtn = false;
    if (data && size)
    {
        size_t offset = 0;
        while (offset < size)
        {
            if (data[offset] == 'p')
            {
                offset = parsePoint(data, offset+5, size);
            }
            else if (data[offset] == 'b')
            {
                offset = parseBox(data, offset+3, size);
            }
        }
    }
    return rtn;
}

bool swStorage::findPointsInBoxes()
{

}

void swStorage::printBoxes()
{

}
