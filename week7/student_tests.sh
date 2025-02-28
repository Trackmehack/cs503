#!/usr/bin/env bats

# Basic test to check if shell runs and exits correctly
@test "Shell exits with status code 0" {
    echo "exit" | ../dsh
    [ "$?" -eq 0 ]
}

# Test a simple pipe between two commands
@test "Simple pipe with ls and grep" {
    result=$(echo "ls | grep Makefile" | ../dsh | grep Makefile)
    [ "$?" -eq 0 ]
    [[ "$result" == *"Makefile"* ]]
}

# Test a pipe with three commands
@test "Triple pipe with ls, grep, and wc" {
    result=$(echo "ls | grep .c | wc -l" | ../dsh)
    [ "$?" -eq 0 ]
}

# Test error handling for too many pipe commands
@test "Too many commands error" {
    result=$(echo "cmd1 | cmd2 | cmd3 | cmd4 | cmd5 | cmd6 | cmd7 | cmd8 | cmd9" | ../dsh)
    [[ "$result" == *"error: piping limited to 8 commands"* ]]
}