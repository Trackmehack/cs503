CC=gcc
CFLAGS=-Wall -Werror -I/opt/homebrew/Cellar/argp-standalone/1.5.0/include
LDFLAGS=-L/opt/homebrew/Cellar/argp-standalone/1.5.0/lib -largp

all: dsh

dsh: dsh_cli.c dshlib.c rsh_cli.c rsh_server.c missing_functions.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f dsh *.o

test:
	cd bats && ./assignment_tests.sh
	cd bats && ./student_tests.sh