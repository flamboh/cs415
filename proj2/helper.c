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
  if (buf == NULL) return 0;
  int len = strlen(buf);
  int result = 0;


  for (int i = 0; i < len; ++i) {
    if (i == 0 && buf[i] == *delim) continue;
    if (i == len - 1 && buf[i] == *delim) break;
    if (buf[i] == *delim) result++;
    if (i == len - 1) result++;
  }
  return result;
}

str_list str_tokenize(char* buf, const char* delim)
{
  str_list result;
  result.list = NULL;
  result.size = count_token(buf, delim);
  result.list = malloc((result.size + 1) * sizeof(char *));
  char *token;
  char *save = buf;
  int cur = 0;

  while ((token = strtok_r(save, delim, &save))) {
    token = trim(token);
    if (strlen(token) < 1) continue;
    result.list[cur] = malloc(strlen(token) + 1);
    strcpy(result.list[cur], token);
    cur++;
  }

  result.list[cur] = NULL;
  return result;
}


void free_str_list(str_list *list) {
  for (int i = 0; i < list->size; ++i) {
    free(list->list[i]);
  }
  free(list->list);
}
