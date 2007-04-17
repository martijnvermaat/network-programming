#include "assignment3.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>


#define SERVER_PORT 3242
#define READ_BUFFER_SIZE 20
#define AVAILABILITY_FORMAT "%s %f %i"
#define LIST_REQUEST "list\n\n"
#define GUESTS_REQUEST "guests\n\n"
#define BOOK_REQUEST "book\n%s\n%s\n\n"


void usage_error () {
    printf("Usage: hotelgwclient list <hostname>\n");
    printf("       hotelgwclient book <hostname> [type] <guest>\n");
    printf("       hotelgwclient guests <hostname>\n");
    exit(EXIT_FAILURE);
}


char *read_line(int fd) {

    char *buffer;
    int buffer_size;
    char *p;
    int bytes_read;

    char character;
    int length = 0;

    buffer = malloc(READ_BUFFER_SIZE);

    if (buffer == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }

    buffer_size = READ_BUFFER_SIZE;
    p = buffer;

    while (1) {

        bytes_read = read(fd, &character, 1);
        if (bytes_read == -1) {
            perror("Read error");
            exit(EXIT_FAILURE);
        }

        if (bytes_read == 0) {
            if (length > 0) { // we have a string to return
                *p = '\0';
            } else { // no string, return null
                free(buffer);
                buffer = NULL;
            }
            break;
        }

        if (character == '\n') {
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
            exit(EXIT_FAILURE);
        }
        written = written + result;
        p = p + result;
    }

}


void print_availability (char *availability) {

    char *type = malloc(strlen(availability) + 1);
    float price;
    int number;

    if (sscanf(availability, AVAILABILITY_FORMAT, type, &price, &number) != 3) {
        fprintf(stderr, "Cannot read response");
    }

    printf("%d rooms of type %s at %.2f euros per night\n", number, type, price);

}


int connect_to_gateway (char *hostname) {

    struct hostent *server_resolve;
    struct in_addr *server_address;
    struct sockaddr_in server;
    size_t address_length;
    int server_socket;

    server_resolve = gethostbyname(hostname);
    if (server_resolve == NULL) {
        fprintf(stderr, "Address not found for %s\n", hostname);
        exit(EXIT_FAILURE);
    }

    server_address = (struct in_addr*) server_resolve->h_addr_list[0];

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(inet_ntoa(*server_address));

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    address_length = sizeof(struct sockaddr_in);

    if (connect(server_socket, (struct sockaddr *) &server, address_length) == -1) {
        perror("Error connecting to gateway");
        exit(EXIT_FAILURE);
    }

    return server_socket;

}


void list (char *hostname) {

    int server_socket;
    char *request = LIST_REQUEST;
    char *line;

    server_socket = connect_to_gateway(hostname);

    write_line(server_socket, request);

    // todo network to host conversion?

    line = read_line(server_socket);

    if (!line) {
        fprintf(stderr, "Cannot read response\n");
        exit(EXIT_FAILURE);
    }

    if (*line != '0') {
        fprintf(stderr, "Error response: %s\n", line + 2); // todo check if +2 exists
        exit(EXIT_FAILURE);
    }

    do {
        free(line);
        line = read_line(server_socket);
        if (!line) {
            fprintf(stderr, "Cannot read response\n");
            exit(EXIT_FAILURE);
        }
        if (strlen(line) > 0) {
            print_availability(line);
        }
    } while (strlen(line) > 0);

    free(line);

    if (close(server_socket) == -1) {
        perror("Error closing connection");
        exit(EXIT_FAILURE);
    }

}


void book (char *hostname, char *type, char *guest) {

    int server_socket;
    char *request = malloc(strlen(BOOK_REQUEST) + strlen(type) + strlen(guest) + 1);
    char *line;

    snprintf(request, strlen(BOOK_REQUEST) + strlen(type) + strlen(guest) + 1, BOOK_REQUEST, type, guest);

    server_socket = connect_to_gateway(hostname);

    write_line(server_socket, request);

    // todo network to host conversion?

    line = read_line(server_socket);

    if (!line) {
        fprintf(stderr, "Cannot read response\n");
        exit(EXIT_FAILURE);
    }

    if (*line != '0') {
        fprintf(stderr, "Error response: %s\n", line + 2); // todo check if +2 exists
        exit(EXIT_FAILURE);
    }

    free(line);

    line = read_line(server_socket);

    if (strlen(line) != 0) {
        fprintf(stderr, "Cannot read response\n");
        exit(EXIT_FAILURE);
    }

    free(line);

    if (close(server_socket) == -1) {
        perror("Error closing connection");
        exit(EXIT_FAILURE);
    }

}


void guests (char *hostname) {

    int server_socket;
    char *request = GUESTS_REQUEST;
    char *line;

    server_socket = connect_to_gateway(hostname);

    write_line(server_socket, request);

    // todo network to host conversion?

    line = read_line(server_socket);

    if (!line) {
        fprintf(stderr, "Cannot read response\n");
        exit(EXIT_FAILURE);
    }

    if (*line != '0') {
        fprintf(stderr, "Error response: %s\n", line + 2); // todo check if +2 exists
        // TODO: read empty line
        exit(EXIT_FAILURE);
    }

    do {
        free(line);
        line = read_line(server_socket);
        if (!line) {
            fprintf(stderr, "Cannot read response\n");
            exit(EXIT_FAILURE);
        }
        if (strlen(line) > 0) {
            printf("%s\n", line);
        }
    } while (strlen(line) > 0);

    free(line);

    if (close(server_socket) == -1) {
        perror("Error closing connection");
        exit(EXIT_FAILURE);
    }

}


int main (int argc, char **argv) {

    if (argc < 2) {
        usage_error();
    }

    if (!strcmp(argv[1], "list")) {

        if (argc < 3) {
            usage_error();
        }

        list(argv[2]);

    } else if (!strcmp(argv[1], "book")) {

        if (argc < 4) {
            usage_error();
        }

        if (argc > 4) {
            book(argv[2], argv[3], argv[4]);
        } else {
            //book_any(argv[2], argv[3]); // TODO book_any
        }

    } else if (!strcmp(argv[1], "guests")) {

        if (argc < 3) {
            usage_error();
        }

        guests(argv[2]);

    } else {

        usage_error();

    }

    exit(EXIT_FAILURE);

}
