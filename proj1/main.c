#include "string_parser.h"
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char** argv) {
  // char input[] = "ls -l; rm file; cd ~ ; mkdir test; cd /home/user/obo;";
  command_line commands;
  // commands = str_tokenize(input);

  // for (int i = 0; i < commands.num_token; i++) {
  //   printf("[%s]\n", commands.command_list[i]);
  // }

  // free_command_line(&commands);

  char *line = NULL;
  size_t len = 0;
  while (1) {
    ssize_t num_read;
    ssize_t num_wrote = write(STDOUT_FILENO, ">>> ", 4);
    if (num_wrote < 0) perror("write");
    ssize_t nread = getline(&line, &len, stdin);
    commands = str_tokenize(line);

    for (int i = 0; i < commands.num_token; i++) {
      char *cur = commands.command_list[i];
      printf("[%s]\n", cur); // REMOVE LATER!!!!
      if (!strcasecmp(cur, "exit")) return EXIT_SUCCESS;
      else if (!strcasecmp(cur, "ls")) {
        listDir();
      }
    }
  }
  return 0;
}
