#include "string_parser.h"
#include "helper.h"
#include <linux/limits.h>
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

  printf("%s\n", path);
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
  char path[PATH_MAX];
  if (getcwd(path, PATH_MAX) == NULL) {
    perror("getcwd");
  }

  printf("%s\n", path);
  DIR *dir = opendir(path);

  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    if (!strcasecmp(entry->d_name, dirName))
    {
      easy_write("Error! Directory name already exists:");
      easy_write(dirName);
      easy_write("\n");
    }
  }
  mkdir(dirName, 0755);
  closedir(dir);
}

void changeDir(char *dirName); /*for the cd command*/

void copyFile(char *sourcePath, char *destinationPath); /*for the cp command*/

void moveFile(char *sourcePath, char *destinationPath); /*for the mv command*/

void deleteFile(char *filename); /*for the rm command*/

void displayFile(char *filename); /*for the cat command*/
