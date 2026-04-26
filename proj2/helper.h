#ifndef HELPER_H
#define HELPER_H


typedef struct
{
  char** list;
  int size;
} str_list;


int count_token (char* buf, const char* delim);

str_list str_tokenize(char* str, const char* delim);

void free_str_list(str_list* arr);


void easy_write(const char *s);

char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

#endif
