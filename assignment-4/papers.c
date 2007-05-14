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


void printHTML(char *s) {

    int length = strlen(s);

    while (length > 0) {

        if (*s == '<') {
            printf("&lt;");
        } else if (*s == '&') {
            printf("&amp;");
        } else if (*s == '>') {
            printf("&gt;");
        } else {
            putchar(*s);
        }

        s++;

        length--;

    }

}


void paper_listing(char *hostname) {

    CLIENT *client;
    list_out *out;
    document_list papers;
    int paper_count = 0;

    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS,
                         "tcp");

    if (client == NULL) {
        printf("<p>Could not connect to paperserver at %s</p>\n", hostname);
        return;
    }

    out = list_proc_1(NULL, client);

    if (out == NULL) {
        printf("<p>Error querying %s</p>\n\n", hostname);
        clnt_destroy(client);
        return;
    }

    papers = out->papers;

    while (papers) {
        paper_count++;
        printf("<p>Paper %d<br>Author: ``", *(papers->item.number));
        printHTML(papers->item.author);
        printf("''<br>Title:  ``<a href=\"paperview.cgi?paper=%d\">",
               *(papers->item.number));
        printHTML(papers->item.title);
        printf("</a>''</p>\n");
        papers = papers->next;
    }

    if (paper_count == 1) {
        printf("<p>1 paper stored</p>\n");
    } else {
        printf("<p>%d papers stored\n</p>", paper_count);
    }

    clnt_destroy(client);

}


int main(int argc, char **argv) {

    time_t current_time;

    printf("Content-Type: text/html\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" ");
    printf("\"http://www.w3.org/TR/html4/strict.dtd\">\n");
    printf("<html lang=\"en\">\n\n<head>\n\t<title>\n\t\tPapers stored ");
    printf("at paperserver\n\t</title>\n\n</head>\n\n");
    printf("<body>\n\n<h1>Conference Website</h1><hr>");
    printf("<h2>Stored papers</h2>\n");

    paper_listing(HOSTNAME);

    time(&current_time);

    printf("<hr><address>Conference Website - %s</address>\n", ctime(&current_time));
    printf("</body>\n</html>\n");

    return (EXIT_SUCCESS);

}
