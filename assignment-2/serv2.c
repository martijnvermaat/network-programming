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
int shm_id, mutex, listen_socket;
struct sembuf up = {0, 1, 0};
struct sembuf down = {0, -1, 0};


void sig_chld (int sig) {
    while (waitpid(0, NULL, WNOHANG) > 0) {
        ;
    }
    signal(SIGCHLD, sig_chld);
}


void sig_int (int sig) {
    // on termination, clean up shared memory
    /*
      TODO: if my experiments are correct, sig_int is not called
      when we do exit() ourselves...!
    */
    shmdt((void *) connection_counter); // detach from shared memory
    shmctl(shm_id, IPC_RMID, NULL); // remove shared memory
    semctl(mutex, 0, IPC_RMID); // remove semaphore
    close(listen_socket);
    exit(EXIT_SUCCESS);
}


void setup_shm (void) {
    shm_id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    connection_counter = (int *) shmat(shm_id, 0, 0);
    *connection_counter = 0;
}


void treat_request (int socket) {
    int written = 0, current = 0;

    semop(mutex, &down, 1); // mutual exclusion
    *connection_counter += 1;
    current = htonl(*connection_counter);
    semop(mutex, &up, 1); // release mutex

    while (written != sizeof(int)) { // make sure we write an int
        written = write(socket, (const void *) &current, sizeof(int));
        if (written == -1) {
            perror("Connection error");
            break;
        }
    }

    // detach from shared memory
    // TODO: move to fork()==0 {} code?
    shmdt((void *) connection_counter);
}


int main (int argc, char **argv) {

    int client_socket, option_value;
    struct sockaddr_in server_address, client_address;
    socklen_t address_length;

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
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value))) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

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
    signal(SIGINT, sig_int); // clean up shared memory on termination
    setup_shm();

    while (1) {
        client_socket = accept(listen_socket,(struct sockaddr *) &client_address, &address_length);
        if (client_socket < 0) {
            perror("Connection error");
            continue;
        }

        if (fork() == 0) { // child
            treat_request(client_socket);
            close(client_socket);
            exit(EXIT_SUCCESS);
        } else { // parent
            close(client_socket); // no need for duplicate sockets
        }
    }

    exit(EXIT_SUCCESS);

}
