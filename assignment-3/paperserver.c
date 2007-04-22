#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#define PAPER_BUFFER_SIZE 10
#define PAPER_NOT_FOUND "Paper not found"


/*
  The paper number is implicit in the location of the paper in the buffer
  because we number them successively and never remove one.
*/
struct paper_t {
    char author[256];
    char title[256];
    int size;
    char *data;
};


struct paper_t **papers;
int papers_size = 0;
int papers_count = 0;


/*
  TODO: Check XDR structures for NULL pointers, we should not trust rpcgen too
        much.
  TODO: Maybe check if paper exists by comparing author and title.
*/
add_out *add_proc_1_svc(add_in *in, struct svc_req *rqstp) {

    struct paper_t *paper;
    static add_out out;

    // We can only do so much, free the result of the previous call
    xdr_free((xdrproc_t) xdr_add_out, (char *) &out);

    out.result = STATUS_SUCCESS;

    // Initialize paper buffer
    if (papers_size == 0) {
        papers = malloc(PAPER_BUFFER_SIZE * sizeof(void*));
        if (papers == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        papers_size = PAPER_BUFFER_SIZE;
    }

    // Resize paper buffer
    if (papers_count == papers_size) {
        papers = realloc(papers, papers_size + (PAPER_BUFFER_SIZE * sizeof(void*)));
        if (papers == NULL) {
            // Remember to free() the original buffer if we would remove the
            // exit() call
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        papers_size += PAPER_BUFFER_SIZE;
    }

    paper = malloc(sizeof(struct paper_t));
    if (paper == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }
    paper->data = malloc(in->paper.content->data_len);
    if (paper->data == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }

    strcpy(paper->author, in->paper.author);
    strcpy(paper->title, in->paper.title);

    paper->size = in->paper.content->data_len;
    memcpy(paper->data, in->paper.content->data_val, in->paper.content->data_len);

    papers[papers_count] = paper;

    papers_count++;

    dprint("Received paper %d from ``%s'' with title ``%s''\n", papers_count, in->paper.author, in->paper.title);

    out.add_out_u.paper.content = NULL;

    out.add_out_u.paper.number = malloc(sizeof(int));
    if (out.add_out_u.paper.number == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }
    *out.add_out_u.paper.number = papers_count;

    // Yes, we keep on doing malloc()s instead of pointing to our fine data
    // structure. Being consistent in this way, makes it easy to free the
    // entire structure with a xdr_free() call.
    // (This is the way we do it in all procedures.)
    out.add_out_u.paper.author = malloc(strlen(papers[papers_count - 1]->author) + 1);
    if (out.add_out_u.paper.author == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }
    out.add_out_u.paper.title = malloc(strlen(papers[papers_count - 1]->title) +1);
    if (out.add_out_u.paper.title == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }

    strcpy(out.add_out_u.paper.author, papers[papers_count - 1]->author);
    strcpy(out.add_out_u.paper.title, papers[papers_count - 1]->title);

    return &out;

}


/*
  TODO: Check XDR structures for NULL pointers, we should not trust rpcgen too
        much.
*/
get_out *get_proc_1_svc(get_in *in, struct svc_req *rqstp) {

    static get_out out;

    // We can only do so much, free the result of the previous call
    xdr_free((xdrproc_t) xdr_get_out, (char *) &out);

    out.result = STATUS_SUCCESS;

    if (in->number < 1 || in->number > papers_count) {
        out.result = STATUS_FAILURE;
        out.get_out_u.reason = malloc(strlen(PAPER_NOT_FOUND) + 1);
        if (out.get_out_u.reason == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        strcpy(out.get_out_u.reason, PAPER_NOT_FOUND);
        return &out;
    }

    out.get_out_u.paper.number = malloc(sizeof(int));
    if (out.get_out_u.paper.number == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }
    *out.get_out_u.paper.number = in->number;

    out.get_out_u.paper.author = malloc(strlen(papers[in->number - 1]->author) + 1);
    if (out.get_out_u.paper.author == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }
    out.get_out_u.paper.title = malloc(strlen(papers[in->number - 1]->title) + 1);
    if (out.get_out_u.paper.title == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }

    strcpy(out.get_out_u.paper.author, papers[in->number - 1]->author);
    strcpy(out.get_out_u.paper.title, papers[in->number - 1]->title);

    if (in->representation == DETAILED) {
        out.get_out_u.paper.content = malloc(sizeof(data));
        if (out.get_out_u.paper.content == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        out.get_out_u.paper.content->data_val = malloc(papers[in->number - 1]->size);
        if (out.get_out_u.paper.content->data_val == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        memcpy(out.get_out_u.paper.content->data_val, papers[in->number - 1]->data, papers[in->number - 1]->size);
        out.get_out_u.paper.content->data_len = papers[in->number - 1]->size;
    } else {
        out.get_out_u.paper.content = NULL;
    }

    return &out;

}


/*
  TODO: Check XDR structures for NULL pointers, we should not trust rpcgen too
        much.
*/
list_out *list_proc_1_svc(void *in, struct svc_req *rqstp) {

    static list_out out;
    document_list paper_list;
    document_list *previous;
    int i;

    // We can only do so much, free the result of the previous call
    xdr_free((xdrproc_t) xdr_list_out, (char *) &out);

    out.papers = NULL;

    previous = &out.papers;

    for (i = 0; i < papers_count; i++) {

        paper_list = malloc(sizeof(document_node));
        if (paper_list == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        *previous = paper_list;

        paper_list->item.number = malloc(sizeof(int));
        if (paper_list->item.number == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        *paper_list->item.number = i + 1;

        paper_list->item.author = malloc(strlen(papers[i]->author) + 1);
        if (paper_list->item.author == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }
        paper_list->item.title = malloc(strlen(papers[i]->title) + 1);
        if (paper_list->item.title == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }

        strcpy(paper_list->item.author, papers[i]->author);
        strcpy(paper_list->item.title, papers[i]->title);

        // Don't send the entire document
        paper_list->item.content = NULL;

        previous = &paper_list->next;

    }

    *previous = NULL;

    return &out;

}
