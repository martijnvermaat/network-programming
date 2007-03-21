#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <rpc/rpc.h>


int main (int argc, char **argv) {

    CLIENT *client;
    details_in in;
    details_out *out;

    if (argc < 4) {
        printf("Usage: %s <command> <hostname> <number>\n", argv[0]);
        return 1;
    }

    client = clnt_create(argv[2], PAPERSTORAGE_PROG, PAPERSTORAGE_VERS, "tcp");
    in = atoi(argv[3]); // TODO: error checking
    out = details_proc_1(&in, client);

    if (out == NULL) {
        printf("Error: %s\n", clnt_sperror(client, argv[2]));
    } else {
        printf("Artikel ``%s'' van ``%s''\n", out->author, out->title);
    }

    clnt_destroy(client);

    return 0;

}
