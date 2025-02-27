#!/bin/bash

echo "Running tests..."
echo "----------------------------"

echo "Testing 'ls' command"
(echo "ls"; echo "exit") | ./dsh | tail -n +2
echo "----------------------------"

echo "Testing 'pwd' command"
(echo "pwd"; echo "exit") | ./dsh | tail -n +2
echo "----------------------------"

echo "Testing 'cd ..' and 'pwd'"
(echo "cd .."; echo "pwd"; echo "exit") | ./dsh | tail -n +2
echo "----------------------------"

echo "Testing invalid command"
(echo "invalidcmd"; echo "exit") | ./dsh | tail -n +2
echo "----------------------------"

echo "All tests completed."
