#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int last_paper = 0;


SERVER_PROC(details) {

    static details_out out;

    out.author = "E. Wattel";
    out.title = "Cool Beards and More";

    return &out;

}


SERVER_PROC(add) {

    static add_out out;

    last_paper++;

    printf("Received paper from ``%s'' with title ``%s''\n", in->author, in->title);

    out = last_paper;

    return &out;

}
