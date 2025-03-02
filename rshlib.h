#ifndef __RSHLIB_H__
#define __RSHLIB_H__
#define STOP_SERVER_SC 200

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dshlib.h"  // Include this first to avoid redefinition issues

// Remote shell constants
#define RDSH_DEF_PORT           1234        // Default port #
#define RDSH_DEF_SVR_INTFACE    "0.0.0.0"   // Default start all interfaces
#define RDSH_DEF_CLI_CONNECT    "127.0.0.1" // Default server is localhost

#define RDSH_COMM_BUFF_SZ       4096        // Communication buffer size

// Special character for end-of-stream marker
static const char RDSH_EOF_CHAR = 0x04;     // EOF character (ASCII code 0x04)

// Error and status codes - using OK_EXIT from dshlib.h now
#define ERR_RDSH_COMMUNICATION  -10

// Function prototypes for client
int exec_remote_cmd_loop(char *server_ip, int port);
int start_client(char *server_ip, int port);
void client_cleanup(int socket, char *buffer);

// Function prototypes for server
int start_server(char *interface, int port, int threaded);  // Updated to match dsh_cli.c
int boot_server(char *interface, int port);
int process_cli_requests(int server_socket);
int exec_cli_requests(int client_socket);
int rsh_execute_pipeline(command_list_t *cmd_list, int client_socket);
int stop_server(int server_socket);
int send_eof(int socket);

#endif