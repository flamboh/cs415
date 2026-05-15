CC = gcc
CFLAGS = -Wall -Wextra -g -ggdb3

HELPERS = helper.c
HELPER_OBJS = helper.o

HELPER_PROGRAMS = main
STANDALONE_PROGRAMS = iobound

PROGRAMS = $(HELPER_PROGRAMS) $(STANDALONE_PROGRAMS)
ALL_PROGRAMS = $(PROGRAMS)
OBJS = $(ALL_PROGRAMS:%=%.o) $(HELPER_OBJS)

all: $(PROGRAMS)

# programs that need helper.o
$(HELPER_PROGRAMS): %: %.o $(HELPER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# self-contained programs
$(STANDALONE_PROGRAMS): %: %.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(ALL_PROGRAMS) $(OBJS)
