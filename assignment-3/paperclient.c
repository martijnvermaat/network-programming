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
    printf("       paperclient details <hostname> <number>\n");
    exit(EXIT_FAILURE);
}


void add (char *hostname, char *author, char *title, char *filename) {

    CLIENT *client;
    add_in in;
    add_out *out;
    FILE *stream;
    struct stat file_stat;

    if (strlen(author) > MAX_AUTHOR_LENGTH) {
        fprintf(stderr, "Maximum length for author is %d\n", MAX_AUTHOR_LENGTH);
        exit(EXIT_FAILURE);
    }

    if (strlen(title) > MAX_TITLE_LENGTH) {
        fprintf(stderr, "Maximum length for title is %d\n", MAX_TITLE_LENGTH);
        exit(EXIT_FAILURE);
    }

    if (stat(filename, &file_stat) == -1) {
        perror("Cannot stat file");
        exit(EXIT_FAILURE);
    }

    in.paper.number = NULL;
    in.paper.content->data_len = file_stat.st_size; // TODO: check for NULL pointers
    in.paper.content->data_val = malloc(file_stat.st_size);

    if (in.paper.content->data_val == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    stream = fopen(filename, "r");

    if (stream == NULL) {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    if (fread(in.paper.content->data_val, file_stat.st_size, 1, stream) != 1) {

        if (feof(stream)) {
            fprintf(stderr, "File is unstable: %s\n", filename);
        } else {
            perror("Error reading from file");
        }
        exit(EXIT_FAILURE);

    }

    in.paper.author = author;
    in.paper.title = title;

    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS, "tcp");

    if (client == NULL) {
        fprintf(stderr, "Error connecting to %s", hostname);
        clnt_pcreateerror("");
        exit(EXIT_FAILURE);
    }

    out = add_proc_1(&in, client);

    if (out == NULL) {
        fprintf(stderr, "Error querying %s", hostname);
        clnt_perror(client, "");
        clnt_destroy(client);
        exit(EXIT_FAILURE);
    }

    if (out->error) {
        fprintf(stderr, "Error adding paper: %s\n", out->add_out_u.reason);
        clnt_destroy(client);
        exit(EXIT_FAILURE);
    }

    printf("Artikel toegevoegd onder nummer %d\n", *(out->add_out_u.paper.number));

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

    in.number = atoi(number);
    out = details_proc_1(&in, client);

    if (out == NULL) {
        fprintf(stderr, "Error querying %s", hostname);
        clnt_perror(client, "");
        clnt_destroy(client);
        exit(EXIT_FAILURE);
    }

    if (out->error) {
        fprintf(stderr, "Error requesting paper details: %s\n", out->details_out_u.reason);
        clnt_destroy(client);
        exit(EXIT_FAILURE);
    }

    printf("Artikel ``%s'' van ``%s''\n", out->details_out_u.paper.title, out->details_out_u.paper.author);

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

        details(argv[2], argv[3]);

    } else {

        usage_error();

    }

    exit(EXIT_FAILURE);

}
