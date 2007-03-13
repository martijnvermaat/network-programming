#include "assignment2.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int start_client (char *hostname) {

    struct hostent *server_resolve;
    struct in_addr *server_address;
    struct sockaddr_in server;
    size_t address_length;
    int server_socket;

    server_resolve = gethostbyname(hostname);
    if (server_resolve == NULL) {
        fprintf(stderr, "Address not found for %s\n", hostname);
        return -1;
    }

    server_address = (struct in_addr*) server_resolve->h_addr_list[0];

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(inet_ntoa(*server_address));

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        perror("Error creating socket");
        return -1;
    }

    address_length = sizeof(struct sockaddr_in);

    if (connect(server_socket, (struct sockaddr *) &server, address_length) == -1) {
        perror("Error connecting to server");
        return -1;
    }

    return server_socket;

}


int start_server () {

    int listen_socket, client_socket;
    int option_value;
    struct sockaddr_in server_address, client_address;
    socklen_t address_length;

    address_length = sizeof(struct sockaddr_in);
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_socket == -1) {
        perror("Error creating socket");
        return -1;
    }

    option_value = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) == -1) {
        perror("Error setting socket option");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_socket, (struct sockaddr *) &server_address, address_length) == -1) {                
        perror("Error binding to socket");
        return -1;
    }

    if(listen(listen_socket, PENDING_CONNECTIONS_QUEUE) == -1) {
        perror("Error setting connection to server mode");
        return -1;
    }

    client_socket = accept(listen_socket, (struct sockaddr *) &client_address, &address_length);

    if (client_socket == -1) {
        perror("Connection error");
        return -1;
    }

    return client_socket;

}


void read_from_network (int socket) {

}


void read_from_keyboard (int socket) {

}


int main (int argc, char **argv) {

    int socket;

    pthread_t thread_id;
    pthread_attr_t thread_attr;

    if (argc < 2) {
        dprint("We are the server!\n");
        socket = start_server();
    } else {
        dprint("We are the client!\n");
        socket = start_client(argv[1]);
    }

    if (socket == -1) {
        dprint("We were neither!\n");
        exit(EXIT_FAILURE);
    }

    // chatin' 'round, oh la die jee
    dprint("chatin' 'round, oh la die jee\n");

    pthread_attr_init(&thread_attr);
    pthread_create(&thread_id, &thread_attr, read_from_network, (void *) NULL);

    read_from_keyboard(socket);

    pthread_join(thread_id, NULL);

    if (close(socket) == -1) {
        perror("Error closing connection");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}
