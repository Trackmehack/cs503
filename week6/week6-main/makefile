CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = dsh
SRCS = starter/dsh_cli.c starter/dshlib.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

run:
	./dsh

test:
	bats bats/assignment_tests.sh
