#ifndef TPLI_HDR_H
#define TPLI_HDR_H
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "get_num.h"
#include "error_functions.h"

typedef enum { FALSE, TRUE } Boolean;

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#endif
