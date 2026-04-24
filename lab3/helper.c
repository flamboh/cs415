#include "helper.h"
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

void easy_write(const char *s) {
  ssize_t n = write(STDOUT_FILENO, s, strlen(s));
  if (n < 0) perror("write");
}

#include <ctype.h>


// from SO (https://stackoverflow.com/questions/656542/trim-a-string-in-c)
// lightly modified
char *ltrim(char *s)
{
  while(isspace((unsigned char)*s)) s++;
  return s;
}


char *rtrim(char *s)
{
  size_t len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1])) len--;
  s[len]= '\0';
  return s;
}


char *trim(char *s)
{
  return rtrim(ltrim(s));
}

command_args parse_command_args(char *str) {
  command_args result;
  result.arg_list = NULL;
  result.num_args = 0;

  int capacity = 4;
  result.arg_list = malloc(capacity * sizeof(char *));

  char *token;
  char *save = str;

  while ((token = strtok_r(save, " ", &save))) {
    if (result.num_args >= capacity) {
      capacity *= 2;
      result.arg_list = realloc(result.arg_list, capacity * sizeof(char *));
    }
    token = trim(token);
    if (strlen(token) < 1) continue;
    result.arg_list[result.num_args] = malloc(strlen(token) + 1);
    strcpy(result.arg_list[result.num_args], token);
    result.num_args++;
  }

  return result;
}

void free_command_args(command_args *args) {
  for (int i = 0; i < args->num_args; ++i) {
    free(args->arg_list[i]);
  }
  free(args->arg_list);
}
