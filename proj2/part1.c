#include "helper.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>


int main(int argc, char* argv[]) {
  FILE* in;
  if (argc != 3 || strcmp(argv[1], "-f") != 0) {
    printf("Invalid use: incorrect number of parameters\n");
    return 1;
  }
  in = fopen(argv[2], "r");
  if (!in) {
    perror("fopen");
    return 1;
  }
  str_list command_lines;
  str_list args;

  char* line = NULL;
  size_t len = 0;
  ssize_t nread;
  pid_t* pid_array = malloc(sizeof(pid_t) * 2);
  if (!pid_array) {
    perror("malloc");
    free(line);
    fclose(in);
    return 1;
  }
  int pid_array_size = 2;
  int processes = 0;

  while ((nread = getline(&line, &len, in)) != -1) {
    command_lines = str_tokenize(line, ";");

    for (int i = 0; i < command_lines.size; i++) {
      args = str_tokenize(command_lines.list[i], " ");
      if (pid_array_size == processes) {
        pid_array_size *= 2;
        pid_t* tmp = realloc(pid_array, sizeof(pid_t) * pid_array_size);
        if (!tmp) {
          perror("realloc");
          free(line);
          free(pid_array);
          free_str_list(&command_lines);
          free_str_list(&args);
          fclose(in);
          return 1;
        }
        pid_array = tmp;
      }
      pid_t pid = fork();
      if (pid < 0) {
        perror("fork");
        free_str_list(&args);
        continue;
      }
      if (pid == 0) {
        printf("Child process %d executing: %s, from command_lines.list[%d]\n", processes, command_lines.list[i], i);
        if (execvp(args.list[0], args.list) == -1) {
          perror("exec");
          free(line);
          free(pid_array);
          free_str_list(&command_lines);
          free_str_list(&args);
          fclose(in);
          _exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
      }
      pid_array[processes++] = pid;
      free_str_list(&args);
    }
    free_str_list(&command_lines);
  }
  for (int i = 0; i < processes; i++) {
    int status;
    pid_t done = waitpid(pid_array[i], &status, 0);
    if (done == -1) {
      perror("waitpid");
      continue;
    }

    if (WIFEXITED(status)) {
      printf("Parent: child %d exited with status %d\n", done, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("Parent: child %d terminated by signal %d\n", done, WTERMSIG(status));
    }
  }
  free(line);
  free(pid_array);
  // free_str_list(&command_lines);
  // free_str_list(&args);
  fclose(in);
  exit(EXIT_SUCCESS);
}
