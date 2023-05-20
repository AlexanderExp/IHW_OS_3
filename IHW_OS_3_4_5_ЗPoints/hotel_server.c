#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NUM_ROOMS 30

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t room_available = PTHREAD_COND_INITIALIZER;

int occupied_rooms = 0;

void* client_handler(void* arg) {
    int client_socket = *(int*)arg;
    char buffer[1024] = {0};
    int valread;

    while ((valread = read(client_socket, buffer, sizeof(buffer))) > 0) {
        if (strcmp(buffer, "CHECKIN") == 0) {
            pthread_mutex_lock(&mutex);

            if (occupied_rooms < NUM_ROOMS) {
                occupied_rooms++;
                char response[] = "SUCCESS: Room checked in";
                send(client_socket, response, strlen(response), 0);
            } else {
                char response[] = "ERROR: No available rooms";
                send(client_socket, response, strlen(response), 0);
                pthread_cond_wait(&room_available, &mutex);
                occupied_rooms++;
                char retry_response[] = "SUCCESS: Room checked in after waiting";
                send(client_socket, retry_response, strlen(retry_response), 0);
            }

            pthread_mutex_unlock(&mutex);
        } else if (strcmp(buffer, "CHECKOUT") == 0) {
            pthread_mutex_lock(&mutex);

            occupied_rooms--;
            char response[] = "SUCCESS: Room checked out";
            send(client_socket, response, strlen(response), 0);

            pthread_cond_signal(&room_available);

            pthread_mutex_unlock(&mutex);
        } else if (strcmp(buffer, "EXIT") == 0) {
            break;
        }

        memset(buffer, 0, sizeof(buffer));
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

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Hotel server started. Listening on %s:%d...\n", ip, port);

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen))) {
        printf("New client connected\n");

        pthread_t thread_id;
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread_id, NULL, client_handler, (void*)client_socket) < 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        pthread_detach(thread_id);
    }

    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return 0;
}
