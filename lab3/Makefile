CC = gcc
CFLAGS = -Wall -Wextra -g -ggdb3

TARGET = shell.exe
SRC = main.c string_parser.c command.c helper.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
