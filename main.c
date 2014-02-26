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

int process_status(const char *process_id, const char* status, char *value, const size_t size);

int getCmdline(const char *processID, char **cmdline, size_t *line_max) {
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


int getCmdName(const char *processID, char *cmd, const size_t size) {
    char value[LINE_MAX];
    if ( -1 == process_status(processID, "Name", value, LINE_MAX)) {
        errMsg("get status");
        return -1;
    }
    char *token = strtok(value, " \t\n");
    assert(token != NULL);
    if (size <= strlen(token)) {
        errMsg("not enough memroy");
        return -1;
    }
    strcpy(cmd, token);
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


int process_status(const char *process_id, const char* status, char *value, const size_t size) {
    char path[PATH_MAX];
    int n;
    n = snprintf(path, PATH_MAX, "/proc/%s/status", process_id);
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
    size_t value_size = 0;
    char *line = malloc(line_max);
    char *token;

    while ((n = getline(&line, &line_max, fd)) != -1) {
        token = strtok(line, ":");
        assert(token != NULL);
        if (strcmp(token, status) != 0) {
            continue;
        }
        token = strtok(NULL, ":");
        assert(token != NULL);
        
        if (size <= strlen(token)) {
            errMsg("allocated memory not enough");
            return -1;
        }
        strcpy(value, token);
        value_size = strlen(token);
        break;
    }
    free(line);
    return size;
}


long long processEid(const char *processId) {
    char uids[LINE_MAX];
    int n = process_status(processId, "Uid", uids, LINE_MAX);
    if (n == -1) {
        errMsg("get process uid error");
        return -1;
    }
    char *token;
    // jump over real user id
    token = strtok(uids, " \t");
    assert(token != NULL);
    token = strtok(NULL, " \t");
    assert(token != NULL);
    return strtoll(token, NULL, 10);
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


void listProcessCmdName(uid_t uid) {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        errExit("open /proc");
    }
    
    struct dirent *ent;
    char *processId;
    size_t cmdline_max = LINE_MAX;
    char *cmdline = malloc(cmdline_max);
    char cmd[LINE_MAX] = {0};
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
            n = getCmdline(processId, &cmdline, &cmdline_max);
            if (n == -1) {
                printf("Get process %s's cmdline error\n", processId);
                continue;
            }
            if (-1 == getCmdName(processId, cmd, LINE_MAX)) {
                printf("get cmd name error\n");
                continue;
            }
            
            printf("Process ID: %8s\t", processId);
            printf("Cmd name: %15s\t", cmd);
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
    listProcessCmdName(uid);
    exit(EXIT_SUCCESS);
}