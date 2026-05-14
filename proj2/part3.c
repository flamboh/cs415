#include "helper.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

volatile sig_atomic_t alarm_fired = 0;

void handle_alarm(int sig) {
  (void)sig;
  alarm_fired = 1;
}

int find_next_runnable(ProcessList* process_list, int current) {
  for (int offset = 1; offset <= process_list->count; offset++) {
    int next = (current + offset) % process_list->count;
    if (process_list->processes[next].status != EXITED) {
      return next;
    }
  }

  return -1;
}

// int is_process_exited(pid_t pid) {
//   if (kill(pid, 0) == 0) {
//     // process exists and is still running
//     return 0;
//   }

//   if (errno == ESRCH) {
//     // process does not exist
//     return 1;
//   }

//   if (errno == EPERM) {
//     // process exists no perms
//     return 0;
//   }

//   return -1;
// }

void mark_exited(ProcessList* process_list, int i) {
    if (process_list->processes[i].status != EXITED) {
      process_list->processes[i].status = EXITED;
      process_list->active--;
  }
}

void reap_exited(ProcessList* process_list) {
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    for (int i = 0; i < process_list->count; i++) {
      if (process_list->processes[i].pid == pid) {
        mark_exited(process_list, i);

        if (WIFEXITED(status)) {
          printf("[MCP] child %d exited with status %d\n", pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
          printf("[MCP] child %d terminated by signal %d\n", pid, WTERMSIG(status));
        }

        break;
      }
    }
  }
}

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
  // pid_t* pid_array = malloc(sizeof(pid_t) * 2);
  ProcessList process_list = {NULL, 0, 0, 0};
  process_list.processes = malloc(sizeof(Process) * 2);
  if (!process_list.processes) {
    perror("malloc");
    return 1;
  }
  process_list.processes_size = 2;


  while ((nread = getline(&line, &len, in)) != -1) {
    command_lines = str_tokenize(line, ";");

    for (int i = 0; i < command_lines.size; i++) {
      args = str_tokenize(command_lines.list[i], " ");
      if (process_list.count == process_list.processes_size) {
        process_list.processes_size *= 2;
        process_list.processes = realloc(process_list.processes, sizeof(Process) * process_list.processes_size);
        if (!process_list.processes) {
          perror("realloc");
          return 1;
        }
      }
      sigset_t set;
      sigemptyset(&set);
      sigaddset(&set, SIGUSR1);
      // sigaddset(&set, SIGSTOP);
      // sigaddset(&set, SIGCONT);

      if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
          perror("sigprocmask");
          exit(EXIT_FAILURE);
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
          free_process_list(&process_list);
          free_str_list(&command_lines);
          free_str_list(&args);
          fclose(in);
          exit(EXIT_FAILURE);
        }
        printf("[child %d] received SIGUSR1, execing %s\n", getpid(), command_lines.list[i]);
        execvp(args.list[0], args.list);
        perror("execvp");
        free(line);
        free_process_list(&process_list);
        free_str_list(&command_lines);
        free_str_list(&args);
        fclose(in);
        _exit(EXIT_FAILURE);

          // } else if (sig == SIGSTOP) {
          //   pause();
          // } else if (sig == SIGCONT) {
          //   continue;
      }
      process_list.processes[process_list.count].pid = pid;
      process_list.processes[process_list.count].status = NEVER_RUN;
      process_list.count++;
      free_str_list(&args);
    }
    free_str_list(&command_lines);
  }
  process_list.active = process_list.count;

  // sleep(1);

  printf("[MCP] %d processes forked\n", process_list.count);
  printf("[MCP] sending SIGUSR1 to all children\n");
  if (process_list.active == 0) {
    free(line);
    free_process_list(&process_list);
    fclose(in);
    return 0;
  }
  int current = 0;
  if (kill(process_list.processes[current].pid, SIGUSR1) == -1) {
    perror("kill SIGUSR1");
  }
  process_list.processes[current].status = RUNNING;


  struct sigaction sa = {0};
  sa.sa_handler = handle_alarm;
  sigaction(SIGALRM, &sa, NULL);

  alarm(1);
  while (process_list.active > 0) {
    if (!alarm_fired) pause();


    if (alarm_fired) {
      alarm_fired = 0;
      printf("[MCP] alarm fired, time slice expired for PID %d\n", process_list.processes[current].pid);
      reap_exited(&process_list);
      // need to check if current process is still running
      // int exited = is_process_exited(process_list.processes[current].pid);
      // if (exited == -1) {
      //   perror("is_process_exited");
      // }
      printf("[MCP] stopping PID %d\n", process_list.processes[current].pid);
      if (process_list.processes[current].status != EXITED) {
        if (kill(process_list.processes[current].pid, SIGSTOP) == -1) {
          perror("kill SIGSTOP");
        }
        process_list.processes[current].status = STOPPED;
      }
      // else if (exited == 1) {
      //   if (kill(process_list.processes[current].pid, SIGTERM) == -1) {
      //     perror("kill SIGTERM");
      //   }
      //   mark_exited(&process_list, current);
      // }
      // else {
      //   if (kill(process_list.processes[current].pid, SIGSTOP) == -1) {
      //     perror("kill SIGSTOP");
      //   }
      //   process_list.processes[current].status = STOPPED;
      // }

      reap_exited(&process_list);
      // find next process
      current = find_next_runnable(&process_list, current);
      if (current == -1) {
        continue;
      }
      printf("[MCP] next runnable process is PID %d\n", process_list.processes[current].pid);
      // based on the current state, determine signal to send
      // NEVER_RUN -> SIGUSR1
      //
      if (process_list.processes[current].status == NEVER_RUN) {
        printf("[MCP] starting PID %d\n", process_list.processes[current].pid);
        if (kill(process_list.processes[current].pid, SIGUSR1) == -1) {
          perror("kill SIGUSR1");
        }
        process_list.processes[current].status = RUNNING;
      }
      else if (process_list.processes[current].status == STOPPED) {
        printf("[MCP] continuing PID %d\n", process_list.processes[current].pid);
        if (kill(process_list.processes[current].pid, SIGCONT) == -1) {
          perror("kill SIGCONT");
        }
        process_list.processes[current].status = RUNNING;
      }
      alarm(2);
    }
  }
  printf("[MCP] all processes exited, MCP exiting");
  reap_exited(&process_list);

  // for (int i = 0; i < processes; i++) {
  //   if (kill(pid_array[i], SIGUSR1) == -1) {
  //     perror("kill SIGUSR1");
  //   }
  // }
  //


  // sleep(1);

  // printf("[MCP] sending SIGSTOP to all children\n");
  // for (int i = 0; i < processes; i++) {
  //   if (kill(pid_array[i], SIGSTOP) == -1) {
  //     perror("kill SIGSTOP");
  //   }
  // }

  // sleep(1);

  // printf("[MCP] sending SIGCONT to all children\n");
  // for (int i = 0; i < processes; i++) {
  //   if (kill(pid_array[i], SIGCONT) == -1) {
  //     perror("kill SIGCONT");
  //   }
  // }

  // printf("[MCP] waiting for all children to exit\n");
  // for (int i = 0; i < processes; i++) {
  //   int status;
  //   pid_t done = waitpid(pid_array[i], &status, 0);
  //   if (done == -1) {
  //     perror("waitpid");
  //     continue;
  //   }
  //   if (WIFEXITED(status)) {
  //     printf("[MCP] child %d exited with status %d\n", done, WEXITSTATUS(status));
  //   } else if (WIFSIGNALED(status)){
  //     printf("[MCP] child %d terminated by signal %d\n", done, WTERMSIG(status));
  //   }
  // }
  // free_str_list(&args);
  // free_str_list(&command_lines);
  free(line);
  free_process_list(&process_list);
  fclose(in);
  exit(EXIT_SUCCESS);
}
