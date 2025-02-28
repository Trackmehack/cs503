CC=gcc
CFLAGS=-Wall -Werror

all: dsh

dsh: main.c dshlib.c dshlib.h
	$(CC) $(CFLAGS) -o dsh main.c dshlib.c

clean:
	rm -f dsh *.o

test:
	cd bats && ./assignment_tests.sh
	cd bats && ./student_tests.sh