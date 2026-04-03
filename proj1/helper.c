#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <string.h>

void easy_write(const char *s) {
  ssize_t n = write(STDOUT_FILENO, s, strlen(s));
  if (n < 0) perror("write");
}

#include <ctype.h>


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
