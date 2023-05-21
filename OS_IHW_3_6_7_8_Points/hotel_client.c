#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888

void send_guest_id(int guest_id) {
    int socket_desc;
    struct sockaddr_in server;
    char message[2000];

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return;
    }

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return;
    }

    printf("Connected to server\n");

    sprintf(message, "%d", guest_id);

    if (send(socket_desc, message, strlen(message), 0) < 0) {
        perror("send failed");
        return;
    }

    printf("Guest %d sent check-in request\n", guest_id);

    close(socket_desc);
}

int main() {
    srand(time(NULL));

    int num_guests = 100;

    printf("Welcome to the hotel!\n");

    for (int i = 1; i <= num_guests; i++) {
        send_guest_id(i);
        sleep(rand() % 3 + 1);
    }

    printf("All guests have checked in and out.\n");

    return 0;
}
