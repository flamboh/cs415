#include "string_parser.h"
#include "helper.h"
#include <string.h>
#include <stdlib.h>


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
    if (strlen(token) < 1) continue;
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
