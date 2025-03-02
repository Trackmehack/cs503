#!/usr/bin/env bats

@test "Local shell starts and exits" {
    echo "exit" | ../dsh
    [ "$?" -eq 0 ]
}

@test "Server starts and can be stopped" {
    # Start server in background with unique port
    ../dsh -s -p 9876 &
    server_pid=$!
    sleep 1
    
    # Verify server is running
    ps -p $server_pid
    [ "$?" -eq 0 ]
    
    # Connect client and send stop-server command
    echo "stop-server" | ../dsh -c -p 9876
    
    # Wait for server to stop
    sleep 1
    
    # Verify server has stopped
    ps -p $server_pid
    [ "$?" -ne 0 ]
}

@test "Client can connect and run simple command" {
    # Start server in background
    ../dsh -s -p 9877 &
    server_pid=$!
    sleep 1
    
    # Run command and capture output
    result=$(echo "echo hello_world" | ../dsh -c -p 9877)
    
    # Stop server
    echo "stop-server" | ../dsh -c -p 9877
    
    # Check result
    [[ "$result" == *"hello_world"* ]]
}

@test "Client can run piped commands" {
    # Start server in background
    ../dsh -s -p 9878 &
    server_pid=$!
    sleep 1
    
    # Run piped command
    result=$(echo "echo hello world | grep world" | ../dsh -c -p 9878)
    
    # Stop server
    echo "stop-server" | ../dsh -c -p 9878
    
    # Check result
    [[ "$result" == *"world"* ]]
}

@test "Client can change directory on server" {
    # Start server in background
    ../dsh -s -p 9879 &
    server_pid=$!
    sleep 1
    
    # Run cd command followed by pwd
    output=$(echo -e "cd /tmp\npwd" | ../dsh -c -p 9879)
    
    # Stop server
    echo "stop-server" | ../dsh -c -p 9879
    
    # Check output contains /tmp
    [[ "$output" == *"/tmp"* ]]
}