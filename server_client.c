#include "dshlib.h"
#include "rshlib.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Client implementation
int exec_remote_cmd_loop(char *server_ip, int port) {
    int client_socket;
    char cmd_buff[SH_CMD_MAX];
    char *response_buffer = NULL;
    int bytes_sent, recv_size, is_last_chunk;
    
    // Connect to server
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_socket);
        return -1;
    }
    
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        return -1;
    }
    
    printf("Connected to server %s:%d\n", server_ip, port);
    
    // Allocate response buffer
    response_buffer = malloc(RDSH_COMM_BUFF_SZ);
    if (!response_buffer) {
        perror("Memory allocation failed");
        close(client_socket);
        return -1;
    }
    
    // Command loop
    while (1) {
        // Prompt for command
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        // Remove newline character
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        // Check if command is empty
        if (strlen(cmd_buff) == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }
        
        // Send command to server (including null terminator)
        bytes_sent = send(client_socket, cmd_buff, strlen(cmd_buff) + 1, 0);
        if (bytes_sent <= 0) {
            perror("send");
            break;
        }
        
        // Receive and process server response
        while (1) {
            // Clear buffer
            memset(response_buffer, 0, RDSH_COMM_BUFF_SZ);
            
            // Receive data
            recv_size = recv(client_socket, response_buffer, RDSH_COMM_BUFF_SZ - 1, 0);
            
            if (recv_size <= 0) {
                // Connection closed or error
                if (recv_size < 0) {
                    perror("recv");
                }
                goto cleanup;
            }
            
            // Check if this is the last chunk (ends with EOF character)
            is_last_chunk = (recv_size > 0 && response_buffer[recv_size - 1] == RDSH_EOF_CHAR);
            
            // If it's the last chunk, remove the EOF character
            if (is_last_chunk) {
                response_buffer[recv_size - 1] = '\0';
                recv_size--;
            }
            
            // Print the received data
            if (recv_size > 0) {
                printf("%.*s", recv_size, response_buffer);
                fflush(stdout); // Ensure output is displayed immediately
            }
            
            // If it's the last chunk, exit the receive loop
            if (is_last_chunk) {
                break;
            }
        }
        
        // Check if user wants to exit
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            printf("Exiting...\n");
            break;
        }
    }
    
cleanup:
    if (response_buffer) free(response_buffer);
    close(client_socket);
    return OK;
}

// Server implementation
int start_server(char *interface, int port, int threaded) {
    int server_socket;
    struct sockaddr_in server_addr;
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_socket);
        return -1;
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert IP address from string to binary
    if (inet_pton(AF_INET, interface, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(server_socket);
        return -1;
    }
    
    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }
    
    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        return -1;
    }
    
    printf("Server started on %s:%d\n", interface, port);
    
    // Process client connections
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char *buffer = malloc(RDSH_COMM_BUFF_SZ);
    int recv_size;
    command_list_t cmd_list;
    int rc;
    
    if (!buffer) {
        perror("Memory allocation failed");
        close(server_socket);
        return -1;
    }
    
    while (1) {
        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));
        
        // Handle client requests
        while (1) {
            // Clear buffer for new command
            memset(buffer, 0, RDSH_COMM_BUFF_SZ);
            
            // Receive command from client
            recv_size = recv(client_socket, buffer, RDSH_COMM_BUFF_SZ - 1, 0);
            
            if (recv_size <= 0) {
                // Client disconnected or error
                if (recv_size < 0) {
                    perror("recv");
                }
                break;
            }
            
            // Ensure buffer is null-terminated
            buffer[recv_size] = '\0';
            
            // Parse command
            rc = build_cmd_list(buffer, &cmd_list);
            
            if (rc != OK) {
                // Send error message for parsing errors
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg), "Command parsing error: %d\n", rc);
                send(client_socket, error_msg, strlen(error_msg), 0);
                
                // Send EOF to indicate end of output
                char eof = RDSH_EOF_CHAR;
                send(client_socket, &eof, 1, 0);
                continue;
            }
            
            // Handle built-in commands
            if (cmd_list.num == 1) {
                // Check for exit command
                if (strcmp(cmd_list.commands[0].exe, "exit") == 0) {
                    printf("Client requested exit\n");
                    char eof = RDSH_EOF_CHAR;
                    send(client_socket, &eof, 1, 0);
                    break;
                }
                
                // Check for stop-server command
                if (strcmp(cmd_list.commands[0].exe, "stop-server") == 0) {
                    printf("Client requested server stop\n");
                    char eof = RDSH_EOF_CHAR;
                    send(client_socket, &eof, 1, 0);
                    close(client_socket);
                    free(buffer);
                    close(server_socket);
                    return OK;
                }
                
                // Handle cd command
                if (strcmp(cmd_list.commands[0].exe, "cd") == 0) {
                    char *dir = cmd_list.commands[0].args;
                    if (strlen(dir) == 0) {
                        dir = getenv("HOME");
                    }
                    
                    if (chdir(dir) != 0) {
                        char error_msg[100];
                        snprintf(error_msg, sizeof(error_msg), "cd: failed to change directory to %s\n", dir);
                        send(client_socket, error_msg, strlen(error_msg), 0);
                    } else {
                        char success_msg[100];
                        snprintf(success_msg, sizeof(success_msg), "Changed directory to %s\n", dir);
                        send(client_socket, success_msg, strlen(success_msg), 0);
                    }
                    
                    // Send EOF to indicate end of output
                    char eof = RDSH_EOF_CHAR;
                    send(client_socket, &eof, 1, 0);
                    continue;
                }
            }
            
            // Execute regular commands
            pid_t pid = fork();
            
            if (pid < 0) {
                // Fork failed
                perror("fork");
                char eof = RDSH_EOF_CHAR;
                send(client_socket, &eof, 1, 0);
                continue;
            } else if (pid == 0) {
                // Child process
                
                // Redirect stdout and stderr to client socket
                dup2(client_socket, STDOUT_FILENO);
                dup2(client_socket, STDERR_FILENO);
                
                // Handle piped commands
                if (cmd_list.num > 1) {
                    int i;
                    int pipes[CMD_MAX-1][2]; // Pipes between commands
                    pid_t pids[CMD_MAX];
                    
                    // Create all necessary pipes
                    for (i = 0; i < cmd_list.num - 1; i++) {
                        if (pipe(pipes[i]) == -1) {
                            perror("pipe");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    // Create and connect processes
                    for (i = 0; i < cmd_list.num; i++) {
                        pids[i] = fork();
                        
                        if (pids[i] < 0) {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        } else if (pids[i] == 0) {
                            // Child process for command
                            
                            // Connect pipes
                            if (i > 0) {
                                // Connect input from previous pipe
                                dup2(pipes[i-1][0], STDIN_FILENO);
                            }
                            
                            if (i < cmd_list.num - 1) {
                                // Connect output to next pipe
                                dup2(pipes[i][1], STDOUT_FILENO);
                            }
                            
                            // Close all pipe file descriptors
                            for (int j = 0; j < cmd_list.num - 1; j++) {
                                close(pipes[j][0]);
                                close(pipes[j][1]);
                            }
                            
                            // Execute command
                            char** argv = cmd_to_argv(&cmd_list.commands[i]);
                            execvp(cmd_list.commands[i].exe, argv);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    // Parent in fork
                    // Close all pipe file descriptors
                    for (i = 0; i < cmd_list.num - 1; i++) {
                        close(pipes[i][0]);
                        close(pipes[i][1]);
                    }
                    
                    // Wait for all child processes
                    for (i = 0; i < cmd_list.num; i++) {
                        int status;
                        waitpid(pids[i], &status, 0);
                    }
                    
                    exit(0);
                } else {
                    // Single command execution
                    char** argv = cmd_to_argv(&cmd_list.commands[0]);
                    execvp(cmd_list.commands[0].exe, argv);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
                
                // Add a small delay to ensure all output is sent
                usleep(10000);
                
                // Send EOF to indicate end of output
                char eof = RDSH_EOF_CHAR;
                send(client_socket, &eof, 1, 0);
            }
        }
        
        close(client_socket);
    }
    
    free(buffer);
    close(server_socket);
    return OK;
}