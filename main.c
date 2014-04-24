#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include <math.h>
#include "tlpi_hdr.h"
#include "util.h"


#ifndef FILENAME_LENGTH
#define FILENAME_LENGTH 6
#endif


int createFile(const char *filename) {
    int fd = open(filename, O_CREAT|O_WRONLY|O_EXCL);
    if (fd == -1) {
        errMsg("create %s error", filename);
        return -1;
    }
    ssize_t byteWritten;
    if (-1 == (byteWritten = write(fd, "1", 1))) {
        errMsg("write one byte error");
        if (-1 == close(fd)) {
            errExit("close file %s error", filename);
        }
        return -1;
    }

    if (-1 == close(fd)) {
        errExit("close file %s error", filename);
    }
    return 0;
}


int deleteFile(const char *filename) {
    if (-1 == unlink(filename)) {
        errMsg("delete file %s error", filename);
        return -1;
    }
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        usageErr("%s dir numberOfFiles", argv[0]);
    }

    int numOfFiles = getInt(argv[2], GN_ANY_BASE | GN_GT_0, "numberOfFiles");

    char *path = malloc(PATH_MAX);
    if (realpath(argv[1], path) == NULL) {
        errExit("get abs path error");
    }
    DIR *dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "%s is not a valid dir\n", path);
        usageErr("%s dir numberOfFiles", argv[0]);
    }

    int i = 0, randNum, filePostfix;
    char filename[FILENAME_MAX];
    srand(time(NULL));
    while (i < numOfFiles) {
        randNum = rand();
        filePostfix = randNum % ((int)pow(10, FILENAME_LENGTH) -1);
        snprintf(filename, FILENAME_MAX, "%s/x%.6d", path, filePostfix);
        // fprintf(stderr, "%s\n", filename);
        if (-1 == createFile(filename)) {
            continue;
        }
        i++;
    }

    rewinddir(dir);

    struct dirent *ent;
    while((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG && strstr(ent->d_name, "x") != NULL) {
            snprintf(filename, FILENAME_MAX, "%s/%s", path, ent->d_name);
            // fprintf(stderr, "delete %s\n", filename);
            if (-1 == deleteFile(filename)) {
                // remove file error
            }
        }
    }

    free(path);
    exit(EXIT_SUCCESS);
}
