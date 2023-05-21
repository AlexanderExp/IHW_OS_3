#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

void handle_response(const char *response) {
    printf("Server response: %s\n", response);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_address;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    printf("Connected to the server\n");

    char buffer[MAX_BUFFER_SIZE];
    char response[MAX_BUFFER_SIZE];

    while (1) {
        printf("Enter command (STATUS, EXIT): ");
        fgets(buffer, sizeof(buffer), stdin);

        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            printf("Send failed\n");
            break;
        }

        memset(response, 0, sizeof(response));
        if (recv(client_socket, response, sizeof(response), 0) <= 0) {
            printf("Receive failed\n");
            break;
        }

        handle_response(response);

        if (strcmp(buffer, "EXIT\n") == 0) {
            break;
        }
    }

    close(client_socket);

    return 0;
}
