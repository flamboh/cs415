#include "helper.h"
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void easy_write(const char *s) {
  ssize_t n = write(STDOUT_FILENO, s, strlen(s));
  if (n < 0) perror("write");
}



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

int count_token (char* buf, const char* delim)
{
  if (buf == NULL || delim == NULL) return 0;

  char *copy = malloc(strlen(buf) + 1);
  if (!copy) return 0;
  strcpy(copy, buf);

  int result = 0;
  char *token;
  char *save = NULL;

  for (token = strtok_r(copy, delim, &save); token != NULL; token = strtok_r(NULL, delim, &save)) {
    token = trim(token);
    if (strlen(token) > 0) {
      result++;
    }
  }

  free(copy);
  return result;
}

str_list str_tokenize(char* buf, const char* delim)
{
  str_list result;
  result.list = NULL;
  result.size = 0;

  int capacity = count_token(buf, delim);
  result.list = calloc((capacity + 1), sizeof(char *));
  if (!result.list) {
    perror("calloc");
    return result;
  }

  char *token;
  char *save = NULL;

  for (token = strtok_r(buf, delim, &save); token != NULL; token = strtok_r(NULL, delim, &save)) {
    token = trim(token);
    if (strlen(token) < 1) continue;

    result.list[result.size] = malloc(strlen(token) + 1);
    if (!result.list[result.size]) {
      perror("malloc");
      free_str_list(&result);
      result.list = NULL;
      result.size = 0;
      return result;
    }

    strcpy(result.list[result.size], token);
    result.size++;
  }

  result.list[result.size] = NULL;
  return result;
}


void free_str_list(str_list *list) {
  if (!list || !list->list) return;
  for (int i = 0; i < list->size; ++i) {
    free(list->list[i]);
  }
  free(list->list);
  list->list = NULL;
  list->size = 0;
}


void free_process_list(ProcessList *list) {
  free(list->processes);
}
