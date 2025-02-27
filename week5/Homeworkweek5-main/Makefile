CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = dsh

SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS)

test: $(TARGET)
	./test.sh

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean test run

