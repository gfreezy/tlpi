#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include "tlpi_hdr.h"
#include "util.h"

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif


int main(int argc, char *argv[]) {
    int inputFd, outputFd, openFlags;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE];
    
    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1) {
        errExit("open %s error", argv[1]);
    }
    
    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH;
    outputFd = open(argv[2], openFlags, filePerms);
    if (outputFd == -1) {
        errExit("open %s error", argv[2]);
    }
    
    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0) {
        if (write(outputFd, buf, numRead) < numRead) {
            fatal("write error");
        }
    }
    if (numRead == -1) {
        errExit("read file error");
    }
    if (close(inputFd) == -1) {
        errExit("close file");
    }
    if (close(outputFd) == -1) {
        errExit("close file");
    }
    exit(EXIT_SUCCESS);
}