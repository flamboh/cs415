# Pseudo-Shell Project Checklist

This checklist is aligned directly with the project rubric. Verify each item before submission.

---

## 1. Core Shell Behavior

### Input and Parsing
- [ ] Uses `getline()` to read input (file and stdin)
- [ ] Tokenizes input using provided `command_line` struct and parser
- [ ] Supports multiple commands per line using `;`
- [ ] Executes commands sequentially in order
- [ ] Handles arbitrary spacing (`ls;ls` == `ls ; ls`)

### Execution Flow
- [ ] Main loop runs until `exit` is entered
- [ ] `exit` cleanly terminates the shell
- [ ] After each line:
  - [ ] Interactive mode waits for next input
  - [ ] File mode reads next line

---

## 2. Modes

### Interactive Mode
- [ ] Default when no arguments provided
- [ ] Reads from stdin
- [ ] Writes to stdout
- [ ] Displays prompt `>>>`
- [ ] Handles continuous usage without crashing

### File Mode
- [ ] Triggered with `-f <filename>`
- [ ] Reads commands from file
- [ ] Writes all output to `output.txt`
- [ ] No console output in file mode
- [ ] Creates `output.txt` in working directory

---

## 3. Error Handling

### Parameter Errors
- [ ] Detects missing parameters
- [ ] Detects too many parameters
- [ ] Prints correct error messages

### Syntax Errors
- [ ] Detects unrecognized commands
- [ ] Detects malformed input (e.g. `; ls`, `ls ls`)
- [ ] Stops processing rest of the line after an error

### Robustness
- [ ] Errors do not crash the program
- [ ] Errors do not cause memory leaks
- [ ] Works with irregular spacing

---

## 4. Command Implementations

General requirements:
- [ ] All commands implemented in `command.c`
- [ ] Use Linux system calls (not libc wrappers)
- [ ] Do not use `printf`, `fprintf`, `fopen`, `freopen`

### ls
- [ ] Uses directory syscalls (e.g. `opendir`, `readdir`)
- [ ] Prints directory contents correctly

### pwd
- [ ] Uses `getcwd`
- [ ] Prints correct working directory

### mkdir
- [ ] Uses `mkdir`
- [ ] Creates directory successfully
- [ ] Handles "already exists" error

### cd
- [ ] Uses `chdir`
- [ ] Changes working directory correctly

### cp
- [ ] Uses `open`, `read`, `write`
- [ ] Supports:
  - [ ] `cp file dir`
  - [ ] `cp dir/file dir`
  - [ ] `cp dir/file dir/file`
  - [ ] `cp file file`
- [ ] Copies file contents correctly

### mv
- [ ] Uses `open`, `read`, `write`, `unlink`
- [ ] Supports:
  - [ ] `mv file dir`
  - [ ] `mv dir/file dir/file`
  - [ ] `mv file file`
- [ ] Removes source file after move

### rm
- [ ] Removes files only (not directories)
- [ ] Errors when file does not exist

### cat
- [ ] Uses system calls to read file
- [ ] Outputs file contents correctly

---

## 5. Project Structure

Required files:
- [ ] `main.c`
- [ ] `command.c`
- [ ] `command.h` (unchanged)
- [ ] `string_parser.c`
- [ ] `string_parser.h` (unchanged)
- [ ] `Makefile`
- [ ] `README.txt`

### Code Organization
- [ ] `main.c` handles modes and execution loop
- [ ] Uses `argc` and `argv`
- [ ] Parses input and dispatches commands
- [ ] `command.c` functions match `command.h`
- [ ] Provided headers are not modified

---

## 6. Memory and System Constraints

- [ ] Uses `malloc` and `free` correctly
- [ ] No memory leaks (Valgrind clean)
- [ ] No invalid memory access
- [ ] No dangling pointers

---

## 7. Build and Submission

### Build
- [ ] Compiles with `make`
- [ ] Runs correctly in required environment

### Submission Contents
- [ ] Screenshot of successful compile
- [ ] All source files included
- [ ] Working Makefile
- [ ] README included
- [ ] 1–2 page report included

---

## 8. Disallowed Practices

- [ ] No use of `execvp` or similar
- [ ] No calling native shell commands
- [ ] No modifying `command.h`
- [ ] No `printf` in command functions
- [ ] Makefile must work

---

## Final Priority Check

Before submitting, confirm:
- [ ] Parsing and execution loop works correctly
- [ ] Commands implemented with system calls
- [ ] Error handling matches specification
- [ ] No memory leaks (Valgrind clean)
- [ ] Both modes function correctly