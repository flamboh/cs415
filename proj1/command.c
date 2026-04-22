#include "helper.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

void listDir() {
  char path[PATH_MAX];
  if (getcwd(path, PATH_MAX) == NULL) {
    perror("getcwd");
  }

  DIR *dir = opendir(path);

  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    easy_write(entry->d_name);
  easy_write(" ");
  }
  easy_write("\n");

  closedir(dir);
}

void showCurrentDir() {
  char path[PATH_MAX];
  if (getcwd(path, PATH_MAX) == NULL) {
    perror("getcwd");
  }
  easy_write(path);
  easy_write("\n");
}

void makeDir(char *dirName) {
  if (mkdir(dirName, 0755) < 0) {
    easy_write("Error! Directory name already exists: ");
    easy_write(dirName);
    easy_write("\n");
  }
}

void changeDir(char *dirName) {
  if (!dirName) dirName = getenv("HOME");
  if (!dirName) {
    easy_write("Error! Home directory not found: cd");
    easy_write("\n");
  }
  if (chdir(dirName) < 0) {
    easy_write("Error! Directory does not exist:");
    easy_write(dirName);
    easy_write("\n");
  }
}

void copyFile(char *sourcePath, char *destinationPath) {
  int s = -1, d = -1;
  s = open(sourcePath, O_RDONLY);
  char *destFile = NULL;
  struct stat s_st, d_st;
  int dest_exists = 0;

  if (s < 0) {
    easy_write("Error! File does not exist: ");
    easy_write(sourcePath);
    easy_write("\n");
    return;
  }

  if (stat(sourcePath, &s_st) == -1) {
    perror("stat");
    close(s);
    return;
  }
  if (stat(destinationPath, &d_st) == -1) {
    if (errno != ENOENT) {
      perror("stat");
      close(s);
      return;
    }
  } else {
    dest_exists = 1;
  }

  destFile = malloc(strlen(destinationPath) + 1);
  if (!destFile) {
      close(s);
      return;
  }
  strcpy(destFile, destinationPath);
  if (dest_exists && S_ISDIR(d_st.st_mode)) {
    char *base = basename(sourcePath);
    void *tmp = realloc(destFile, strlen(destinationPath) + strlen(base) + 2);
    if (tmp == NULL) {
      free(destFile);
      return;
    }
    destFile = tmp;
    strcat(destFile, "/");
    strcat(destFile, base);
  }

  // char *buf = malloc(s_st.st_size);
  // read(s, buf, s_st.st_size);

  d = open(destFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (d < 0) {
      perror("open destination");
      free(destFile);
      close(s);
      return;
  }

  // if (write(d, buf, s_st.st_size) < 0) perror("write");

  char buf[4096];
  ssize_t nread;
  while ((nread = read(s, buf, sizeof(buf))) > 0) {
      ssize_t off = 0;
      while (off < nread) {
          ssize_t nwritten = write(d, buf + off, nread - off);
          if (nwritten < 0) {
              perror("write");
              free(destFile);
              close(s);
              close(d);
          }
          off += nwritten;
      }
  }
  if (nread < 0) perror("read");

  free(destFile);
  close(s);
  close(d);
} /*for the cp command*/

void deleteFile(char *filename) {
  unlink(filename);
  if (errno == EISDIR) {
    easy_write("Error! filename is a directory: ");
    easy_write(filename);
    easy_write("\n");
  }
  else {
    easy_write("Error! File does not exist: ");
    easy_write(filename);
    easy_write("\n");
  }
} /*for the rm command*/

void moveFile(char *sourcePath, char *destinationPath) {
  copyFile(sourcePath, destinationPath);
  struct stat st;
  if (stat(destinationPath, &st) == 0) deleteFile(sourcePath);
} /*for the mv command*/


void displayFile(char *filename) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    easy_write("Error! File does not exist: ");
    easy_write(filename);
    easy_write("\n");
    return;
  }

  struct stat st;
  stat(filename, &st);

  char *buf = malloc(st.st_size + 1);
  read(fd, buf, st.st_size);
  buf[st.st_size] = '\0';
  easy_write(buf);
  easy_write("\n");

  free(buf);
  close(fd);
} /*for the cat command*/
