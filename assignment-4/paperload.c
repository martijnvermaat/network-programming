#ifdef WIN32
#define SAVED_ENVIRONMENT "c:\\cgicsave.env"
#else
#define SAVED_ENVIRONMENT "/tmp/cgicsave.env"
#endif /* WIN32 */

#include "assignment4.h"
#include "paperstorage.h"
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <rpc/rpc.h>


void error_message(char *message) {

  cgiHeaderContentType("text/html");

  fprintf(cgiOut, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n");
  fprintf(cgiOut, "<html lang=\"en\">\n\n<head>\n\t<title>\n\t\tPaper fetch error\n\t</title>\n\n</head>\n\n");
  fprintf(cgiOut, "<body>\n\n<h2>Error fetching paper</h2>\n");

  fprintf(cgiOut, "<p class=\"error\">%s</p>\n\n", message);

  fprintf(cgiOut, "</body>\n</html>\n");
}

char *add_paper(char *hostname, char *author, char *title, char *filename, char* buffer) {

  CLIENT *client;
  add_in in;
  add_out *out;
  FILE *stream;
  struct stat file_stat;
  char *p;
  char *error = malloc (ERROR_STRING_SIZE * sizeof(char));

  if (strlen(author) > MAX_AUTHOR_LENGTH) {
    snprintf(error, ERROR_STRING_SIZE, "Maximum length for author is %d\n",
	    MAX_AUTHOR_LENGTH);
    return error;
  }

  if (strlen(title) > MAX_TITLE_LENGTH) {
    snprintf(error, ERROR_STRING_SIZE, "Maximum length for title is %d\n",
	    MAX_TITLE_LENGTH);
    return error;
  }

  p = strrchr(filename, '.');
  if (p == NULL || !(!strcmp(p+1, "pdf") || !strcmp(p+1, "doc"))) {
    snprintf(error, ERROR_STRING_SIZE, "Paper must have pdf or doc file extension\n");
    return error;
  }

  if (stat(filename, &file_stat) == -1) {
    snprintf(error, ERROR_STRING_SIZE, "Cannot stat file");
    return error;
  }

  in.paper.number = NULL;

  in.paper.content = malloc(sizeof(data));
  if (in.paper.content == NULL) {
    snprintf(error, ERROR_STRING_SIZE, "Unable to allocate necessary memory");
    return error;
  }
  in.paper.content->data_len = file_stat.st_size;
  in.paper.content->data_val = malloc(file_stat.st_size);
  if (in.paper.content->data_val == NULL) {
    snprintf(error, ERROR_STRING_SIZE, "Error allocating memory");
    return error;
  }

  stream = fopen(filename, "r");

  if (stream == NULL) {
    snprintf(error, ERROR_STRING_SIZE, "Cannot open file");
    return error;
  }

  if (fread(in.paper.content->data_val, file_stat.st_size, 1, stream)
      != 1) {

    if (feof(stream)) {
      snprintf(error, ERROR_STRING_SIZE, "File is unstable: %s\n", filename);
    } else {
      snprintf(error, ERROR_STRING_SIZE, "Error reading from file");
    }
    return error;

  }

  in.paper.author = author;
  in.paper.title = title;

  client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS,
		       "tcp");

  if (client == NULL) {
    snprintf(error, ERROR_STRING_SIZE, "Error connecting to %s", hostname);
    clnt_pcreateerror("");
    return error;
  }

  out = add_proc_1(&in, client);

  if (out == NULL) {
    snprintf(error, ERROR_STRING_SIZE, "Error querying %s", hostname);
    clnt_perror(client, "");
    clnt_destroy(client);
    return error;
  }

  if (out->result == STATUS_FAILURE) {
    snprintf(error, ERROR_STRING_SIZE, "Error adding paper: %s\n", out->add_out_u.reason);
    clnt_destroy(client);
    return error;
  }

  printf("<p>Added paper %d</p>\n\n", *(out->add_out_u.paper.number));

  clnt_destroy(client);

  return error;

}

void show_form(void) {
  ;
}


int cgiMain() {

  char *add_error;
  char filename[255];
  char author[255];
  char title[255];
  char *buffer;
  int filesize = 0, read_bytes = 0;
  cgiFilePtr file;

  if (cgiFormFileName("file", filename, sizeof(filename)) == cgiFormSuccess) {
    cgiFormFileSize("file", &filesize);
    buffer = malloc(filesize * sizeof(char));
    cgiFormFileOpen("file", &file);
    cgiFormFileRead(file, buffer, sizeof(buffer), &read_bytes);
    cgiFormFileClose(file);

    add_error = add_paper(HOSTNAME, author, title, filename, buffer);
    if (strlen(add_error) > 0) {
      error_message(add_error);
      free(add_error);
    }
  }

  show_form();

  return 0;
}


