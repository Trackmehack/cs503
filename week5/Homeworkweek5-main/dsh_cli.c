#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

int main()
{
    char cmd_buff[SH_CMD_MAX];
    int rc;
    command_list_t clist;

    while (1)
    {
        printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0)
        {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0)
        {
            exit(OK);  // ✅ Removed "Exiting dsh..."
        }

        rc = build_cmd_list(cmd_buff, &clist);

        if (rc == OK)
        {
            printf(CMD_OK_HEADER, clist.num);
            for (int i = 0; i < clist.num; i++)
            {
                printf("<%d> %s", i + 1, clist.commands[i].exe);
                if (strlen(clist.commands[i].args) > 0)
                {
                    printf("[%s]", clist.commands[i].args); // ✅ Ensured no extra spaces
                }
                printf("\n");
            }
        }
        else if (rc == WARN_NO_CMDS)
        {
            printf("%s", CMD_WARN_NO_CMD);
        }
        else if (rc == ERR_TOO_MANY_COMMANDS)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        }
    }

    return OK;
}
