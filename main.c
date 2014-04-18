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


void list_fds(const char *pathname) {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        errExit("open /proc");
    }
    
    struct dirent *ent, *ent2;
    char fd_dirname[FILENAME_MAX] = {0};
    char link[FILENAME_MAX] = {0};
    char link_path[FILENAME_MAX] = {0};
    DIR *fd_dir = NULL;
    char *pid = NULL, *fd = NULL;
    while ((ent = readdir(dir)) != NULL) {
        pid = ent->d_name;

        if (ent->d_type != DT_DIR || !strisdigit(pid)) {
            continue;
        }
        
        // find fds
        snprintf(fd_dirname, FILENAME_MAX, "/proc/%s/fd", pid);
        fd_dir = opendir(fd_dirname);
        if (fd_dir == NULL) {
            continue;
        }
        while ((ent2 = readdir(fd_dir)) != NULL) {
            if (ent2->d_type != DT_LNK) {
                continue;
            }
            fd = ent2->d_name;
            snprintf(link_path, FILENAME_MAX, "%s/%s", fd_dirname, fd);
            memset(link, 0, FILENAME_MAX);
            if (readlink(link_path, link, FILENAME_MAX) == -1) {
                errMsg("readlink error: %s", link_path);
                continue;
            }
            if (strncmp(pathname, link, strlen(link)) == 0) {
                printf("pid: %s\n", pid);
                break;
            }
        }
        
        closedir(fd_dir);
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        usageErr("%s pathname\n", argv[0]);
    }
    list_fds(argv[1]);
    exit(EXIT_SUCCESS);
}