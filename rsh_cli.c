#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"  // Default server IP
#define BUFFER_SIZE 4096
#define CMD_SIZE 1024

int connect_to_server(int port) {
    int sock;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // Set up server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IP address from text to binary
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        return -1;
    }

    return sock;
}

void handle_client_input(int sock) {
    char cmd[CMD_SIZE];
    char output[BUFFER_SIZE];

    while (1) {
        printf("dsh4> ");
        fflush(stdout);

        // Read user input
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            printf("\n");  // Handle Ctrl+D (EOF)
            break;
        }

        // Remove newline and check exit condition
        cmd[strcspn(cmd, "\n")] = '\0';
        if (strcmp(cmd, "exit") == 0) {
            printf("Exiting client...\n");
            break;
        }

        // Send command to server (including null terminator)
        if (send(sock, cmd, strlen(cmd) + 1, 0) < 0) {
            perror("Send failed");
            break;
        }

        // Receive response from server
        ssize_t bytes_received;
        while ((bytes_received = recv(sock, output, sizeof(output) - 1, 0)) > 0) {
            output[bytes_received] = '\0';
            printf("%s", output);

            // Check for end-of-stream character (0x04 / EOF)
            if (output[bytes_received - 1] == 0x04) {
                break;
            }
        }

        // Handle server disconnect
        if (bytes_received <= 0) {
            printf("Server disconnected.\n");
            break;
        }
    }

    // Cleanup
    close(sock);
}
