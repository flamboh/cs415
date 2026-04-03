#include "string_parser.h"
#include "command.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>

void write_parameters_error(char *command) {
  easy_write("Error! Unsupported parameters for command: ");
  easy_write(command);
  easy_write("\n");
}


int main(int argc, char** argv) {
  FILE* in = stdin;
  if (argc > 1) {
    in = fopen(argv[1], "r");
    if (!in) {
      perror("fopen");
      return 1;
    }
  }

  command_line commands;

  char *line = NULL;
  size_t len = 0;
  int exit_flag = 1;
  while (exit_flag) {
    easy_write(">>> ");
    ssize_t nread = getline(&line, &len, in);
    if (nread == -1) {
      if (feof(in)) break;
      else perror("getline");
    }
    commands = str_tokenize(line);
    command_args args;
    args.arg_list = NULL;
    args.num_args = 0;

    for (int i = 0; i < commands.num_token; i++) {
      char *cur = commands.command_list[i];

      args = parse_command_args(cur);
      if (args.num_args <= 0) continue;

      char *command = args.arg_list[0];
      // printf("[%s]\n", cur);


      if (!strcasecmp(command, "exit")) {
        exit_flag = 0;
        break; }
      else if (!strcasecmp(command, "ls")) listDir();
      else if (!strcasecmp(command, "pwd")) showCurrentDir();
      else if (!strcasecmp(command, "mkdir")) {
        if (args.num_args != 2) {
          write_parameters_error(command);
          continue;
        }
        makeDir(args.arg_list[1]);
      }
      else if (!strcasecmp(command, "cd")) {
        if (args.num_args > 2) {
          write_parameters_error(command);
          continue;
        }
        changeDir(args.arg_list[1]);
      }
      else {
        easy_write("Error! Unrecognized command: ");
        easy_write(command);
        easy_write("\n");
      }
    }
    if (args.arg_list) {
      free_command_args(&args);
    }
    free_command_line(&commands);
    commands.command_list = NULL;
    commands.num_token = 0;
  }
  fclose(in);
  free(line);
  return 0;
}
