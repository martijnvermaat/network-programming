/* XDR Data Description for a Paper Storage Protocol */


/* Constant definitions */

const MAX_AUTHOR_LENGTH  = 255;
const MAX_TITLE_LENGTH   = 255;
const MAX_MESSAGE_LENGTH = 255;


/* Basic datatypes */

typedef opaque data <>;

struct document {
    int    *number                       ;
    string  author   <MAX_AUTHOR_LENGTH> ;
    string  title    <MAX_TITLE_LENGTH>  ;
    data   *content                      ;
};

typedef string message <MAX_MESSAGE_LENGTH>;


/* ADD_PROC input and output types */

struct add_in {
    document paper;
};

union add_out switch(int error) {
    case 0:
        document paper;
    default:
        message reason;
};


/* DETAILS_PROC input and output types */

struct details_in {
    int number;
};

union details_out switch(int error) {
    case 0:
        document paper;
    default:
        message reason;
};


/* Program and procedures declaration */

program PAPERSTORAGE_PROG {
    version PAPERSTORAGE_VERS {
        add_out     ADD_PROC(add_in)         = 1;
        details_out DETAILS_PROC(details_in) = 2;
    } = 1;
} = 0x3234763;
