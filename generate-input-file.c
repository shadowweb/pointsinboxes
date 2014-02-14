#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <uuid/uuid.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

double minX = 0.0;
double maxX = 0.0;
double minY = 0.0;
double maxY = 0.0;

// this is from http://stackoverflow.com/questions/2704521/generate-random-double-numbers-in-c
double generateCoordinate(double minValue, double maxValue)
{
    double f = (double)rand() / RAND_MAX;
    return (minValue + f * (maxValue - minValue));
}

void generatePoint(FILE *out)
{
    uuid_t uuidValue = {0};
    uuid_generate_random(uuidValue);
    char uuidString[37];
    uuid_unparse(uuidValue, uuidString);
    if (fprintf(out, "point %f %f %s\n", generateCoordinate(minX, maxX), generateCoordinate(minY, maxY), uuidString) < 0)
        handle_error("Failed to write to the output stream\n");
}

void generateBox(FILE *out)
{
    if (fprintf(out, "box %f %f %f %f\n", generateCoordinate(minX, maxX), generateCoordinate(minY, maxY), generateCoordinate(minX, maxX), generateCoordinate(minY, maxY)) < 0)
        handle_error("Failed to write to the output stream\n");
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (argc < 7)
        handle_error("Invalid number of argument\n");

    int32_t x1 = atoi(argv[1]);
    int32_t y1 = atoi(argv[2]);
    int32_t x2 = atoi(argv[3]);
    int32_t y2 = atoi(argv[4]);
    printf ("x1 = %d, y1 = %d, x2 = %d, y2 = %d\n", x1, y1, x2, y2);

    uint32_t points = atoi(argv[5]);
    uint32_t boxes = atoi(argv[6]);

    minX = (double)((x1 < x2)? x1 : x2);
    maxX = (double)((x1 > x2)? x1 : x2);
    minY = (double)((y1 < y2)? y1 : y2);
    maxY = (double)((y1 > y2)? y1 : y2);
    printf ("doubles: minX = %f, minY = %f, maxX = %f, maxY = %f\n", minX, minY, maxX, maxY);

    if (minX < maxX && minY < maxY)
    {
        char *data = NULL;
        size_t size = 0;
        FILE *out = open_memstream(&data, &size);
        if (out)
        {
            for (uint32_t i = 0; i < points; i++)
                generatePoint(out);
            for (uint32_t i = 0; i < boxes; i++)
                generateBox(out);
            fclose(out);
            if (data && size)
            {
                FILE *outFile = fopen("./data.out", "w+");
                if (outFile)
                {
                    if (fwrite(data, size, 1, outFile) == 1)
                        fclose(outFile);
                    else
                        handle_error("Failed to write all the data to the output file\n");
                }
                free(data);
            }
            else
                handle_error("Generated empty buffer\n");
        }
    }
    else
        handle_error("Invalid input\n");

    exit(EXIT_SUCCESS);
}
