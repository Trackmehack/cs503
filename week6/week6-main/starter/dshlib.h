#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define SH_PROMPT "dsh2> "
#define EXIT_CMD "exit"
#define CMD_ARGV_MAX 10
#define OK 0
#define ERR_EXEC_CMD -1

// Command buffer struct
typedef struct {
    int argc;
    char *argv[CMD_ARGV_MAX];
} cmd_buff_t;

// Function prototypes
int exec_local_cmd_loop();
void exec_cmd(cmd_buff_t *cmd);
int parse_command(char *input, cmd_buff_t *cmd);
void execute_cd(cmd_buff_t *cmd);

#endif
