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
struct sockaddr_in server_addr, client_addr;
socklen_t addrlen;


void sig_chld(int sig) {
    while (waitpid(0, NULL, WNOHANG) > 0) {
        ;
    }
    signal(SIGCHLD,sig_chld);
}

void sig_int(int sig) {
    // on termination, clean up shared memory
    dprint("Terminating\n");
    shmdt((void *) connection_counter); // detach from shared memory
    shmctl(shm_id, IPC_RMID, 0); // remove shared memory
    semctl(mutex, 0, IPC_RMID); // remove semaphore
    exit(EXIT_SUCCESS);
}

void setup_shm(void) {
    shm_id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    connection_counter = (int *) shmat(shm_id, 0, 0);
    *connection_counter = 0;
}

void treat_request(int socket) {

    int current, result, written = 0;

    if (semop(mutex, &down, 1) == -1) {
        perror("Semaphore error");
        return;
    }

    *connection_counter += 1;
    current = htonl(*connection_counter);

    if (semop(mutex, &up, 1) < 0) {
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

void recv_requests(int socket) {
    int newsock;

    while(1) {
        newsock = accept(socket,(struct sockaddr *) &client_addr, &addrlen);
        if(newsock < 0) {
            perror("Connection error");
            continue;
        }
        connection_counter = (int *) shmat(shm_id, 0, 0);
        treat_request(newsock);
        shmdt((void *) connection_counter);    // detach from shared memory
        close(newsock);
    }
}

int main(int argc, char **argv) {
    int option, process_counter, cl_socket;
    pid_t pid;

    mutex = semget(IPC_PRIVATE, 1, 0600);
    semop(mutex, &up, 1);  // Initialize for mutual exclusion

    addrlen = sizeof(struct sockaddr_in);
    cl_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (cl_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // When one tries to invoke the server multiple times after eachother within
    // a small amount of time, one will find that the socket is still in the
    // TIME_WAIT state, in order to catch delayed packets and not to confuse
    // the next user of the socket. Set socket to REUSEADDR to be able to start
    // the server again.
    option = 1;
    setsockopt(cl_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(cl_socket, (struct sockaddr *) &server_addr, addrlen) < 0) {                
        perror("Error binding to socket");
        exit(EXIT_FAILURE);
    }
    
    if(listen(cl_socket,5) < 0) { // we use magic number 5
        perror("Error setting connection to server mode");
        exit(EXIT_FAILURE);
    }
    
    signal(SIGCHLD, sig_chld); // handle waits for children
    setup_shm();

    for (process_counter = 0; process_counter < NB_PROC; process_counter++) {
        pid = fork();
        if (pid == 0 ) {
            recv_requests(cl_socket);
            exit(EXIT_SUCCESS);
        } else if (pid == -1) {
            perror("Fork error");
            exit(EXIT_FAILURE);
        }
    }

    // clean up shared memory on termination, installed after fork in order to
    // be called only by the parent process
    signal(SIGINT, sig_int);

    while(1) { pause(); }

    exit(EXIT_SUCCESS);
}
