#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <rpc/rpc.h>


void usage_error () {
    printf("Usage: paperclient add <hostname> <author> <title> <filename.{pdf|doc}>\n");
    printf("Usage: paperclient details <hostname> <number>\n");
    exit(EXIT_FAILURE);
}


void add (char *hostname, char *author, char *title, char *filename) {

    CLIENT *client;
    struct rpc_err rpc_errno;
    add_in in;
    add_out *out;
    FILE *stream;
    struct stat file_stat;

    if (stat(filename, &file_stat) == -1) {
        perror("Cannot stat file");
        exit(EXIT_FAILURE);
    }

    in.paper.paper_len = file_stat.st_size;
    in.paper.paper_val = malloc(file_stat.st_size);

    if (in.paper.paper_val == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    stream = fopen(filename, "r");

    if (stream == NULL) {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    if (fread(in.paper.paper_val, file_stat.st_size, 1, stream) != 1) {

        if (feof(stream)) {
            fprintf(stderr, "File is unstable: %s\n", filename);
        } else {
            perror("Error reading from file");
        }
        exit(EXIT_FAILURE);

    }

    in.author = author;
    in.title = title;

    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS, "tcp");

    if (client == NULL) {
        fprintf(stderr, "Error connecting to %s", hostname);
        clnt_pcreateerror("");
        exit(EXIT_FAILURE);
    }

    out = add_proc_1(&in, client);

    if (out == NULL) {

        clnt_geterr(client, &rpc_errno);
        // TODO: use const HOHOI = 3444; in .x and HOHOI from .h
        if (rpc_errno.re_status == RPC_CANTENCODEARGS) {
            fprintf(stderr, "Invalid argument length\n");
        } else {
            fprintf(stderr, "Error querying %s", hostname);
            clnt_perror(client, "");
        }

        clnt_destroy(client);
        exit(EXIT_FAILURE);

    }

    printf("Artikel toegevoegd onder nummer %d\n", *out);

    clnt_destroy(client);

}


void details (char *hostname, char *number) {

    CLIENT *client;
    details_in in;
    details_out *out;

    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS, "tcp");

    if (client == NULL) {
        fprintf(stderr, "Error connecting to %s", hostname);
        clnt_pcreateerror("");
        exit(EXIT_FAILURE);
    }

    in = atoi(number);
    out = details_proc_1(&in, client);

    if (out == NULL) {
        fprintf(stderr, "Error querying %s", hostname);
        clnt_perror(client, "");
        clnt_destroy(client);
        exit(EXIT_FAILURE);
    }

    printf("Artikel ``%s'' van ``%s''\n", out->title, out->author);

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
        details(argv[2], argv[3]);

    } else {

        usage_error();

    }

    exit(EXIT_FAILURE);

}
