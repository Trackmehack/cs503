#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>

#include "dshlib.h"
#include "rshlib.h"

int send_message_eof(int cli_socket) {
    char eof_char = RDSH_EOF_CHAR;
    return send(cli_socket, &eof_char, 1, 0) == 1 ? OK : ERR_RDSH_COMMUNICATION;
}

int rsh_execute_pipeline(command_list_t *cmd_list, int client_socket) {
    int pipes[cmd_list->num - 1][2];
    pid_t child_pids[CMD_MAX];
    int status;

    // Create pipes
    for (int i = 0; i < cmd_list->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }

    // Fork and execute commands
    for (int i = 0; i < cmd_list->num; i++) {
        child_pids[i] = fork();

        if (child_pids[i] == -1) {
            perror("fork");
            return -1;
        }

        if (child_pids[i] == 0) {  // Child process
            // Last command: redirect stdout to socket
            if (i == cmd_list->num - 1) {
                if (dup2(client_socket, STDOUT_FILENO) == -1) {
                    perror("dup2 stdout");
                    exit(1);
                }
                if (dup2(client_socket, STDERR_FILENO) == -1) {
                    perror("dup2 stderr");
                    exit(1);
                }
            } else {
                // Redirect stdout to next pipe
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdout");
                    exit(1);
                }
            }

            // Not first command: redirect stdin from previous pipe
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 stdin");
                    exit(1);
                }
            }

            // Close all pipe file descriptors in child
            for (int j = 0; j < cmd_list->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Prepare arguments for execvp
            char *args[ARG_MAX/2 + 2];
            args[0] = cmd_list->commands[i].exe;
            int arg_count = 1;

            // Parse arguments if present
            if (strlen(cmd_list->commands[i].args) > 0) {
                char *token = strtok(cmd_list->commands[i].args, " ");
                while (token != NULL && arg_count < ARG_MAX/2 + 1) {
                    args[arg_count++] = token;
                    token = strtok(NULL, " ");
                }
            }
            args[arg_count] = NULL;

            // Execute command
            execvp(args[0], args);
            
            // If execvp fails
            perror("execvp");
            exit(1);
        }
    }

    // Parent process: close pipe file descriptors
    for (int i = 0; i < cmd_list->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes
    for (int i = 0; i < cmd_list->num; i++) {
        waitpid(child_pids[i], &status, 0);
    }

    return WEXITSTATUS(status);
}

int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    char response[RDSH_COMM_BUFF_SZ];
    command_list_t clist;

    while (1) {
        // Receive command
        int recv_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (recv_size <= 0) break;

        // Null-terminate command
        io_buff[recv_size] = '\0';

        // Handle exit
        if (strcmp(io_buff, "exit") == 0) {
            free(io_buff);
            return OK;
        }

        // Handle stop-server
        if (strcmp(io_buff, "stop-server") == 0) {
            free(io_buff);
            return STOP_SERVER_SC;
        }

        // Handle cd command
        if (strncmp(io_buff, "cd ", 3) == 0) {
            char *path = io_buff + 3;
            
            // Remove trailing whitespace
            path[strcspn(path, "\n")] = '\0';

            // Handle ~ for home directory
            if (path[0] == '~') {
                char *home = getenv("HOME");
                if (home) {
                    char full_path[1024];
                    snprintf(full_path, sizeof(full_path), "%s%s", home, path + 1);
                    path = full_path;
                }
            }

            // Attempt to change directory
            if (chdir(path) == 0) {
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                snprintf(response, sizeof(response), "Changed to directory: %s\n", cwd);
            } else {
                snprintf(response, sizeof(response), "Error: Cannot change to directory %s\n", path);
            }

            // Send response
            send(cli_socket, response, strlen(response), 0);
            send_message_eof(cli_socket);
            continue;
        }

        // Parse and execute command
        int rc = build_cmd_list(io_buff, &clist);
        if (rc == OK) {
            // Execute pipeline, using socket as I/O
            rsh_execute_pipeline(&clist, cli_socket);
        }

        // Send EOF to client
        send_message_eof(cli_socket);
    }

    free(io_buff);
    return OK;
}