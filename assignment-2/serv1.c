#include "assignment2.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


void treat_request (int socket, int connection_counter) {

    int current, result, written = 0;

    current = htonl(connection_counter);

    while (written < sizeof(int)) {
        result = write(socket, (const void *) (&current + written), sizeof(int) - written);
        if (result == -1) {
            perror("Write error");
            break;
        }
        written = written + result;
    }

}


int main (int argc, char **argv) {

    int listen_socket, client_socket;
    int option_value, connection_counter;
    struct sockaddr_in server_address, client_address;
    socklen_t address_length;

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
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) == -1) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_socket, (struct sockaddr *) &server_address, address_length) == -1) {                
        perror("Error binding to socket");
        exit(EXIT_FAILURE);
    }

    if(listen(listen_socket, PENDING_CONNECTIONS_QUEUE) == -1) {
        perror("Error setting connection to server mode");
        exit(EXIT_FAILURE);
    }

    connection_counter = 0;

    while(1) {

        client_socket = accept(listen_socket, (struct sockaddr *) &client_address, &address_length);

        if (client_socket == -1) {
            perror("Connection error");
            continue;
        }

        connection_counter += 1;

        treat_request(client_socket, connection_counter);

        if (close(client_socket) == -1) {
            perror("Error closing connection");
        }

    }

    if (close(listen_socket) == -1) {
        perror("Error closing socket");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}
