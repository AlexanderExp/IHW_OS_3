#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct HotelStatus {
    int totalRooms;
    int occupiedRooms;
    int availableRooms;
};

struct HotelStatus hotel_status;

void send_response(int client_socket, const char *response) {
    if (send(client_socket, response, strlen(response), 0) < 0) {
        printf("Send failed\n");
    }
}

void handle_checkin(int client_socket) {
    pthread_mutex_lock(&mutex);

    if (hotel_status.availableRooms > 0) {
        hotel_status.occupiedRooms++;
        hotel_status.availableRooms--;
        send_response(client_socket, "Room assigned");
    } else {
        send_response(client_socket, "No available rooms");
    }

    pthread_mutex_unlock(&mutex);
}

void handle_checkout(int client_socket) {
    pthread_mutex_lock(&mutex);

    if (hotel_status.occupiedRooms > 0) {
        hotel_status.occupiedRooms--;
        hotel_status.availableRooms++;
        send_response(client_socket, "SUCCESS: Room checked out");
    } else {
        send_response(client_socket, "No occupied rooms");
    }

    pthread_mutex_unlock(&mutex);
}

void handle_status(int client_socket) {
    pthread_mutex_lock(&mutex);

    char response[MAX_BUFFER_SIZE];
    snprintf(response, sizeof(response), "Total Rooms: %d\nOccupied Rooms: %d\nAvailable Rooms: %d",
             hotel_status.totalRooms, hotel_status.occupiedRooms, hotel_status.availableRooms);
    send_response(client_socket, response);

    pthread_mutex_unlock(&mutex);
}

void *client_handler(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[MAX_BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0) {
            printf("Receive failed\n");
            break;
        }

        printf("Received command from client: %s\n", buffer);

        if (strcmp(buffer, "CHECKIN") == 0) {
            handle_checkin(client_socket);
        } else if (strcmp(buffer, "CHECKOUT") == 0) {
            handle_checkout(client_socket);
        } else if (strcmp(buffer, "STATUS") == 0) {
            handle_status(client_socket);
        } else if (strcmp(buffer, "EXIT") == 0) {
            break;
        }
    }

    close(client_socket);
    free(arg);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int server_socket;
    struct sockaddr_in server_address;
    pthread_t thread_id;

    hotel_status.totalRooms = 30;
    hotel_status.occupiedRooms = 0;
    hotel_status.availableRooms = hotel_status.totalRooms;

    pthread_mutex_init(&mutex, NULL);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Bind failed\n");
        return -1;
    }

    if (listen(server_socket, 5) < 0) {
        printf("Listen failed\n");
        return -1;
    }

    printf("Server started. Waiting for clients...\n");

    while (1) {
        int client_socket;
        struct sockaddr_in client_address;
        int client_address_size = sizeof(client_address);

        client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_address_size);
        if (client_socket < 0) {
            printf("Accept failed\n");
            return -1;
        }

        printf("New client connected\n");

        if (pthread_create(&thread_id, NULL, client_handler, (void *)&client_socket) != 0) {
            printf("Thread creation error\n");
            return -1;
        }
    }

    pthread_mutex_destroy(&mutex);
    close(server_socket);

    return 0;
}
