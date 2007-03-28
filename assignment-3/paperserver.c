#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


struct paper_struct {
    char author[256];
    char title[256];
    int size;
    char *data;
};


int paper_count = 0;
struct paper_struct **papers;
int papers_size = 0;


SERVER_PROC(details, in) {

    static details_out out;

    if (*in > paper_count) {
        out.author = "";
        out.title = "";
        return &out;
    }

    out.author = papers[*in - 1]->author;
    out.title = papers[*in - 1]->title;

    return &out;

}


SERVER_PROC(add, in) {

    struct paper_struct *paper;
    static add_out out;

    if (papers_size == 0) {
        papers = malloc(10 * sizeof(void*)); // TODO: malloc failure
        papers_size = 10;
    }

    if (paper_count == papers_size) {

        papers = realloc(papers, papers_size + (10 * sizeof(void*))); // TODO: failure
        papers_size += 10;

    }

    paper = malloc(sizeof(struct paper_struct)); // TODO: malloc failure
    paper->data = malloc(in->paper.paper_len);

    strcpy(paper->author, in->author);
    strcpy(paper->title, in->title);

    paper->size = in->paper.paper_len;
    memcpy(paper->data, in->paper.paper_val, in->paper.paper_len);

    papers[paper_count] = paper;

    /*
    fprintf(stderr, "Received paper from ``%s'' with title ``%s''\n", in->author, in->title);
    if (fwrite(in->paper.paper_val, in->paper.paper_len, 1, stdout) != 1) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
    fflush(stdout);
    fprintf(stderr, "Ok, paper_len: %d\n", in->paper.paper_len);
    */

    out = ++paper_count;

    return &out;

}
