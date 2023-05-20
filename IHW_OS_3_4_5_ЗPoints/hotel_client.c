#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_address;
    int sock = 0;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_address.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        printf("Enter command (CHECKIN, CHECKOUT, or EXIT): ");
        scanf("%s", buffer);

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            printf("Send failed\n");
            return -1;
        }

        memset(buffer, 0, sizeof(buffer));

        if (strcmp(buffer, "EXIT") == 0) {
            break;
        }

        if (recv(sock, buffer, sizeof(buffer), 0) < 0) {
            printf("Receive failed\n");
            return -1;
        }

        printf("Server response: %s\n", buffer);
    }

    close(sock);

    return 0;
}
