struct add_in {
    string author <255>;
    string title  <255>;
    opaque paper  <>;
};

typedef int add_out;

typedef int details_in;

struct details_out {
    string author <255>;
    string title  <255>;
};

program PAPERSTORAGE_PROG {
    version PAPERSTORAGE_VERS {
        add_out     ADD_PROC(add_in)         = 1;
        details_out DETAILS_PROC(details_in) = 2;
    } = 1;
} = 0x3234763;
