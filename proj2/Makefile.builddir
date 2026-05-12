CC = gcc
CFLAGS = -Wall -Wextra -g -ggdb3

BUILD_DIR = build

HELPERS = helper.c
HELPER_OBJS = $(HELPERS:%.c=$(BUILD_DIR)/%.o)

HELPER_PROGRAMS = part1 part2
STANDALONE_PROGRAMS = cpubound iobound

PROGRAMS = $(HELPER_PROGRAMS) $(STANDALONE_PROGRAMS)
EXES = $(PROGRAMS:%=$(BUILD_DIR)/%)

all: $(EXES)

# programs that need helper.o
$(HELPER_PROGRAMS:%=$(BUILD_DIR)/%): $(BUILD_DIR)/%: $(BUILD_DIR)/%.o $(HELPER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# self-contained programs
$(STANDALONE_PROGRAMS:%=$(BUILD_DIR)/%): $(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
