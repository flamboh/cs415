#ifndef HELPER_H
#define HELPER_H

typedef struct
{
  char** arg_list;    /* Array of args seperated by ' ' */
  int num_args;
} command_args;

void easy_write(const char *s);

char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

command_args parse_command_args(char *str);

void free_command_args(command_args *args);

#endif
