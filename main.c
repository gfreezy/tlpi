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

void print_process(gpointer data, gpointer user_data) {
    Process *p = (Process *)data;
    printf("Name: %s\tPid: %ld\tPPid: %ld\tTgid: %ld\n", p->name, p->pid, p->ppid, p->tgid);
}

void free_process(gpointer data) {
    free(data);
}

GSList *find_children(Process *parent, GSList *candiates) {
    assert(parent != NULL);
    assert(candiates != NULL);
    Process p;
    GSList *match;
    p.ppid = parent->pid;
    match = g_slist_find_custom(candiates, &p, compare_process_ppid);
    return match;
}

void add_node_to_tree(Process *child_process, GNode *parent_node, GSList *candiates) {
    assert(child_process != NULL);
    assert(parent_node != NULL);
    assert(candiates != NULL);
    Process *parent_process = (Process *)parent_node->data;
    GNode *child_node = g_node_append_data(parent_node, child_process);
    
    GSList *match = NULL;
    GSList *src = candiates;
    Process *p = NULL;
    while (NULL != (match = find_children(child_process, src))) {
        p = (Process *)match->data;
        add_node_to_tree(p, child_node, candiates);
        if (NULL == (src = g_slist_next(match))) {
            break;
        }
    }
}


gboolean print_tree(GNode *node, gpointer user_data) {
    Process *p = (Process *)node->data;
    if (!p) {
        return;
    }
    print_process(p, NULL);
}

void print_indent(char c, int n) {
    int i = 0;
    for (i = 0; i < n; i++) {
        printf("%c", c);
    }
}

gboolean print_node(GNode *node, sds *preindent) {
    assert(NULL != node);
    Process *p = (Process *)node->data;
    
    if (p) {
        GNode *prev_node = g_node_prev_sibling(node);
        GNode *next_node = g_node_next_sibling(node);
        if (!prev_node && next_node) {
            *preindent = sdscat(*preindent, " | ");
            printf("-┬-");
        }
        else if (prev_node && next_node) {
            *preindent = sdscat(*preindent, " | ");
            printf(" ├-");
        } else {
            *preindent = sdscat(*preindent, "   ");
            printf(" └-");
        }
        *preindent = sdscat(*preindent, "      ");
        printf("%6ld", p->pid);
    }
    GNode *first_child = g_node_first_child(node);
    GNode *next_sibling = NULL;
    sds next_indent;
    if (first_child) {
        next_indent = sdsdup(*preindent);
        print_node(first_child, &next_indent);
        sdsfree(next_indent);
        while (NULL != (next_sibling = g_node_next_sibling(first_child))) {
            printf("\n");
            printf("%s", *preindent);
            
            next_indent = sdsdup(*preindent);
            print_node(next_sibling, &next_indent);
            sdsfree(next_indent);
            first_child = next_sibling;
        }
    }
}

int main(int argc, char *argv[]) {
    GSList *processes = NULL, *match = NULL;
    Process *p = malloc(sizeof(Process));
    p->name = sdsnew("+");
    p->ppid = 0;
    p->pid = 0;
    GNode *root = g_node_new(NULL);
    
    list_processes(&processes);
    processes = g_slist_sort(processes, compare);
    g_slist_foreach(processes, print_process, root);
    
    add_node_to_tree(p, root, processes);
    g_node_traverse(root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, print_tree, NULL);
    
    sds indent = sdsnew("");
    print_node(root, &indent);
    sdsfree(indent);
    
    g_node_destroy(root);
    free(p);
    g_slist_free_full(processes, free_process);
    exit(EXIT_SUCCESS);
}