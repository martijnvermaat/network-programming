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
    int written = 0;
    connection_counter++;
    while(!written) {
        written = write(socket, (const void *)&connection_counter, sizeof(int));
        if(written == -1) {
            perror("Connection error");
            break;
        }
    }
}

int main(int argc, char **argv) {

    int cl_socket, newsock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;
    //struct in_addr inp;

    addrlen = sizeof(struct sockaddr_in);
    cl_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (cl_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    //inet_aton("127.0.0.1", &inp);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    //server_addr.sin_addr = inp;
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
        dprint("Accepting incoming connections\n");
        newsock = accept(cl_socket,(struct sockaddr *) &client_addr, &addrlen);
        dprint("Accepted connection\n");
        if(newsock < 0) {
            perror("Connection error");
            exit(EXIT_FAILURE);
        }           
        treat_request(newsock);
        dprint("Sent reply: %d\n", connection_counter);
        close(newsock);     
        dprint("Closed connection\n");
    }

    exit(EXIT_SUCCESS);

}
