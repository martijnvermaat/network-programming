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


struct fd_input {
    char *name;
    char *value;
    struct fd_input *next;
};
typedef struct fd_input form_data_input;

struct fd_input_file {
    char *name;
    char *filename;
    char *contents;
    int size;
    char *mime;
    struct fd_input_file *next;
};
typedef struct fd_input_file form_data_file;

char *boundary;
form_data_input *form_input;
form_data_file *files;


char *unquote(char *quoted) {
    *(quoted + strlen(quoted) - 1) = 0; // chop off last quote;
    return (quoted + 1);
}


char *plustospace(char *plus) {
    int i;
    for (i = 0; i < strlen(plus); i++) {
        if (plus[i] == '+') {
            plus[i] = ' ';
        }
    }
    return plus;
}


char *sanitize(char *san) {

    int i;

    // remove trailing spaces
    for (i = strlen(san); i >= 0; i--) {
        if (*(san + i) == ' ' || *(san + i) == '\r' || *(san + i) == '\n') {
            *(san + i) = 0;
        }
    }

    // remove quotes
    return unquote(san);

}


char *read_line(int fd, int *result) {

    char *buffer;
    int buffer_size;
    char *p;
    int bytes_read;

    char character;
    int length = 0;

    buffer = malloc(READ_BUFFER_SIZE);

    if (buffer == NULL) {
        printf("Could not allocate needed memory\n");
        exit(EXIT_FAILURE);
    }

    buffer_size = READ_BUFFER_SIZE;
    p = buffer;

    while (1) {

        bytes_read = read(fd, &character, 1);
        if (bytes_read == -1) {
            printf("Read error");
            exit(EXIT_FAILURE);
        }

        if (bytes_read == 0) {
            if (length > 0) { // we have a string to return
                *p = '\0';
            } else { // no string, return null
                free(buffer);
                *result = 0;
                buffer = NULL;
            }
            break;
        }

        if (character == '\n') {
            *p = '\n';
            p++;
            *p = '\0';
            length++;
            break;
        }
        /*
          If character == '\0' we read more than we process, but that's not
          too bad I guess.
        */

        *p = character; // put character in buffer
        p++;
        length++;

        // check if we need to resize the buffer
        if (length >= buffer_size - 1) {
            buffer_size += READ_BUFFER_SIZE;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                perror("Unable to allocate necessary memory");
                exit(EXIT_FAILURE);
            }
            p = buffer + length;
        }

    }
    *result = length;
    return buffer;

}


void error_message(char *message) {
    printf("<h3>Error uploading paper</h3>\n");
    printf("<p>%s</p>\n\n", message);
}


int add_paper(char *hostname, char *author, 
              char *title, char *filename, char* buffer, 
              int size, char* mime, char *result) {

    CLIENT *client;
    add_in in;
    add_out *out;
    char *extension;

    if (strlen(author) > MAX_AUTHOR_LENGTH) {
        snprintf(result, ERROR_STRING_SIZE, 
                 "Maximum length for author is %d\n", MAX_AUTHOR_LENGTH);
        return -1;
    }

    if (strlen(title) > MAX_TITLE_LENGTH) {
        snprintf(result, ERROR_STRING_SIZE,
                 "Maximum length for title is %d\n", MAX_TITLE_LENGTH);
        return -1;
    }

    extension = strrchr(filename, '.');
    if (extension == NULL || !(!strcmp(extension + 1, "pdf") || !strcmp(extension + 1, "doc"))) {
        snprintf(result, ERROR_STRING_SIZE, 
                 "Paper must have pdf or doc file extension\n");
        return -1;
    }

    in.paper.number = NULL;
    in.paper.content = malloc(sizeof(data));
    if (in.paper.content == NULL) {
        snprintf(result, ERROR_STRING_SIZE,
                 "Unable to allocate necessary memory");
        return -1;
    }
    in.paper.content->data_len = size;
    in.paper.content->data_val = malloc(size * sizeof(char));
    if (in.paper.content->data_val == NULL) {
        snprintf(result, ERROR_STRING_SIZE, "Error allocating memory");
        return -1;
    }

    memcpy(in.paper.content->data_val, buffer, size);
    in.paper.author = author;
    in.paper.title = title;
    in.paper.type = mime;
    client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS,"tcp");

    if (client == NULL) {
        snprintf(result, ERROR_STRING_SIZE,
                 "Error connecting to paperserver at %s", hostname);
        return -1;
    }

    out = add_proc_1(&in, client);

    if (out == NULL) {
        snprintf(result, ERROR_STRING_SIZE,
                 "Error querying %s", hostname);
        clnt_destroy(client);
        return -1;
    }

    if (out->result == STATUS_FAILURE) {
        snprintf(result, ERROR_STRING_SIZE,
                 "Error adding paper: %s\n", out->add_out_u.reason);
        clnt_destroy(client);
        return -1;
    }

    snprintf(result, ERROR_STRING_SIZE, "%d", *(out->add_out_u.paper.number));
    clnt_destroy(client);

    return 1;

}


void show_form(void) {

    printf("<form id=\"uploadpaper\" method=\"post\" action=\"paperload.cgi\"");
    printf("enctype=\"multipart/form-data\">\n");
    printf("\t<label for=\"file\">Paper:</lable>\n");
    printf("\t<input type=\"file\" name=\"file\" id=\"file\">\n");

    printf("<br>");

    printf("\t<label for=\"author\">Author:</lable>\n");
    printf("\t<input type=\"text\" maxlength=\"254\" name=\"author\" ");
    printf("id=\"author\">\n");

    printf("<br>");

    printf("\t<label for=\"title\">Paper title:</lable>\n");
    printf("\t<input type=\"text\" maxlength=\"254\" name=\"title\" ");
    printf("id=\"title\">\n");

    printf("<br>");
    printf("<input type=\"submit\" value=\"Upload paper\">\n");
    printf("</form>\n");

}

 
void read_file_contents(form_data_file *current,
                        char *name, char* file, char *mime) {

     form_data_file *temp;
     char *buffer, *line;
     int size = 0, readbytes = 0;

     temp =  malloc(sizeof(form_data_file));
     buffer = malloc(sizeof(char));
     current->name = malloc(strlen(name));
     current->filename = malloc(strlen(file));
     current->mime = malloc(strlen(mime));

     if(temp == NULL || buffer == NULL || current->name == NULL ||
        current->filename == NULL || current->mime == NULL ) {
         printf("Could not allocate needed memory\n");
         exit(EXIT_FAILURE);
     }

     strcpy(current->name, name);
     strcpy(current->filename, file);
     strcpy(current->mime, mime);

     line = read_line(STDIN_FILENO, &readbytes);
     if(line != NULL) {
         free(line); // blank line
     }

     while ((line = read_line(STDIN_FILENO, &readbytes)) != NULL) {

         if(strstr(line, boundary) != NULL) { // at boundary?
             free(line);
             *(buffer + size - 1) = 0; // remove last newline 
             current->contents = buffer;
             current->size = size - 2;
             current->next = temp;
             return;
         }

         buffer = realloc(buffer, size + readbytes); // resize buffer
         if(buffer == NULL) {
             printf("Could not allocate needed memory\n");
         }
         memcpy(buffer + size, line, readbytes); // copy string
         size += readbytes; // adjust size administration
         free(line); 

     }

}

 
void read_input_contents(form_data_input *current, char *name) {

     form_data_input *temp;
     char *buffer, *line;
     int size = 0, readbytes = 0;
     temp =  malloc(sizeof(form_data_input));
     buffer = malloc(sizeof(char));
     current->name = malloc(strlen(name));

     if(temp == NULL || buffer == NULL || current->name == NULL) {
         printf("Could not allocate needed memory\n");
         exit(EXIT_FAILURE);
     }

     strcpy(current->name, name);

     line = read_line(STDIN_FILENO, &readbytes);
     if(line != NULL) {
         free(line); // blank line
     }

     while ((line = read_line(STDIN_FILENO, &readbytes)) != NULL) {

         if(strstr(line, boundary) != NULL) { // at boundary?
             free(line);
             *(buffer+size-2) = 0; // chop off \r\n
             current->value = buffer;
             current->next = temp;
             return;
         }

         buffer = realloc(buffer, size + readbytes + 1); // resize buffer
         if(buffer == NULL) {
             printf("Could not allocate needed memory\n");
             exit(EXIT_FAILURE);
         }
 
         memcpy(buffer + size, line, readbytes); // copy string
         *(buffer + size + readbytes) =  0; // make sure it ends with \0
         size += readbytes; // adjust size administration
         free(line); 

     }

}   

 
void read_boundary() {

     char *line;
     int readbytes = 0;

     while((line = read_line(STDIN_FILENO, &readbytes)) != NULL) {
         if(strstr(line, boundary) != NULL ) { //boundary found
             free(line);
             return;
         }
         free(line);
     }

}


int get_var_file(const char *varname, char **filename, char **buffer,
                 char **mime, int *size) {

    form_data_file *current = files;

    while((current->name) != NULL) {
        if(!strcmp(current->name, varname)) {
            if(current->size == 0) {
                return -1;
            }
            *filename = current->filename;
            *buffer = current->contents;
            *size = current->size;
            *mime = current->mime;
            return 0;
        }
    }

    return -1;

}

 
int get_var_input(const char *varname, char *dest, int max_size) {

    form_data_input *current = form_input;

    while (current->name != NULL) {
        if (!strcmp(current->name, varname)) {
            if (strlen(current->value) == 0) {
                return -1;
            } else if (strlen(current->value) > max_size) {
                return -2;
            } else {
                strcpy(dest, current->value);
                return 0;
            }
        }
        current = current->next;
    }

    return -1;

}


void process_post_query(void) {

    char *type, *line;
    int readbytes = 0;
    form_data_input *fi_current = form_input;
    form_data_file *files_current = files;

    type = getenv("CONTENT_TYPE");

    if(strcmp(strtok(type,";"), "multipart/form-data")) {
        error_message("Sorry, we can only deal with multipart/form-data.");
        return;
    }

    /* retrieve boundary */
    boundary = strtok(NULL, ";"); // " boundary=----123"
    boundary = strtok(boundary, "="); // " boundary"
    boundary = strtok(NULL, "="); // "----123"

    /* Since POST data has to be read from stdin, we have only one chance of 
       getting all data. So we'll go over it once saving everything we come
       across */

    read_boundary();

    while((line = read_line(STDIN_FILENO, &readbytes)) != NULL) {

        char *name;
        char *filename;
        char *mime;

        if(strstr(line, "Content-Disposition: form-data; ") == NULL) {
            // expected form data
            free(line);
            read_boundary();
            continue;
        }

        // "Content-Dis... name=inputname; filename=filename.ext"
        strtok(line, "="); // "Content-Dis... name"
        name = strtok(NULL, "="); // "inputname; filename=filename.ext";

        if (strstr(name, ";") != NULL) { // ah, we're dealing with a file
            // fetch filename and mime type
            char *magicwand;

            magicwand = strstr(name, ";");
            if(magicwand != NULL) {
                *magicwand = 0;
            }

            // " filename=filename.ext"
            filename = strtok(NULL, "="); // "filename.ext"
            line = read_line(STDIN_FILENO, &readbytes); // need to get mime type
            if(line == NULL || strstr(line, "Content-Type: ") == NULL) {
                // expected mime type
                free(line);
                read_boundary();
                continue;
            }

            // "Content-Type: mime/type"
            strtok(line, ": "); // "Content-Type"
            mime = strtok(NULL, ": "); // "mime/type"

            magicwand = strstr(mime, "\r"); // chop off trailing newlines
            if(magicwand != NULL ) { 
                *magicwand = 0;
            }

            read_file_contents(files_current, sanitize(name), sanitize(filename), mime);
            files_current = files_current->next;
        } else {
            read_input_contents(fi_current, sanitize(name));
            fi_current = fi_current->next;
        }
        free(line);

    }

}


void handle_upload(void) {

    char author[MAX_AUTHOR_LENGTH], title[MAX_TITLE_LENGTH];
    char *filename, *filecontents, *mime, *add_result;
    int size;  

    form_input = malloc(sizeof(form_data_input));
    files = malloc(sizeof(form_data_file));
    add_result = malloc(ERROR_STRING_SIZE);

    if(form_input == NULL || files == NULL || add_result == NULL) {
        printf("Could not allocate needed memory\n");
        exit(EXIT_FAILURE);
    }

    process_post_query();

    switch(get_var_file("file", &filename, &filecontents, &mime, &size)) {
    case -1: error_message("You didn't send a paper"); return;
    default: break; // all is well
    }

    if (strlen(mime) > MAX_TYPE_LENGTH) {
        error_message("Mimetype too large");
        return;
    }

    switch(get_var_input("author", author, MAX_AUTHOR_LENGTH - 1)) {
    case -1: error_message("Omitted author"); return;
    case -2: error_message("Author length too large"); return;
    default: break; // all is well
    }

    switch(get_var_input("title", title, MAX_TITLE_LENGTH - 1)) {
    case -1: error_message("Omitted title"); return;
    case -2: error_message("Title length too large"); return;
    default: break; // all is well
    }

    if (add_paper(HOSTNAME, author, title, files->filename, files->contents,
                  files->size, mime, add_result) == -1) {
        error_message(add_result);
    } else {
        printf("<p>Added paper %s.</p>\n", add_result);
    }

    free(add_result);

}

 
int main(int argc, char **argv) {

    time_t current_time;

    printf("Content-Type: text/html\r\n\r\n");

    printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" ");
    printf("\"http://www.w3.o\rg/TR/html4/strict.dtd\">\n");
    printf("<html lang=\"en\">\n\n<head>\n\t<title>\n\t\t");
    printf("Conference Website - Upload a paper\n\t</title>\n\n</head>\n\n");
    printf("<body>\n\n<h1>Conference Website</h1><hr>");
    printf("<h2>Upload a paper</h2>\n");

    if(!strcmp(getenv("REQUEST_METHOD"), "POST")) {
        handle_upload();
    }

    show_form();

    time(&current_time);

    printf("<hr><address>Conference Website - %s</address>\n",
           ctime(&current_time));
    printf("</body>\n</html>\n");

    return(EXIT_SUCCESS);

}
