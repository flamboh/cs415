#include "string_parser.h"
#include "helper.h"
#include <linux/limits.h>
#include <fcntl.h>
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

} /*for the cp command*/

void moveFile(char *sourcePath, char *destinationPath); /*for the mv command*/

void deleteFile(char *filename) {
  if (rmdir(filename) < 0) {
    easy_write("Error! File does not exist: ");
    easy_write(filename);
    easy_write("\n");
  }
} /*for the rm command*/

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
