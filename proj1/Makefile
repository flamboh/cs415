CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = test.out
SRC = test.c string_parser.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
