//
//  main.c
//  linux
//
//  Created by Chao Fei on 12/18/13.
//  Copyright (c) 2013 Chao Fei. All rights reserved.
//
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <setjmp.h>
#include "tlpi_hdr.h"


extern char **environ;


int setenv(const char *name, const char *value, int overwrite)
{
    char *old_value = getenv(name);
    if (old_value && !overwrite) {
        return 0;
    }
    char *name_ = strdup(name);
    char *value_ = strdup(value);
    
    size_t size = strlen(name) + strlen(value) + 2;
    char *new = malloc(size);
    if(snprintf(new, size, "%s=%s", name_, value_) < 0)
    {
        errExit("snprintf");
    }
    return putenv(new) < 0 ? -1 : 0;
}

int unsetenv(const char* name) 
{
    char **old_env = environ;
    environ = NULL;
    
    size_t env_size = 0;
    char **ep, **ep2;
    int ret;
    for(ep=old_env; *ep != NULL; ep++)
    {
        env_size++;
    }

    char **env = malloc(env_size * sizeof(*old_env));
    for(ep=old_env, ep2=env; *ep != NULL; ep++, ep2++)
    {
        *ep2 = *ep;
    }
    
    for(ep=env; *ep!=NULL; ep++)
    {
        if(strncmp(name, (const char*)*ep, strlen(name)) != 0)
        {
            if(putenv(strdup(*ep)) != 0)
            {
                return -1;
            }
        }
    }
    free(env);
    return 0;
}


void print_environ()
{
    char **ep;
    for(ep=environ; *ep!=NULL; ep++)
    {
        printf("%s\n", *ep);
    }
}

int main(int argc, const char * argv[])
{
    print_environ();
    
    printf("after set environ\n");
    setenv("en1", "va1", 0);
    setenv("en1", "va2", 1);
    print_environ();
    
    printf("after clear\n");
    unsetenv("en1");
    print_environ();
    exit(EXIT_SUCCESS);
}