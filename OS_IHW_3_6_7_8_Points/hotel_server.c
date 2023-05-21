#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NUM_ROOMS 30
#define MAX_WAIT_TIME 5
#define PORT 8888

int occupied_rooms = 0;
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;
int monitor_sock;

void check_in(int guest_id) {
    pthread_mutex_lock(&rooms_mutex);

    if (occupied_rooms == NUM_ROOMS) {
        printf("Guest %d is waiting for a room.\n", guest_id);
        sleep(1);
    }

    occupied_rooms++;
    printf("Guest %d checked in. Occupied rooms: %d\n", guest_id, occupied_rooms);

    // Send update to the monitoring client
    char update_message[200];
    sprintf(update_message, "Guest %d checked in. Occupied rooms: %d\n", guest_id, occupied_rooms);
    send(monitor_sock, update_message, strlen(update_message), 0);

    pthread_mutex_unlock(&rooms_mutex);
}

void check_out(int guest_id) {
    pthread_mutex_lock(&rooms_mutex);

    occupied_rooms--;
    printf("Guest %d checked out. Occupied rooms: %d\n", guest_id, occupied_rooms);

    // Send update to the monitoring client
    char update_message[200];
    sprintf(update_message, "Guest %d checked out. Occupied rooms: %d\n", guest_id, occupied_rooms);
    send(monitor_sock, update_message, strlen(update_message), 0);

    pthread_mutex_unlock(&rooms_mutex);
}

void* client_handler(void* arg) {
    int client_sock = *(int*)arg;
    char client_message[2000];

    while (recv(client_sock, client_message, 2000, 0) > 0) {
        int guest_id = atoi(client_message);
        check_in(guest_id);
        sleep(rand() % MAX_WAIT_TIME + 1);
        check_out(guest_id);
    }

    printf("Client disconnected\n");

    close(client_sock);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int socket_desc, client_sock;
    struct sockaddr_in server, client;
    pthread_t thread_id;
    int *new_sock;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return 1;
    }

    listen(socket_desc, 3);

    printf("Hotel server started. Waiting for incoming connections...\n");

    monitor_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (monitor_sock == -1) {
        printf("Could not create monitoring socket");
        return 1;
    }

    if (connect(monitor_sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("monitoring client connect failed. Error");
        return 1;
    }

    printf("Monitoring client connected\n");

    while ((client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&client))) {
        printf("Connection accepted\n");

        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&thread_id, NULL, client_handler, (void*)new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }

        pthread_detach(thread_id);
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    close(monitor_sock);

    return 0;
}
