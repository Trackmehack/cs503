#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <unistd.h>
#include <sys/wait.h>

#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define SH_CMD_MAX (EXE_MAX + ARG_MAX)

typedef struct command {
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

typedef struct command_list {
    int num;
    command_t commands[CMD_MAX];
} command_list_t;

#define SPACE_CHAR ' '
#define PIPE_CHAR '|'
#define PIPE_STRING "|"
#define SH_PROMPT "dsh4> "
#define EXIT_CMD "exit"

#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3

#define M_NOT_IMPL "The requested operation is not implemented yet!\n"
#define EXIT_NOT_IMPL 3

int build_cmd_list(char *cmd_line, command_list_t *clist);
int exec_local_cmd_loop(void);
char** cmd_to_argv(command_t* cmd);

#define CMD_OK_HEADER "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"

#endif