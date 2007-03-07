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

void sig_chld(int sig) {
    while(waitpid(0, NULL, WNOHANG) > 0) {
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
    int written = 0;
    int current = 0;
   
    semop(mutex, &down, 1); // mutual exclusion
        dprint("counter: %d - current: %d\n", *connection_counter, current);
        *connection_counter += 1;
        current = *connection_counter;
        dprint("counter: %d - current: %d\n", *connection_counter, current);
    semop(mutex, &up, 1); // release mutex
    
    while(!written) {
        written = write(socket, (const void *) &current, sizeof(int));
        if(written == -1) {
            perror("Connection error");
            break;
        }
    }
    
    // detach from shared memory
    shmdt((void *) connection_counter);
}

int main(int argc, char **argv) {
    int cl_socket, newsock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;

    mutex = semget(IPC_PRIVATE, 1, 0600);
    semop(mutex, &up, 1);  // Initialize for mutual exclusion

    addrlen = sizeof(struct sockaddr_in);
    cl_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (cl_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
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
    signal(SIGINT, sig_int); // clean up shared memory on termination
    setup_shm();
    
    while(1) {
        dprint("Accepting incoming connections\n");
        newsock = accept(cl_socket,(struct sockaddr *) &client_addr, &addrlen);
        dprint("Accepted connection\n");
        if(newsock < 0) {
            perror("Connection error");
            continue; //exit(EXIT_FAILURE);
        }

        if (fork() == 0) { // child
            dprint("Forked off child to treat request\n");
            treat_request(newsock);
            dprint("Finished request\n");
            exit(0);
        } else { // parent
            close(newsock); // no need for duplicate sockets
        }
    }

    exit(EXIT_SUCCESS);

}
