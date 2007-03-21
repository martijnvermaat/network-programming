#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <rpc/rpc.h>


void usage_error () {
    printf("Usage: paperclient add <hostname> <author> <title> <filename.{pdf|doc}>\n");
    printf("Usage: paperclient details <hostname> <number>\n");
    exit(EXIT_FAILURE);
}


void add (char *hostname, char *author, char *title, char *filename) {

    CLIENT *client;
    add_in in;
    add_out *out;

    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS, "tcp");

    if (client == NULL) {
        fprintf(stderr, "Error connecting to %s", hostname);
        clnt_pcreateerror("");
        exit(EXIT_FAILURE);
    }

    in.author = author;
    in.title = title;
    out = add_proc_1(&in, client);

    if (out == NULL) {
        fprintf(stderr, "Error querying %s", hostname);
        clnt_perror(client, "");
        clnt_destroy(client);
        exit(EXIT_FAILURE);
    }

    printf("Artikel toegevoegd onder nummer %d\n", *out);

    clnt_destroy(client);

}


void details (char *hostname, int number) {

    CLIENT *client;
    details_in in;
    details_out *out;

    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS, "tcp");

    if (client == NULL) {
        fprintf(stderr, "Error connecting to %s", hostname);
        clnt_pcreateerror("");
        exit(EXIT_FAILURE);
    }

    in = number;
    out = details_proc_1(&in, client);

    if (out == NULL) {
        fprintf(stderr, "Error querying %s", hostname);
        clnt_perror(client, "");
        clnt_destroy(client);
        exit(EXIT_FAILURE);
    }

    printf("Artikel ``%s'' van ``%s''\n", out->author, out->title);

    clnt_destroy(client);

}


int main (int argc, char **argv) {

    if (argc < 2) {
        usage_error();
    }

    if (!strcmp(argv[1], "add")) {

        if (argc < 6) {
            usage_error();
        }

        add(argv[2], argv[3], argv[4], argv[5]);
        
    } else if (!strcmp(argv[1], "details")) {

        if (argc < 3) {
            usage_error();
        }

        // TODO: error checking
        details(argv[2], atoi(argv[3]));

    } else {

        usage_error();

    }

    exit(EXIT_FAILURE);

}
