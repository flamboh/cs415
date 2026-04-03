#include "string_parser.h"
#include "helper.h"
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
  int s = open(sourcePath, O_RDONLY);
  if (s < 0) {
    easy_write("Error! File does not exist: ");
    easy_write(sourcePath);
    easy_write("\n");
  }

  struct stat s_st, d_st;
  stat(sourcePath, &s_st);
  stat(destinationPath, &d_st);
  char *destFile = NULL;
  destFile = malloc(strlen(destinationPath) + 1);
  strcpy(destFile, destinationPath);
  if (S_ISDIR(d_st.st_mode)) {
    char *base = basename(sourcePath);
    void *tmp = realloc(destFile, strlen(destinationPath) + strlen(base) + 1);
    if (tmp == NULL) {
      free(destFile);
      return;
    }
    destFile = tmp;
    strcat(destFile, base);
  }


  char *buf = malloc(s_st.st_size);
  read(s, buf, s_st.st_size);

  int d = open(destFile, O_CREAT | O_WRONLY, 0755);

  if (write(d, buf, s_st.st_size) < 0) perror("write");
} /*for the cp command*/

void deleteFile(char *filename) {
  if (remove(filename) < 0) {
    easy_write("Error! File does not exist: ");
    easy_write(filename);
    easy_write("\n");
  }
} /*for the rm command*/

void moveFile(char *sourcePath, char *destinationPath) {
  copyFile(sourcePath, destinationPath);
  deleteFile(sourcePath);
} /*for the mv command*/


void displayFile(char *filename) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    easy_write("Error! File does not exist: ");
    easy_write(filename);
    easy_write("\n");
  }

  struct stat st;
  stat(filename, &st);

  char *buf = malloc(st.st_size + 1);
  read(fd, buf, st.st_size);
  buf[st.st_size] = '\0';
  easy_write(buf);
  easy_write("\n");

  free(buf);
} /*for the cat command*/
