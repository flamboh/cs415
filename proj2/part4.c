#include "helper.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>


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

typedef struct {
  int available;
  unsigned long long syscr;
  unsigned long long syscw;
  unsigned long long read_bytes;
  unsigned long long write_bytes;
} ProcIO;

typedef struct {
  int available;
  char state[64];
  unsigned long vmrss_kb;
  unsigned long voluntary_ctxt_switches;
  unsigned long nonvoluntary_ctxt_switches;
} ProcStatus;

ProcIO read_io(pid_t pid) {
  ProcIO io = {0, 0, 0, 0, 0};
  char path[64];
  char line[256];

  snprintf(path, sizeof(path), "/proc/%d/io", pid);
  FILE* fp = fopen(path, "r");
  if (!fp) {
    return io;
  }

  io.available = 1;
  while (fgets(line, sizeof(line), fp)) {
    sscanf(line, "syscr: %llu", &io.syscr);
    sscanf(line, "syscw: %llu", &io.syscw);
    sscanf(line, "read_bytes: %llu", &io.read_bytes);
    sscanf(line, "write_bytes: %llu", &io.write_bytes);
  }

  fclose(fp);
  return io;
}

ProcStatus read_status(pid_t pid) {
  ProcStatus status = {0, "unknown", 0, 0, 0};
  char path[64];
  char line[256];

  snprintf(path, sizeof(path), "/proc/%d/status", pid);
  FILE* fp = fopen(path, "r");
  if (!fp) {
    return status;
  }

  status.available = 1;
  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "State:", 6) == 0) {
      sscanf(line, "State:\t%63[^\n]", status.state);
    }
    else if (strncmp(line, "VmRSS:", 6) == 0) {
      sscanf(line, "VmRSS: %lu kB", &status.vmrss_kb);
    }
    else if (strncmp(line, "voluntary_ctxt_switches:", 24) == 0) {
      sscanf(line, "voluntary_ctxt_switches: %lu", &status.voluntary_ctxt_switches);
    }
    else if (strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0) {
      sscanf(line, "nonvoluntary_ctxt_switches: %lu", &status.nonvoluntary_ctxt_switches);
    }
  }

  fclose(fp);
  return status;
}

void print_status(const ProcessList* process_list) {
  printf("[MCP] === Process status ===\n");
  printf("%-8s %-10s %-14s %10s %12s %12s %12s %12s %12s %12s %s\n",
         "PID",
         "STATUS",
         "PROC_STATE",
         "RSS_KB",
         "VOL_CTX",
         "INVOL_CTX",
         "SYS_READS",
         "SYS_WRITES",
         "READ_BYTES",
         "WRITE_BYTES",
         "NOTE");
  printf("%-8s %-10s %-14s %10s %12s %12s %12s %12s %12s %12s %s\n",
         "--------",
         "----------",
         "--------------",
         "----------",
         "------------",
         "------------",
         "------------",
         "------------",
         "------------",
         "------------",
         "----------------");

  for (int i = 0; i < process_list->count; i++) {
    int status = process_list->processes[i].status;
    char* status_str;
    if (status == EXITED) {
      status_str = "EXITED";
    }
    else if (status == STOPPED) {
      status_str = "STOPPED";
    }
    else if (status == NEVER_RUN) {
      status_str = "NEVER_RUN";
    }
    else {
      status_str = "RUNNING";
    }

    pid_t pid = process_list->processes[i].pid;
    ProcIO io = read_io(pid);
    ProcStatus proc_status = read_status(pid);
    const char* note = "";

    if (!io.available && !proc_status.available) {
      note = "proc unavailable";
    }
    else if (!proc_status.available) {
      note = "status unavailable";
    }
    else if (!io.available) {
      note = "io unavailable";
    }

    printf("%-8d %-10s ", pid, status_str);

    if (proc_status.available) {
      printf("%-14s %10lu %12lu %12lu ",
             proc_status.state,
             proc_status.vmrss_kb,
             proc_status.voluntary_ctxt_switches,
             proc_status.nonvoluntary_ctxt_switches);
    }
    else {
      printf("%-14s %10s %12s %12s ", "-", "-", "-", "-");
    }

    if (io.available) {
      printf("%12llu %12llu %12llu %12llu ",
             io.syscr,
             io.syscw,
             io.read_bytes,
             io.write_bytes);
    }
    else {
      printf("%12s %12s %12s %12s ", "-", "-", "-", "-");
    }

    printf("%s\n", note);
  }
  printf("[MCP] === Process status ===\n");
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

  sigset_t set;
  sigset_t oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  if (sigprocmask(SIG_BLOCK, &set, &oldset) == -1) {
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }
  while ((nread = getline(&line, &len, in)) != -1) {
    command_lines = str_tokenize(line, ";");

    for (int i = 0; i < command_lines.size; i++) {
      args = str_tokenize(command_lines.list[i], " ");
      if (process_list.count == process_list.processes_size) {
        process_list.processes_size *= 2;
        Process* tmp = realloc(process_list.processes, sizeof(Process) * process_list.processes_size);
        if (!tmp) {
          perror("realloc");
          free(line);
          free_process_list(&process_list);
          fclose(in);
          return 1;
        }
        process_list.processes = tmp;
      }
      pid_t pid = fork();
      if (pid < 0) {
        perror("fork");
        free_str_list(&args);
        continue;
      }
      if (pid == 0) {
        printf("[child %d] assigned: %s, from command_lines.list[%d]\n", getpid(), command_lines.list[i], i);
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
  if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
    perror("sigprocmask restore");
    exit(EXIT_FAILURE);
  }
  process_list.active = process_list.count;

  sleep(1);

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

  sigset_t blockset;

  sigemptyset(&blockset);
  sigemptyset(&oldset);
  sigaddset(&blockset, SIGALRM);

  if (sigprocmask(SIG_BLOCK, &blockset, &oldset) == -1) {
    perror("sigprocmask block");
    free(line);
    free_process_list(&process_list);
    fclose(in);
    return 1;
  }
  struct sigaction sa = {0};
  sa.sa_handler = handle_alarm;
  if (sigaction(SIGALRM, &sa, NULL) == -1) {
    perror("sigaction");
    free(line);
    free_process_list(&process_list);
    fclose(in);
    return 1;
  }

  sigset_t suspend_mask = oldset;
  sigdelset(&suspend_mask, SIGALRM);

  alarm(2);
  while (process_list.active > 0) {
    while (!alarm_fired) {
      if (sigsuspend(&suspend_mask) == -1 && errno != EINTR) {
        perror("sigsuspend");
        exit(EXIT_FAILURE);
      }
    }
    alarm_fired = 0;
    printf("[MCP] alarm fired, time slice expired for PID %d\n", process_list.processes[current].pid);
    reap_exited(&process_list);
    // need to check if current process is still running
    // int exited = is_process_exited(process_list.processes[current].pid);
    // if (exited == -1) {
    //   perror("is_process_exited");
    // }
    if (process_list.processes[current].status != EXITED) {
      printf("[MCP] stopping PID %d\n", process_list.processes[current].pid);
      if (kill(process_list.processes[current].pid, SIGSTOP) == -1) {
        perror("kill SIGSTOP");
      }
      else {
        process_list.processes[current].status = STOPPED;
      }
    }

    print_status(&process_list);

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
      else {
        process_list.processes[current].status = RUNNING;
      }
    }
    else if (process_list.processes[current].status == STOPPED) {
      printf("[MCP] continuing PID %d\n", process_list.processes[current].pid);
      if (kill(process_list.processes[current].pid, SIGCONT) == -1) {
        perror("kill SIGCONT");
      }
      else {
        process_list.processes[current].status = RUNNING;
      }
    }
    alarm(2);
  }
  printf("[MCP] all processes exited, MCP exiting\n");
  reap_exited(&process_list);

  // sleep(1);

  // free_str_list(&args);
  // free_str_list(&command_lines);
  free(line);
  free_process_list(&process_list);
  fclose(in);
  exit(EXIT_SUCCESS);
}
