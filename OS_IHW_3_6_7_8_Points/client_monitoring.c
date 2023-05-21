#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888

int main() {
    int socket_desc;
    struct sockaddr_in server;
    char server_message[2000];

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected to server\n");

    while (1) {
        int recv_size = recv(socket_desc, server_message, sizeof(server_message), 0);
        if (recv_size <= 0) {
            break;
        }

        printf("%.*s\n", recv_size, server_message);
        memset(server_message, 0, sizeof(server_message));// Clear the message buffer
    }

    close(socket_desc);

    return 0;
}
