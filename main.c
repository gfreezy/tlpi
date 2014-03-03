#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <glib-2.0/glib.h>
#include "tlpi_hdr.h"
#include "util.h"
#include "list.h"
#include "sds.h"


#define LINE_SIZE 200

typedef struct Process {
    sds name;
    long pid;
    long tgid;
    long ppid;
    int threads;
} Process;

typedef struct Tuple {
    gconstpointer a;
    gconstpointer b;
} Tuple;

int process_status(const char *process_id, const char* status, char *value, const size_t size);
int process_cmd_name(const char*, char*, const size_t);
long long process_parent_id(const char *process_id);


int process_cmd_name(const char *process_id, char *cmd, const size_t size) {
    char value[LINE_SIZE];
    if ( -1 == process_status(process_id, "Name", value, LINE_SIZE)) {
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
    
    size_t line_max = LINE_SIZE;
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
        
        trim_whitespace(token);
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


long long process_parent_id(const char *process_id) {
    char parent_id[LINE_SIZE] = {0};
    if (-1 == process_status(process_id, "PPid", parent_id, LINE_SIZE)) {
        errMsg("get PPid error");
        return -1;
    }
    assert(parent_id != NULL);
    return strtoll(parent_id, NULL, 10);
}


void list_processes(GSList **processes) {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        errExit("open /proc");
    }
    
    struct dirent *ent;
    size_t line_size = LINE_SIZE;
    char line[LINE_SIZE] = {0};
    int n;
    Process *p;
    while ((ent = readdir(dir)) != NULL) {
        if (!strisdigit(ent->d_name)) {
            continue;
        }

        p = malloc(sizeof(Process));
        p->pid = strtol(ent->d_name, NULL, 10);
        if (-1 == process_status(ent->d_name, "PPid", line, line_size)) {
            errMsg("get PPid error");
            free(p);
            continue;
        }
        p->ppid = strtol(line, NULL, 10);
        
        if (-1 == process_status(ent->d_name, "Tgid", line, line_size)) {
            errMsg("get tgid error");
            free(p);
            continue;
        }

        p->tgid = strtol(line, NULL, 10);

        if (-1 == process_status(ent->d_name, "Name", line, line_size)) {
            errMsg("get name error");
            free(p);
            continue;
        }
        
        sds name = sdsnew(line);
        sdstrim(name, " \t\n");
        p->name = name;

        *processes = g_slist_prepend(*processes, p);
    }
    closedir(dir);
}

gint compare(gconstpointer a, gconstpointer b) {
    return ((Process *)a)->pid - ((Process *)b)->pid;
}

gint compare_process_ppid(gconstpointer a, gconstpointer b) {
    return ((Process *)a)->ppid - ((Process *)b)->ppid;
}

void print_processes(gpointer data, gpointer user_data) {
    Process *p = (Process *)data;
    printf("Name: %s\tPid: %ld\tPPid: %ld\tTgid: %ld\n", p->name, p->pid, p->ppid, p->tgid);
}

void free_process(gpointer data) {
    free(data);
}

GSList *find_children(Process *parent, GSList *candiates) {
    Process p, *match;
    p.ppid = parent->pid;
    match = g_slist_find_custom(children, &p, compare_process_ppid);
    return match;
}

void add_nodes_to_tree(GNode *parent, GSList *children, GSList *candiates) {
    assert(candiates != NULL);
    assert(parent != NULL);
    assert(children != NULL);
    Tuple tuple = {parent, candiates};
    g_slist_foreach(children, add_node_to_tree, &tuple);
}

void add_node_to_tree(Process *child, Tuple *tuple) {
    assert(tuple != NULL);
    assert(child != NULL);
    assert(parent != NULL);
    GNode *parent = (GNode *)tuple->a;
    GSList *candiates = (GSList *)tuple->b;
    GNode *child_node = g_node_append_data(parent, child);
    Tuple t = {child_node, candiates};
    GSList *match = find_children(child, candiates);
    if (!match) {
        return;
    }
    add_nodes_to_tree(child_node, match, candiates);
}


int main(int argc, char *argv[]) {
    GSList *processes = NULL, *match = NULL;
    Process *p = malloc(sizeof(Process));
    p->ppid = 0;
    p->pid = 0;
    GNode *root = g_node_new(p);
    
    list_processes(&processes);
    processes = g_slist_sort(processes, compare);
    g_slist_foreach(processes, print_processes, root);
    

    free(p);
    g_slist_free_full(processes, free_process);
    exit(EXIT_SUCCESS);
}