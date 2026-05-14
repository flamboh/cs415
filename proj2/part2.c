#include "helper.h"
#include <signal.h>
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

  sigset_t set;
  sigset_t oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);

  if (sigprocmask(SIG_BLOCK, &set, &oldset) == -1) {
    perror("sigprocmask");
    free(line);
    free(pid_array);
    fclose(in);
    return 1;
  }

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
        printf("[child %d] executing: %s, from command_lines.list[%d]\n", getpid(), command_lines.list[i], i);
        int sig;
        if(sigwait(&set, &sig) != 0) {
          fprintf(stderr, "sigwait failed\n");
          free(line);
          free(pid_array);
          free_str_list(&command_lines);
          free_str_list(&args);
          fclose(in);
          exit(EXIT_FAILURE);
        }
        printf("[child %d] received SIGUSR1, execing %s\n", getpid(), command_lines.list[i]);
        if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
          perror("sigprocmask restore child");
          free(line);
          free(pid_array);
          free_str_list(&command_lines);
          free_str_list(&args);
          fclose(in);
          _exit(EXIT_FAILURE);
        }
        execvp(args.list[0], args.list);
        perror("execvp");
        free(line);
        free(pid_array);
        free_str_list(&command_lines);
        free_str_list(&args);
        fclose(in);
        _exit(EXIT_FAILURE);

          // } else if (sig == SIGSTOP) {
          //   pause();
          // } else if (sig == SIGCONT) {
          //   continue;
      }
      pid_array[processes++] = pid;
      free_str_list(&args);
    }
    free_str_list(&command_lines);
  }

  if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
    perror("sigprocmask restore parent");
    free(line);
    free(pid_array);
    fclose(in);
    return 1;
  }

  sleep(1);

  printf("[MCP] %d processes forked\n", processes);
  printf("[MCP] sending SIGUSR1 to all children\n");
  for (int i = 0; i < processes; i++) {
    if (kill(pid_array[i], SIGUSR1) == -1) {
      perror("kill SIGUSR1");
    }
  }

  sleep(1);

  printf("[MCP] sending SIGSTOP to all children\n");
  for (int i = 0; i < processes; i++) {
    if (kill(pid_array[i], SIGSTOP) == -1) {
      perror("kill SIGSTOP");
    }
  }

  sleep(1);

  printf("[MCP] sending SIGCONT to all children\n");
  for (int i = 0; i < processes; i++) {
    if (kill(pid_array[i], SIGCONT) == -1) {
      perror("kill SIGCONT");
    }
  }

  printf("[MCP] waiting for all children to exit\n");
  for (int i = 0; i < processes; i++) {
    int status;
    pid_t done = waitpid(pid_array[i], &status, 0);
    if (done == -1) {
      perror("waitpid");
      continue;
    }
    if (WIFEXITED(status)) {
      printf("[MCP] child %d exited with status %d\n", done, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)){
      printf("[MCP] child %d terminated by signal %d\n", done, WTERMSIG(status));
    }
  }
  free(line);
  free(pid_array);
  // free_str_list(&command_lines);
  // free_str_list(&args);
  fclose(in);
  exit(EXIT_SUCCESS);
}
