#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#include "tlpi_hdr.h"


mode_t path_mode(const char *path) {
  struct stat sbuf;
  if (-1 == lstat(path, &sbuf))
  {
    errExit("stat");
  }
  return sbuf.st_mode;
}

char *realpath_dir(const char *pathname, char *resolved_path) {
  int cwd_fd;
  char realpath_[PATH_MAX];

  if (-1 == (cwd_fd = open(".", O_RDONLY))) {
    errMsg("open");
    return NULL;
  }

  if (-1 == chdir(pathname))
  {
    errMsg("chdir");
    return NULL;
  }

  if (NULL == getcwd(realpath_, PATH_MAX))
  {
    errMsg("getcwd");
    return NULL;
  }

  if (-1 == fchdir(cwd_fd))
  {
    errMsg("restore cwd");
    return NULL;
  }

  snprintf(resolved_path, PATH_MAX, "%s", realpath_);

  return resolved_path;
}


char *realpath2(const char *pathname, char *resolved_path) {
  mode_t mode = path_mode(pathname);
  char *pathname1 = strdup(pathname);
  char *pathname2 = strdup(pathname);

  char *dirpath = strdup(dirname(pathname1));
  char *filename = strdup(basename(pathname2));
  free(pathname1);
  free(pathname2);

  char link_buf[PATH_MAX];
  char resolved_dir[PATH_MAX];
  char *ret = NULL;
  ssize_t bytes_read;

  if (NULL == resolved_path) {
    resolved_path = malloc(PATH_MAX);
  }

  if (S_ISDIR(mode)) {
    ret = realpath_dir(pathname, resolved_path);

  } else if (S_ISLNK(mode)) {
    if (-1 == (bytes_read = readlink(pathname, link_buf, PATH_MAX))) {
      errMsg("readlink");
      goto ERROR;
    }
    if (bytes_read == PATH_MAX)
    {
      fprintf(stderr, "bytes_read == PATH_MAX");
      goto ERROR;
    }
    link_buf[bytes_read] = '\0';

    if (-1 == chdir(dirpath)) {
      goto ERROR;
    }
    ret = realpath2(link_buf, resolved_path);

  } else {
    if (NULL == realpath_dir(dirpath, resolved_dir)) {
      goto ERROR;
    }
    snprintf(resolved_path, PATH_MAX, "%s/%s", resolved_dir, filename);
    ret = resolved_path;
  }

ERROR:
  free(dirpath);
  free(filename);
  return ret;
}


int main(int argc, char const *argv[])
{
  if (argc < 2)
  {
    usageErr("%s path\n", argv[0]);
  }

  char path[PATH_MAX];
  if (NULL == realpath2(argv[1], path)) {
    errExit("realpath");
  }

  printf("old: %s\nreal: %s\n", argv[1], path);
  return 0;
}
