#include "assignment2.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int connection_counter = 0;

void treat_request(int socket) {
    int written = 0, current = 0;
    connection_counter++;
    current = htonl(connection_counter);
    while(written != sizeof(int)) {
        written = write(socket, (const void *) &current, sizeof(int));
        if(written == -1) {
            perror("Connection error");
            break;
        }
    }
}

int main(int argc, char **argv) {
    int cl_socket, newsock, option;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;

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
    
    while(1) {
        newsock = accept(cl_socket,(struct sockaddr *) &client_addr, &addrlen);
        if(newsock < 0) {
            perror("Connection error");
            continue;
        }           
        treat_request(newsock);
        close(newsock);
    }

    exit(EXIT_SUCCESS);

}
