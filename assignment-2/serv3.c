#include "assignment2.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int *connection_counter;
int shm_id, mutex;
struct sembuf up = {0, 1, 0};
struct sembuf down = {0, -1, 0};
struct sockaddr_in server_address, client_address;
socklen_t address_length;


void sig_chld (int sig) {
    while (waitpid(0, NULL, WNOHANG) > 0) {
        ;
    }
    signal(SIGCHLD,sig_chld);
}


void sig_int (int sig) {
    // On termination, clean up shared memory
    shmdt((void *) connection_counter);  // detach from shared memory
    shmctl(shm_id, IPC_RMID, 0);         // remove shared memory
    semctl(mutex, 0, IPC_RMID);          // remove semaphore
    exit(EXIT_SUCCESS);
}


void setup_shm (void) {
    shm_id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    connection_counter = (int *) shmat(shm_id, 0, 0);
    *connection_counter = 0;
}


void treat_request (int socket) {

    int current, result, written = 0;

    if (semop(mutex, &down, 1) == -1) {
        perror("Semaphore error");
        return;
    }

    *connection_counter += 1;
    current = htonl(*connection_counter);

    if (semop(mutex, &up, 1) == -1) {
        perror("Semaphore error");
        return;
    }

    while (written < sizeof(int)) {
        result = write(socket, (const void *) (&current - written), sizeof(int) - written);
        if (result == -1) {
            perror("Write error");
            break;
        }
        written = written + result;
    }

}


void receive_requests (int socket) {

    int client_socket;

    while (1) {
        client_socket = accept(socket, (struct sockaddr *) &client_address, &address_length);
        if (client_socket == -1) {
            perror("Connection error");
            continue;
        }
        connection_counter = (int *) shmat(shm_id, 0, 0);
        treat_request(client_socket);
        shmdt((void *) connection_counter);    // detach from shared memory
        close(client_socket);
    }

}

int main (int argc, char **argv) {

    int option_value, process_counter, listen_socket;
    pid_t pid;

    mutex = semget(IPC_PRIVATE, 1, 0600);
    semop(mutex, &up, 1);  // Initialize for mutual exclusion

    address_length = sizeof(struct sockaddr_in);
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // When one tries to invoke the server multiple times after eachother within
    // a small amount of time, one will find that the socket is still in the
    // TIME_WAIT state, in order to catch delayed packets and not to confuse
    // the next user of the socket. Set socket to REUSEADDR to be able to start
    // the server again.
    option_value = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_socket, (struct sockaddr *) &server_address, address_length) == -1) {
        perror("Error binding to socket");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_socket, PENDING_CONNECTIONS_QUEUE) == -1) {
        perror("Error setting connection to server mode");
        exit(EXIT_FAILURE);
    }
    
    signal(SIGCHLD, sig_chld); // handle waits for children
    setup_shm();

    for (process_counter = 0; process_counter < PREFORKED_PROCESSES; process_counter++) {

        pid = fork();

        if (pid == -1) {
            perror("Fork error");
            exit(EXIT_FAILURE);
        }

        if (pid == 0 ) {
            receive_requests(listen_socket);
            exit(EXIT_SUCCESS);
        }

    }

    // Clean up shared memory on termination, installed after fork in order to
    // be called only by the parent process
    signal(SIGINT, sig_int);

    while (1) { pause(); }

    exit(EXIT_SUCCESS);

}
