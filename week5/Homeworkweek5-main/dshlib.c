#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (cmd_line == NULL || strlen(cmd_line) == 0)
    {
        return WARN_NO_CMDS;
    }

    memset(clist, 0, sizeof(command_list_t));

    char *token;
    int cmd_count = 0;

    token = strtok(cmd_line, PIPE_STRING);
    while (token != NULL)
    {
        if (cmd_count >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        while (*token == SPACE_CHAR)
            token++;

        char *end = token + strlen(token) - 1;
        while (end > token && *end == SPACE_CHAR)
            *end-- = '\0';

        char *arg_ptr = strchr(token, SPACE_CHAR);
        if (arg_ptr)
        {
            *arg_ptr = '\0';
            arg_ptr++;
        }

        if (strlen(token) >= EXE_MAX || (arg_ptr && strlen(arg_ptr) >= ARG_MAX))
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        strcpy(clist->commands[cmd_count].exe, token);
        if (arg_ptr)
        {
            strcpy(clist->commands[cmd_count].args, arg_ptr);
        }

        cmd_count++;

        token = strtok(NULL, PIPE_STRING);
    }

    clist->num = cmd_count;
    return OK;
}
