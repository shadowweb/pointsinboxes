#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
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
            void *mmappedData = mmap(NULL, fileSize, PROT_READ, (MAP_PRIVATE | MAP_POPULATE), fd, 0);
            if (mmappedData)
            {
                printf("%s", (char *)mmappedData);
                swStorage *storage = new swStorage();
                if (storage)
                {
                    if (storage->mInited)
                    {
                        if (storage->findPointsInBoxes())
                        {
                            storage->printBoxes();
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