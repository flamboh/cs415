#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct
{
    char** command_list;    /* Array of individual commands separated by ';' */
    int num_token;          /* Number of tokens/commands found */
} command_line;


// from SO (https://stackoverflow.com/questions/656542/trim-a-string-in-c)
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


command_line str_tokenize(char* str) {
  command_line result;
  result.command_list = NULL;
  result.num_token = 0;

  int capacity = 4;
  result.command_list = malloc(capacity * sizeof(char *));

  char *token;
  char *save = str;

  while ((token = strtok_r(save, ";", &save))) {
    if (result.num_token >= capacity) {
      capacity *= 2;
      result.command_list = realloc(result.command_list, capacity * sizeof(char *));
    }
    token = trim(token);
    result.command_list[result.num_token] = malloc(strlen(token) + 1);
    strcpy(result.command_list[result.num_token], token);
    result.num_token++;
  }

  return result;
}


/* Function to free the dynamic memory allocated within the command_line struct */
void free_command_line(command_line* control) {
  for (int i = 0; i < control->num_token; ++i) {
    free(control->command_list[i]);
  }
  free(control->command_list);
}
