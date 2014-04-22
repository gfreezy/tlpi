#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#include "lib/tlpi_hdr.h"
#include "util.h"


#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif

#define LINE_NUMBER 10


char *splitLines(const char *buf, char **pos) {
//    parameters:
//      buf: string to split
//      pos: NULL to begin a new split.
//    return:
//      line one by one
//      NULL represents no more lines.
//      returned pointer memory will be overrited in the following call.
//    
//    newline characters are not stripped
    if (buf == NULL) {
        return NULL;
    }
    static char line[LINE_MAX];
    memset(line, 0, LINE_MAX);
    size_t size = strlen(buf);
    if (*pos == NULL) {
        *pos = (char *)buf;
    }
    if (*pos >= buf + size) {
        return NULL;
    }
    char *lineBegin = *pos;
    char *nextEnd = NULL;
    char cur, next;
    size_t len;
    while (*pos < buf + size) {
        cur = **pos;
        next = *(*pos + 1);

        if ((cur == '\n' || cur == '\r') && (next != '\n' && next != '\r')) {
            nextEnd = *pos;
        } else if (next == '\0') {
            nextEnd = *pos;
        }
        (*pos)++;
        
        if (nextEnd) {
            len = nextEnd - lineBegin + 1;
            if (LINE_MAX < len) {
                return NULL;
            }
            memcpy(line, lineBegin, len);
            return line;
        }
    }
    
    return NULL;    
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        usageErr("tail [-f lineNum] file");
    }
    int c;
    char *filename = NULL, *strLineNum = NULL;
    while((c=getopt(argc, argv, "n:")) != -1) {
        switch(c) {
        case 'n':
            if (optind != argc) {
                strLineNum = optarg;
            } else {
                errExit("tail [-f lineNum] file");
            }
        }
    }
    filename = argv[argc - 1];
    int lineNum = LINE_NUMBER;
    if (strLineNum) {
        lineNum = (int)strtol(strLineNum, NULL, 10);
        if (lineNum == 0 && errno) {
            usageErr("tail [-f lineNum] file");
        }
    }
    
//    printf("filename: %s, lineNum: %d\n", filename, lineNum);
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        errExit("file %s cannot be open", filename);
    }
    
    int product = 1;
    size_t bufSize = BUF_SIZE;
    char *buf = malloc(bufSize + 1);
    ssize_t bytesRead = 0;
    char *line = NULL;
    char *pos = NULL;
    size_t lineCount = 0;
    
    do {
        if (-1 == lseek(fd, -bufSize, SEEK_END)) {
            errExit("seek end error");
        }
        if (-1 == (bytesRead = read(fd, buf, bufSize))) {
            errExit("read error");
        }
        buf[bytesRead] = '\0';

        lineCount = 0;        
        pos = NULL;
        while((line = splitLines(buf, &pos)) != NULL) {
            lineCount++;
        }        
        
//        printf("line Count: %u\n", lineCount);
        if (lineCount > lineNum) {
            break;
        }
        product++;
        buf = realloc(buf, product * BUF_SIZE + 1);
    } while (1);

    pos = NULL;    
    int l;
    for (l=1; (line = splitLines(buf, &pos)) != NULL; l++) {
        if (l > lineCount - lineNum) {
            printf("%s", line);
        }
    }           
    
    free(buf);
    if (-1 == close(fd)) {
        errExit("close file");
    }
    exit(EXIT_SUCCESS);
}
