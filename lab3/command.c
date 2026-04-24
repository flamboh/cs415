#include "helper.h"
#include <asm-generic/errno-base.h>
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
void lfcat()
{
  char path[PATH_MAX];
  if (getcwd(path, PATH_MAX) == NULL) {
    perror("getcwd");
  }

  DIR *dir = opendir(path);

  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    char *filename = entry->d_name;
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
      // easy_write("Error! File does not exist: ");
      // easy_write(filename);
      // easy_write("\n");
      continue;
    }

    struct stat st;
    stat(filename, &st);

    if (S_ISDIR(st.st_mode)) {
      // easy_write("Error! filename is a directory: ");
      // easy_write(filename);
      // easy_write("\n");
      close(fd);
      continue;
    }
    easy_write("File: ");
    easy_write(filename);
    easy_write("\n");
    FILE *fp = fdopen(fd, "r");
    if (fp == NULL) {
        perror("fdopen");
        return;
    }

    char *buf = NULL;
    size_t buf_len = 0;

    char *line = NULL;
    size_t line_cap = 0;

    ssize_t n;
    while ((n = getline(&line, &line_cap, fp)) != -1) {
        char *tmp = realloc(buf, buf_len + n + 1);
        if (tmp == NULL) {
            free(buf);
            free(line);
            fclose(fp);
            return;
        }

        buf = tmp;
        memcpy(buf + buf_len, line, n);
        buf_len += n;
        buf[buf_len] = '\0';
    }

    free(line);
    fclose(fp);
    easy_write(buf);
    easy_write("\n");

    free(buf);
    close(fd);
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
