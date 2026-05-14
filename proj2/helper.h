#ifndef HELPER_H
#define HELPER_H
#include <stdlib.h>

typedef struct
{
  char** list;
  int size;
} str_list;

typedef enum {
  NEVER_RUN = 0,
  RUNNING,
  STOPPED,
  EXITED,
} Status;

typedef struct {
  pid_t pid;
  Status status;
} Process;

typedef struct {
  Process* processes;
  int processes_size;
  int count;
  int active;
} ProcessList;

int count_token (char* buf, const char* delim);

str_list str_tokenize(char* str, const char* delim);

void free_str_list(str_list* arr);
void free_process_list(ProcessList *list);


void easy_write(const char *s);

char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

#endif
