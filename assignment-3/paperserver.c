#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#define PAPER_BUFFER_SIZE 10
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


get_out *get_proc_1_svc(get_in *in, struct svc_req *rqstp) {

    static get_out out;

    // TODO: check for NULL pointers

    out.error = 0;

    if (in->number < 1 || in->number > papers_count) {
        out.error = 1;
        out.get_out_u.reason = malloc(strlen(PAPER_NOT_FOUND) + 1);
        strcpy(out.get_out_u.reason, PAPER_NOT_FOUND);
        return &out;
    }

    // How nice of us to also include the paper number
    out.get_out_u.paper.number = malloc(sizeof(int));
    *out.get_out_u.paper.number = in->number;

    out.get_out_u.paper.author = papers[in->number - 1]->author;
    out.get_out_u.paper.title = papers[in->number - 1]->title;

    if (in->complete) {
        out.get_out_u.paper.content = malloc(sizeof(data)); // TODO: malloc error
        out.get_out_u.paper.content->data_len = papers[in->number - 1]->size;
        out.get_out_u.paper.content->data_val = papers[in->number - 1]->data;
    } else {
        out.get_out_u.paper.content = NULL;
    }

    return &out;

}


add_out *add_proc_1_svc(add_in *in, struct svc_req *rqstp) {

    struct paper_t *paper;
    static add_out out;

    out.error = 0;

    // Initialize paper buffer
    if (papers_size == 0) {
        papers = malloc(PAPER_BUFFER_SIZE * sizeof(void*)); // TODO: malloc failure
        papers_size = PAPER_BUFFER_SIZE;
    }

    // Resize paper buffer
    if (papers_count == papers_size) {
        papers = realloc(papers, papers_size + (PAPER_BUFFER_SIZE * sizeof(void*))); // TODO: malloc failure
        papers_size += PAPER_BUFFER_SIZE;
    }

    // TODO: check for NULL pointers

    paper = malloc(sizeof(struct paper_t)); // TODO: malloc failure
    paper->data = malloc(in->paper.content->data_len);

    strcpy(paper->author, in->paper.author);
    strcpy(paper->title, in->paper.title);

    paper->size = in->paper.content->data_len;
    memcpy(paper->data, in->paper.content->data_val, in->paper.content->data_len);

    papers[papers_count] = paper;

    papers_count++;

    // TODO: check if paper exists

    /*
    if (fwrite(in->paper.paper_val, in->paper.paper_len, 1, stdout) != 1) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
    fflush(stdout);
    fprintf(stderr, "Ok, paper_len: %d\n", in->paper.paper_len);
    */

    dprint("Received paper %d from ``%s'' with title ``%s''\n", papers_count, in->paper.author, in->paper.title);

    out.add_out_u.paper.content = NULL;

    out.add_out_u.paper.number = malloc(sizeof(int));
    *out.add_out_u.paper.number = papers_count;

    out.add_out_u.paper.author = papers[papers_count - 1]->author;
    out.add_out_u.paper.title = papers[papers_count - 1]->title;

    return &out;

}
