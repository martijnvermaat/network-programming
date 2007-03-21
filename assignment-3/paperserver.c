#include "assignment3.h"
#include "paperstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


SERVER_PROC(details) {

    static details_out out;

    out.author = "E. Wattel";
    out.title = "Cool Beards and More";

    return &out;

}
