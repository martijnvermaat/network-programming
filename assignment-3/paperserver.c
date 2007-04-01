#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#define PAPER_NOT_FOUND "Paper not found"


struct paper_t {
    char author[256];
    char title[256];
    int size;
    char *data;
};


struct paper_t **papers;
int papers_size = 0;
int papers_count = 0;


SERVER_PROC(details, in) {

    static details_out out;

    // TODO: check for NULL pointers

    out.error = 0;

    if (in->number > papers_count) {
        out.error = 1;
        out.details_out_u.reason = malloc(strlen(PAPER_NOT_FOUND) + 1);
        strcpy(out.details_out_u.reason, PAPER_NOT_FOUND);
        return &out;
    }

    // How nice of us to also include the paper number
    out.details_out_u.paper.number = malloc(sizeof(int));
    *out.details_out_u.paper.number = in->number;

    // TODO: shouldn't we memcpy this?
    out.details_out_u.paper.author = papers[in->number - 1]->author;
    out.details_out_u.paper.title = papers[in->number - 1]->title;

    return &out;

}


SERVER_PROC(add, in) {

    struct paper_t *paper;
    static add_out out;

    out.error = 0;

    if (papers_size == 0) {
        papers = malloc(10 * sizeof(void*)); // TODO: malloc failure
        papers_size = 10;
    }

    if (papers_count == papers_size) {

        papers = realloc(papers, papers_size + (10 * sizeof(void*))); // TODO: failure
        papers_size += 10;

    }

    // TODO: check for NULL pointers

    paper = malloc(sizeof(struct paper_t)); // TODO: malloc failure
    paper->data = malloc(in->paper.content->data_len);

    strcpy(paper->author, in->paper.author);
    strcpy(paper->title, in->paper.title);

    paper->size = in->paper.content->data_len;
    memcpy(paper->data, in->paper.content->data_val, in->paper.content->data_len);

    papers[papers_count] = paper;

    // TODO: check if paper exists

    /*
    fprintf(stderr, "Received paper from ``%s'' with title ``%s''\n", in->author, in->title);
    if (fwrite(in->paper.paper_val, in->paper.paper_len, 1, stdout) != 1) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
    fflush(stdout);
    fprintf(stderr, "Ok, paper_len: %d\n", in->paper.paper_len);
    */

    papers_count++;

    out.add_out_u.paper.number = malloc(sizeof(int));
    *out.add_out_u.paper.number = papers_count;

    out.add_out_u.paper.author = papers[papers_count - 1]->author;
    out.add_out_u.paper.title = papers[papers_count - 1]->title;

    return &out;

}
