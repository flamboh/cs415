#include "helper.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
  FILE* in;
  if (argc > 1) {
    in = fopen(argv[1], "r");
    if (!in) {
      perror("fopen");
      return 1;
    }
  }
  else {
    printf("Usage: %s [input_file]\n", argv[0]);
    return 1;
  }
  str_list command_lines;
  str_list args;

  char* line = NULL;
  size_t len = 0;
  ssize_t nread;
  pid_t* pid_array = malloc(sizeof(pid_t) * 2);
  int pid_array_size = 2;
  int processes = 0;

  while ((nread = getline(&line, &len, in)) != -1) {
    command_lines = str_tokenize(line, ";");

    for (int i = 0; i < command_lines.size; i++) {
      args = str_tokenize(command_lines.list[i], " ");
      if (pid_array_size == processes) {
        pid_array_size *= 2;
        pid_array = realloc(pid_array, sizeof(pid_t) * pid_array_size);
        if (!pid_array) {
          perror("realloc");
          return 1;
        }
      }
      pid_t pid = fork();
      if (pid < 0) {
        perror("fork");
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
    waitpid(pid_array[i], NULL, 0);
  }
  free(line);
  free(pid_array);
  free_str_list(&command_lines);
  free_str_list(&args);
  fclose(in);
  exit(EXIT_SUCCESS);
}
