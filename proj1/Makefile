CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = test.out
SRC = main.c string_parser.c command.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
