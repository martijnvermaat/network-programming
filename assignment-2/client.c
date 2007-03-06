#define DEBUG 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif


#define SERVER_PORT 80


int _malloc_counter = 0;

void *counting_malloc(size_t siz) {
    _malloc_counter++;
    return malloc(siz);
}

void counting_free(void *mem) {
    _malloc_counter--;
    free(mem);
}

#define malloc counting_malloc
#define free counting_free


int main(int argc, char **argv) {

    struct hostent *server_resolve;
    struct in_addr *server_address;
    struct sockaddr_in server;
    size_t address_length;
    int server_socket;

    if (argc < 2) {
        printf("Usage: %s hostname\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
      We could also use getaddrinfo() and gai_strerror() which is not
      obsolete (like herror() is).
    */
    server_resolve = gethostbyname(argv[1]);
    if (server_resolve == NULL) {
        fprintf(stderr, "Address not found for %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    server_address = (struct in_addr*) server_resolve->h_addr_list[0];
    dprint("The IP address of %s is %s\n", argv[1], inet_ntoa(*server_address));

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
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // TODO: some code to get an int

    if (close(server_socket) == -1) {
        perror("Error closing connection");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}
