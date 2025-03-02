#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>
#include "dshlib.h"
#include "rshlib.h"

// Command line arguments structure
struct arguments {
    int mode;           // 0=local, 1=client, 2=server
    char *ip;           // IP address or interface
    int port;           // Port number
    int threaded_server; // Threaded server flag
};

// Mode constants
#define MODE_LOCAL  0
#define MODE_CLIENT 1
#define MODE_SERVER 2

// Parse command line options
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'c':
            arguments->mode = MODE_CLIENT;
            break;
        case 's':
            arguments->mode = MODE_SERVER;
            break;
        case 'i':
            arguments->ip = arg;
            break;
        case 'p':
            arguments->port = atoi(arg);
            break;
        case 'x':
            arguments->threaded_server = 1;
            break;
        case 'h':
            argp_state_help(state, state->out_stream, ARGP_HELP_STD_HELP);
            exit(0);
            break;
        case ARGP_KEY_ARG:
            return 0;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    struct arguments arguments;
    int rc;

    // Default argument values
    arguments.mode = MODE_LOCAL;
    arguments.ip = NULL;
    arguments.port = RDSH_DEF_PORT;
    arguments.threaded_server = 0;

    // Define command line options
    struct argp_option options[] = {
        {"client", 'c', 0, 0, "Run as client"},
        {"server", 's', 0, 0, "Run as server"},
        {"ip", 'i', "IP", 0, "Set IP/Interface address"},
        {"port", 'p', "PORT", 0, "Set port number"},
        {"threaded", 'x', 0, 0, "Enable threaded mode (server only)"},
        {"help", 'h', 0, 0, "Show this help message"},
        {0}
    };

    // Set up argp parser
    struct argp argp = {options, parse_opt, 0, "Usage: ./dsh [-c | -s] [-i IP] [-p PORT] [-x] [-h]\n  Default is to run ./dsh in local mode"};

    // Parse command line arguments
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // Set appropriate IP address based on mode
    if (arguments.ip == NULL) {
        if (arguments.mode == MODE_SERVER) {
            arguments.ip = RDSH_DEF_SVR_INTFACE;
        } else if (arguments.mode == MODE_CLIENT) {
            arguments.ip = RDSH_DEF_CLI_CONNECT;
        }
    }

    // Run in appropriate mode
    switch (arguments.mode) {
        case MODE_LOCAL:
            printf("local mode\n");
            rc = exec_local_cmd_loop();
            break;
        case MODE_CLIENT:
            printf("socket client mode:  addr:%s:%d\n", arguments.ip, arguments.port);
            rc = exec_remote_cmd_loop(arguments.ip, arguments.port);
            break;
        case MODE_SERVER:
            printf("socket server mode:  addr:%s:%d\n", arguments.ip, arguments.port);
            printf("-> %s Mode\n", arguments.threaded_server ? "Threaded" : "Single-Threaded");
            rc = start_server(arguments.ip, arguments.port, arguments.threaded_server);
            break;
        default:
            fprintf(stderr, "Unknown mode %d\n", arguments.mode);
            return 1;
    }

    // Print return code and exit
    printf("cmd loop returned %d\n", rc);
    return rc;
}