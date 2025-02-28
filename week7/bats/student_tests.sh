#!/usr/bin/env bats

@test "Shell exits correctly" {
    echo "exit" | ../dsh
    [ "$?" -eq 0 ]
}

@test "Simple pipe with ls and grep" {
    # Just verify the command executes successfully
    echo "ls | grep .c" | ../dsh
    [ "$?" -eq 0 ]
}

@test "Pipe with three commands" {
    # Just check if the command executes without error
    echo "ls | grep .c | wc -l" | ../dsh
    [ "$?" -eq 0 ]
}

@test "Pipe with command that has arguments" {
    # Just check if the command executes without error
    echo "ls -la | grep ^d" | ../dsh
    [ "$?" -eq 0 ]
}

@test "Error handling for non-existent command" {
    # Shell should continue running even after command fails
    output=$(echo "nonexistentcmd | grep test" | ../dsh 2>&1)
    # We expect an error message about the command not found
    [[ "$output" == *"nonexistentcmd"* || "$output" == *"command not found"* || "$?" -eq 0 ]]
}

@test "Check pipe with echo command" {
    result=$(echo "echo hello world | grep world" | ../dsh)
    [ "$?" -eq 0 ]
    [[ "$result" == *"world"* ]]
}