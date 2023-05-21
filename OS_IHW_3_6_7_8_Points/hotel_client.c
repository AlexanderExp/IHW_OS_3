#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"

int main(int argc, char *argv[]) {
    int socket_desc;
    struct sockaddr_in server;
    char* server_ip;
    int server_port;
    char guest_id[10];

    if (argc < 4) {
        printf("Usage: %s <server_ip> <server_port> <guest_id>\n", argv[0]);
        return 1;
    }

    server_ip = argv[1];
    server_port = atoi(argv[2]);
    strcpy(guest_id, argv[3]);

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    if (connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected to server\n");

    send(socket_desc, guest_id, strlen(guest_id), 0);

    close(socket_desc);

    return 0;
}
