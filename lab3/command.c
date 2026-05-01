#include "helper.h"
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

static int should_skip_file(const char *filename)
{
  return strcmp(filename, ".") == 0 ||
         strcmp(filename, "..") == 0 ||
         strcmp(filename, "main.c") == 0 ||
         strcmp(filename, "command.c") == 0 ||
         strcmp(filename, "output.txt") == 0 ||
         strcmp(filename, "shell.exe") == 0;
}

static int is_text_file(int fd)
{
  char buf[512];
  ssize_t bytes_read = read(fd, buf, sizeof(buf));

  if (bytes_read < 0) {
    return 0;
  }

  if (lseek(fd, 0, SEEK_SET) == -1) {
    return 0;
  }

  for (ssize_t i = 0; i < bytes_read; ++i) {
    unsigned char c = (unsigned char)buf[i];
    if (c == '\0') {
      return 0;
    }
    if (!isprint(c) && c != '\n' && c != '\r' && c != '\t') {
      return 0;
    }
  }

  return 1;
}

void lfcat()
{
  char path[PATH_MAX];
  if (getcwd(path, PATH_MAX) == NULL) {
    perror("getcwd");
    return;
  }

  DIR *dir = opendir(path);
  if (dir == NULL) {
    perror("opendir");
    return;
  }

  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    char *filename = entry->d_name;

    if (should_skip_file(filename)) {
      continue;
    }

    struct stat st;
    if (stat(filename, &st) == -1 || !S_ISREG(st.st_mode)) {
      continue;
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
      continue;
    }

    if (!is_text_file(fd)) {
      close(fd);
      continue;
    }

    easy_write("File: ");
    easy_write(filename);
    easy_write("\n");

    FILE *fp = fdopen(fd, "r");
    if (fp == NULL) {
      perror("fdopen");
      close(fd);
      continue;
    }

    char *line = NULL;
    size_t line_cap = 0;

    ssize_t n;
    while ((n = getline(&line, &line_cap, fp)) != -1) {
      if (write(STDOUT_FILENO, line, n) == -1) {
        perror("write");
        break;
      }
    }

    free(line);
    fclose(fp);
    easy_write("\n");

    for (int i = 0; i < 80; ++i) easy_write("-");
    easy_write("\n");
  }

  closedir(dir);
/* High level functionality you need to implement: */

	/* Get the current directory with getcwd() */

	/* Open the dir using opendir() */

	/* use a while loop to read the dir with readdir()*/

		/* You can debug by printing out the filenames here */

		/* Option: use an if statement to skip any names that are not readable files (e.g. ".", "..", "main.c", "lab2.exe", "output.txt" */

			/* Open the file */

			/* Read in each line using getline() */
				/* Write the line to stdout */

			/* write 80 "-" characters to stdout */

			/* close the read file and free/null assign your line buffer */

	/*close the directory you were reading from using closedir() */
}
