#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <uuid/uuid.h>

#include "storage.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

size_t getFileSize(const char* filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

#define NSEC_IN_SEC 1000000000

uint64_t getCurrentTime()
{
    struct timespec now = {0, 0};
    clock_gettime(CLOCK_REALTIME, &now);
    return (uint64_t)now.tv_sec * NSEC_IN_SEC + now.tv_nsec;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        handle_error("Invalid number of argument\n");

    size_t fileSize = getFileSize(argv[1]);
    if (fileSize > 0)
    {
        int fd = open(argv[1], O_RDONLY, 0);
        if (fd > -1)
        {
            uint64_t startTime = getCurrentTime();
            char *mmappedData = (char *)mmap(NULL, fileSize, (PROT_READ | PROT_WRITE), (MAP_PRIVATE | MAP_POPULATE), fd, 0);
            if (mmappedData)
            {
                uint64_t mapTime = getCurrentTime();
                uint64_t diffTime = mapTime - startTime;
                fprintf (stderr, "Mapping file to memory took: %ld.%09ld seconds\n", diffTime/NSEC_IN_SEC, diffTime%NSEC_IN_SEC);
                swStorage *storage = new swStorage();
                if (storage)
                {
                    if (storage->mInited)
                    {
                        if (storage->parse(mmappedData, fileSize))
                        {
                            uint64_t parseTime = getCurrentTime();
                            diffTime = parseTime - mapTime;
                            fprintf (stderr, "Parsing file from memory took: %ld.%09ld seconds\n", diffTime/NSEC_IN_SEC, diffTime%NSEC_IN_SEC);
                            if (storage->findPointsInBoxes())
                            {
                                uint64_t processTime = getCurrentTime();
                                diffTime = processTime - parseTime;
                                fprintf (stderr, "Processing data took: %ld.%09ld seconds\n", diffTime/NSEC_IN_SEC, diffTime%NSEC_IN_SEC);
                                storage->printBoxes();
                                uint64_t endTime = getCurrentTime();
                                diffTime = endTime - processTime;
                                fprintf (stderr, "Printing results took: %ld.%09ld seconds\n", diffTime/NSEC_IN_SEC, diffTime%NSEC_IN_SEC);
                            }
                        }
                    }
                    delete storage;
                }
                if (munmap(mmappedData, fileSize) == -1)
                    handle_error("failed to munmap file\n");
            }
            else
                handle_error("failed to mmap file\n");
            close(fd);
        }
        else
            handle_error("failed to open file\n");
    }
    else
        handle_error("invalid file size\n");

    exit(EXIT_SUCCESS);
}