#include "string_parser.h"
#include <stdio.h>



int main(void) {
  char input[] = "ls -l; rm file; cd ~ ; mkdir test; cd /home/user/obo;";
  command_line commands;
  commands = str_tokenize(input);

  for (int i = 0; i < commands.num_token; i++) {
    printf("[%s]\n", commands.command_list[i]);
  }

  free_command_line(&commands);
  return 0;
}
