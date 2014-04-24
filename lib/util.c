#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "util.h"
#ifndef LINE_MAX
#define LINE_MAX 1024
#endif


void trim_whitespace(char* string) {
    if (!string || string == '\0') {
        return;
    }
    /* First remove leading spaces */
    const char* firstNonSpace = string;

    while (*firstNonSpace != '\0' && isspace(*firstNonSpace)) {
        ++firstNonSpace;
    }

    size_t len = strlen(firstNonSpace) + 1;

    memmove(string, firstNonSpace, len);

    /* Now remove trailing spaces */

    char* endOfString = string + len;

    while (string < endOfString && isspace(*endOfString)) {
        --endOfString;
    }

    *endOfString = '\0';

}


int strisdigit(char *str) {
    char *s = str;
    while(*s != '\0') {
        if (!isdigit(*s)) {
            return 0;
        }
        s++;
    }
    return 1;
}


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
    static char staticMemory[LINE_MAX];
    char *line = staticMemory;
    memset(line, 0, LINE_MAX);
    size_t size = strlen(buf);
    if (*pos == NULL) {
        *pos = (char *)buf;
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
