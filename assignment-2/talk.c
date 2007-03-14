#include "assignment2.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>


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


char *read_line(int fd) {

    char *buffer;
    int buffer_size;
    char *p; // pointer to current character in buffer

    int character;
    int length = 0;

    buffer = malloc(READ_BUFFER_SIZE);

    if (buffer == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }

    buffer_size = READ_BUFFER_SIZE;
    p = buffer;

    while (1) {
        if (read(fd, &character, 1) == -1) {
            perror("Read error");
            exit(EXIT_FAILURE);
        }

        if (character == EOF) {
            if (length > 0) { // we have a string to return
                *p = '\0';
            } else { // no string, return null
                free(buffer);
                buffer = NULL;
            }
            break;
        } else if (character == '\n') {
            *p = '\0';
            break;
        }
        /*
          If character == '\0' we read more than we process, but that's not
          too bad I guess.
        */

        *p = character; // put character in buffer
        p++;
        length++;

        // check if we need to resize the buffer
        if (length >= buffer_size) {
            buffer_size += READ_BUFFER_SIZE;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                /*
                  Without the exit() we should do a free() on the original
                  buffer (this would require some more lines though, because
                  we just lost the pointer to it...)
                */
                perror("Unable to allocate necessary memory");
                exit(EXIT_FAILURE);
            }
            p = buffer + length;
        }

    }

    // Go to a new line on EOF
    if (!buffer) {
        printf("\n");
    }

    return buffer;

}


void write_line (int fd, char *line) {

    char *p = line;
    int written = 0;
    int result;

    while (written < strlen(line)) {
        result = write(fd, p, strlen(p));
        if (result == -1) {
            perror("Write error");
            return; // TODO: break
        }
        written = written + result;
        p = p + result;
    }

    // TODO: keep \n in read_line()
    write(fd, "\n", 1);

}


void read_from_network (int socket) {

    pid_t pid;
    char *line;

    pid = fork();

    if (pid == -1) {
        perror("Fork error");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        line = read_line(socket);

        if (line == NULL) {
            exit(EXIT_SUCCESS);
        }

        printf("Ontvangen: ");
        write_line(STDOUT_FILENO, line);

        free(line);

        exit(EXIT_SUCCESS);

    }

}


void read_from_keyboard (int socket) {

    pid_t pid;
    char *line;

    pid = fork();

    if (pid == -1) {
        perror("Fork error");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        printf("* we gaan een line lezen (stdin)\n");

        line = read_line(STDIN_FILENO);

        printf("* we hebben een line gelezen (stdin)\n");

        if (line == NULL) {
            printf("* dit was EOF (stdin)\n");
            exit(EXIT_SUCCESS);
        }

        write_line(socket, line);

        free(line);

        printf("* dit was het voor deze child (stdin)\n");
        exit(EXIT_SUCCESS);

    }

}


void sig_chld (int sig) {
    while (waitpid(0, NULL, WNOHANG) > 0) {
        ;
    }
    signal(SIGCHLD, sig_chld);
}


int main (int argc, char **argv) {

    int socket;
    int nb;
    fd_set read_set;

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

    signal(SIGCHLD, sig_chld);

    while (1) {

        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(socket, &read_set);

        printf("voor select\n");

        // TODO highest fd is socket (?)
        nb = select(socket, &read_set, NULL, NULL, NULL);

        printf("na select\n");

        if (nb == -1) {
            perror("Error waiting for input");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &read_set)) {
            // if EOF exit
            printf("uit stdin\n");
            read_from_keyboard(socket);
        }

        if (FD_ISSET(socket, &read_set)) {
            // if EOF exit
            printf("uit socket\n");
            read_from_network(socket);
        }

    }

    if (close(socket) == -1) {
        perror("Error closing connection");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}
