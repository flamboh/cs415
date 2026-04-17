#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_parser.h"

#define _GUN_SOURCE

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}


char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
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

command_line str_filler (char* buf, const char* delim)
{
  command_line result;
  result.command_list = NULL;
  result.num_token = count_token(buf, delim);
  result.command_list = malloc((result.num_token + 1) * sizeof(char *));
  char *token;
  char *save = buf;
  int cur = 0;

  while ((token = strtok_r(save, delim, &save))) {
    token = trim(token);
    if (strlen(token) < 1) continue;
    result.command_list[cur] = malloc(strlen(token) + 1);
    strcpy(result.command_list[cur], token);
    cur++;
  }

  result.command_list[cur] = NULL;
  return result;
}


void free_command_line(command_line* command)
{
  for (int i = 0; i < command->num_token; ++i) {
    free(command->command_list[i]);
  }
  free(command->command_list);
}
