#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include "dshlib.h"

// Function to parse command input into a list of commands
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (cmd_line == NULL || strlen(cmd_line) == 0) {
        return WARN_NO_CMDS;
    }
    
    memset(clist, 0, sizeof(command_list_t));
    char *token;
    int cmd_count = 0;
    
    token = strtok(cmd_line, PIPE_STRING);
    while (token != NULL) {
        if (cmd_count >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Trim leading spaces
        while (*token == SPACE_CHAR)
            token++;
        
        // Trim trailing spaces
        char *end = token + strlen(token) - 1;
        while (end > token && *end == SPACE_CHAR)
            *end-- = '\0';

        // Separate command and arguments
        char *arg_ptr = strchr(token, SPACE_CHAR);
        if (arg_ptr) {
            *arg_ptr = '\0';
            arg_ptr++;
        }

        // Validate command length
        if (strlen(token) >= EXE_MAX || (arg_ptr && strlen(arg_ptr) >= ARG_MAX)) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Store command and arguments in the command list
        strcpy(clist->commands[cmd_count].exe, token);
        if (arg_ptr) {
            strcpy(clist->commands[cmd_count].args, arg_ptr);
        }

        cmd_count++;
        token = strtok(NULL, PIPE_STRING);
    }

    clist->num = cmd_count;
    return OK;
}

// Convert command and args to argv array format expected by execvp
char** cmd_to_argv(command_t* cmd) {
    char** argv = malloc(ARG_MAX * sizeof(char*));
    if (!argv) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    int i = 0;
    argv[i++] = cmd->exe;
    
    if (strlen(cmd->args) > 0) {
        char* arg_copy = strdup(cmd->args);
        char* token = strtok(arg_copy, " ");
        while (token != NULL) {
            argv[i++] = token;
            token = strtok(NULL, " ");
        }
    }
    
    argv[i] = NULL;
    return argv;
}
// Execute the shell loop, handling command execution and piping
int exec_local_cmd_loop() {
    char cmd_buff[SH_CMD_MAX];
    int rc;
    command_list_t clist;
    
    while (1) {
        // Print shell prompt
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        // Remove trailing newline
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        // Skip empty commands
        if (strlen(cmd_buff) == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }
        
        // Check for exit command
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            printf("exiting...\n");
            return OK;
        }
        
        // Parse command list
        rc = build_cmd_list(cmd_buff, &clist);
        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                printf("%s", CMD_WARN_NO_CMD);
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            }
            continue;
        }
        
        // Execute the commands
        if (clist.num == 1) {
            // Single command (no pipes)
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                char** argv = cmd_to_argv(&clist.commands[0]);
                execvp(clist.commands[0].exe, argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
            } else {
                perror("fork");
                return -1;
            }
        } else {
            // Multiple commands with pipes
            int i;
            int pipes[CMD_MAX-1][2]; // We need pipes between commands
            pid_t pids[CMD_MAX];
            
            // Create all the necessary pipes
            for (i = 0; i < clist.num - 1; i++) {
                if (pipe(pipes[i]) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }
            
            // Create and connect all processes
            for (i = 0; i < clist.num; i++) {
                pids[i] = fork();
                
                if (pids[i] < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pids[i] == 0) {
                    // Child process
                    
                    // Connect pipes
                    if (i > 0) {
                        // Not the first command, connect input from previous pipe
                        if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    if (i < clist.num - 1) {
                        // Not the last command, connect output to next pipe
                        if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    // Close all pipe file descriptors in child
                    for (int j = 0; j < clist.num - 1; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                    
                    // Execute the command
                    char** argv = cmd_to_argv(&clist.commands[i]);
                    execvp(clist.commands[i].exe, argv);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            
            // Parent process
            // Close all pipe file descriptors in parent
            for (i = 0; i < clist.num - 1; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            
            // Wait for all child processes to finish
            for (i = 0; i < clist.num; i++) {
                int status;
                waitpid(pids[i], &status, 0);
            }
        }
    }
    
    return OK;
}