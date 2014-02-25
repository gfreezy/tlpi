#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include "tlpi_hdr.h"

#define LINE_MAX 200


int getCmdName(const char *processID, char **cmdline, size_t *line_max) {
    char path[PATH_MAX];
    int n = snprintf(path, PATH_MAX, "/proc/%s/cmdline", processID);
    if (n == -1) {
        errExit("concat path");
    }
    assert(n <= PATH_MAX);
    
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        errMsg("open file error: %s", path);
        return -1;
    }
    
    ssize_t len;
    errno = 0;
    if ((len = getline(cmdline, line_max, fd)) == -1 && errno) {
        errMsg("read cmdline content error");
        return -1;
    }
    return 0;
}

uid_t userIdFromName(const char *name) {
    if (name == NULL || *name == '\0') {
        return -1;
    }
    struct passwd *pwd = getpwnam(name);
    if (pwd == NULL) {
        return -1;
    }
    return pwd->pw_uid;
}


long long processEid(const char *processId) {
    char path[PATH_MAX];
    int n;
    n = snprintf(path, PATH_MAX, "/proc/%s/status", processId);
    if (n == -1) {
        errExit("concat path");
    }
    assert(n <= PATH_MAX);
    
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        errMsg("open error");
        return -1;
    }
    
    size_t line_max = LINE_MAX;
    char *line = malloc(line_max);
    char *p;
    long long eid = -1;
    while ((n = getline(&line, &line_max, fd)) != -1) {
        p = strtok(line, " \t");
        if (strncmp(p, "Uid:", 4) != 0) {
            continue;
        }
        // jump over real user id
        p = strtok(NULL, "\t ");
        assert(p != NULL);
        
        p = strtok(NULL, " \t");
        assert(p != NULL);
        eid = strtoll(p, NULL, 10);
        break;
    }
    free(line);
    return eid;
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


void listProcessCmdline(uid_t uid) {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        errExit("open /proc");
    }
    
    struct dirent *ent;
    char *processId;
    size_t cmdline_max = LINE_MAX;
    char *cmdline = malloc(cmdline_max);
    while ((ent = readdir(dir)) != NULL) {
        if (!strisdigit(ent->d_name)) {
            continue;
        }
        processId = ent->d_name;
        long long n = processEid(processId);
        if (n == -1) {
            printf("Get process %s's eid error\n", processId);
            continue;
        }
        if ((uid_t)n == uid) {
            n = getCmdName(processId, &cmdline, &cmdline_max);
            if (n == -1) {
                printf("Get process %s's cmdline error\n", processId);
                continue;
            }
            
            printf("Process ID: %s\t", processId);
            printf("Cmdline: %s\n", cmdline);
        }
    }
    free(cmdline);
    closedir(dir);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        cmdLineErr("%s username\n", argv[0]);
    }
    char *username = argv[1];
    uid_t uid = userIdFromName(username);
    if (uid == -1) {
        errMsg("User %s not exist\n", username);
        exit(EXIT_FAILURE);
    }
    listProcessCmdline(uid);
    exit(EXIT_SUCCESS);
}