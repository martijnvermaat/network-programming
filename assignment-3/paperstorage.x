typedef int details_in;

struct details_out {
    string author <255>;
    string title  <255>;
};

program PAPERSTORAGE_PROG {
    version PAPERSTORAGE_VERS {
        details_out DETAILS_PROC(details_in) = 1;
    } = 1;
} = 0x3234763;
