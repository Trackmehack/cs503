#include "dshlib.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Handles the built-in `cd` command.
 */
void execute_cd(cmd_buff_t *cmd) {
    if (cmd->argc < 2) {
        char *home = getenv("HOME"); // Default to HOME if no argument
        if (home) {
            chdir(home);
        }
        return;
    }
    if (chdir(cmd->argv[1]) != 0) {
        perror("cd failed");
    }
}

/**
 * Executes an external command using fork and execvp.
 */
void exec_cmd(cmd_buff_t *cmd) {
    if (cmd->argc == 0) return;

    // Handle built-in commands
    if (strcmp(cmd->argv[0], "cd") == 0) {
        execute_cd(cmd);
        return;
    }

    if (strcmp(cmd->argv[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd failed");
        }
        return;
    }

    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            fprintf(stderr, "Command not found in PATH: %s\n", cmd->argv[0]);
            exit(ERR_EXEC_CMD);
        }
    } else if (pid > 0) { // Parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("Fork failed");
    }
}

/**
 * Parses user input into the `cmd_buff_t` structure (handles quoted strings correctly).
 */
int parse_command(char *input, cmd_buff_t *cmd) {
    cmd->argc = 0;
    char *ptr = input;
    char *arg_start = NULL;

    while (*ptr) {
        while (*ptr == ' ') ptr++; // Skip leading spaces

        if (*ptr == '"') { // Handle quoted strings
            ptr++; // Skip opening quote
            arg_start = ptr;
            while (*ptr && (*ptr != '"' || (*(ptr - 1) == '\\'))) ptr++; // Handle escaped quotes
            if (*ptr == '"') {
                *ptr = '\0'; // Null terminate quoted string
            }
            ptr++; // Move past closing quote
        } else { // Regular argument
            arg_start = ptr;
            while (*ptr && *ptr != ' ' && *ptr != '"') ptr++;
            if (*ptr == ' ') *ptr = '\0'; // Null terminate argument
            ptr++;
        }

        if (arg_start) {
            cmd->argv[cmd->argc++] = arg_start;
        }

        if (cmd->argc >= CMD_ARGV_MAX - 1) break; // Prevent overflow
    }

    cmd->argv[cmd->argc] = NULL;
    return cmd->argc > 0 ? OK : ERR_EXEC_CMD;
}

/**
 * Main loop for the shell. Handles interactive mode and script execution.
 */
int exec_local_cmd_loop() {
    char input[1024];
    cmd_buff_t cmd;

    while (1) {
        // Ensure the prompt is displayed immediately when running interactively
        if (isatty(STDIN_FILENO)) {
            printf("%s", SH_PROMPT);
            fflush(stdout);
        }

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            fflush(stdout);
            break;
        }

        input[strcspn(input, "\n")] = '\0'; // Remove newline

        if (strlen(input) == 0) {
            continue; // Ignore empty inputs
        }

        if (strcmp(input, EXIT_CMD) == 0) {
            break;
        }

        if (parse_command(input, &cmd) == OK) {
            exec_cmd(&cmd);
            fflush(stdout); // Ensure output appears immediately
        }
    }
    return OK;
}
