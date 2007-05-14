#include "assignment4.h"
#include "paperstorage.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <rpc/rpc.h>
#include <time.h>


void request_get_integer(char *var, int *store, int default_val) {

    char *method, *query_string, *current_var, **request_vars;
    int i = 0, req_size = REQUEST_SIZE, current_size = 0;

    method = getenv("REQUEST_METHOD");

    /* Check if existence of var is even possible */
    if(!(!strcmp(method, "GET") || !strcmp(method, "HEAD"))) {
        *store = default_val;
        return;
    }

    query_string = getenv("QUERY_STRING");

    /* Decode query string */
    for (i = 0; i < strlen(query_string); i++) {
        if(query_string[i] == '+') {
            query_string[i] = ' ';
        }
    }

    /* Retrieve tuples of vars and values */
    request_vars = malloc(REQUEST_SIZE * sizeof(char **)); 
    current_var = strtok(query_string, "&");
    while(current_var) {
        request_vars[current_size++] = strdup(current_var);
        if(current_size == req_size) {
            request_vars = realloc(request_vars,
                                   (req_size+REQUEST_SIZE) * sizeof(char **));
            req_size += REQUEST_SIZE;
        }
        current_var = strtok(NULL, "&");
    }

    /* Walk the array of var-value tuples for "var" */
    for (i = 0; i < current_size; i++) {
        current_var = strtok(request_vars[i], "=");
        if(!strcmp(current_var, var)) {
            current_var = strtok(NULL, "=");
            *store = atoi(current_var);
            return;
        }
    }

    /* Unfortunately we have not found it */
    *store = default_val;

}


void set_file_name(int number, char *mime_type) {

    char *p;
    p = strrchr(mime_type, '/');

    if (!strcmp(p+1, "pdf")) {
        printf("Content-Disposition: attachment; filename=\"paper-%d.pdf\"\r\n",
               number);
    } else {
        printf("Content-Disposition: attachment; filename=\"paper-%d.doc\"\r\n",
               number);
    }

}


void error_message(char *message) {

    time_t current_time;

    time(&current_time);
    printf("Status: 404 Paper not found\r\n");
    printf("Content-Type: text/html\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" ");
    printf("\"http://www.w3.org/TR/html4/strict.dtd\">\n");
    printf("<html lang=\"en\">\n\n<head>\n\t<title>\n\t\t");
    printf("Paper fetch error\n\t</title>\n\n</head>\n\n");
    printf("<body>\n\n<h1>Conference Website</h1>\n<hr>");
    printf("<h2>Error fetching paper</h2>\n");

    printf("<p>%s</p>\n\n", message);

    printf("<hr><address>Conference Website - %s</address>\n</body>\n</html>\n",
           ctime(&current_time));

}


char *fetch_paper(char *hostname, int number) {

    CLIENT *client;
    get_in in;
    get_out *out;
    char *error;
    char *mime_type;

    error = malloc(ERROR_STRING_SIZE * sizeof(char));

    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS,
                         "tcp");

    if (client == NULL) {
        snprintf(error, ERROR_STRING_SIZE, "Error connecting to %s",
                 hostname);
        return error;
    }

    in.number = number;
    in.representation = DETAILED;
    out = get_proc_1(&in, client);

    if (out == NULL) {
        snprintf(error, ERROR_STRING_SIZE, "Error querying %s", hostname);
        clnt_destroy(client);
        return error;
    }

    if (out->result == STATUS_FAILURE) {
        snprintf(error, ERROR_STRING_SIZE,
                 "Error requesting paper details: %s", out->get_out_u.reason);
        clnt_destroy(client);
        return error;
    }

    mime_type = out->get_out_u.paper.type;

    set_file_name(number, mime_type);
    printf("Content-Type: %s\r\n\r\n", mime_type);
    if (fwrite(out->get_out_u.paper.content->data_val,
               out->get_out_u.paper.content->data_len, 1, stdout) != 1) {
        clnt_destroy(client);
        return error;
    }

    clnt_destroy(client);
    return error;

}


int main(int argc, char **argv) {

    char *fetch_error;
    int paper_id = -1;

    request_get_integer("paper", &paper_id, -1);

    if (paper_id == -1) {
        error_message("Supplied invalid paper id");
    } else {
        fetch_error = fetch_paper(HOSTNAME, paper_id);
        if (strlen(fetch_error) > 0) {
            error_message(fetch_error);
            free(fetch_error);
        }
    }

    return (EXIT_SUCCESS);

}
