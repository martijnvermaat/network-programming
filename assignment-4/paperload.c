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
#include <time.h>

void error_message(char *message) {
  fprintf(cgiOut, "<h3>Error uploading paper</h3>\n");
  fprintf(cgiOut, "<p class=\"error\">%s</p>\n\n", message);
}

int add_paper(char *hostname, char *author, char *title, char *filename, char* buffer, int size, char *result) {

  CLIENT *client;
  add_in in;
  add_out *out;
  char *p;

  if (strlen(author) > MAX_AUTHOR_LENGTH) {
    snprintf(result, ERROR_STRING_SIZE, "Maximum length for author is %d\n",
	    MAX_AUTHOR_LENGTH);
    return -1;
  }

  if (strlen(title) > MAX_TITLE_LENGTH) {
    snprintf(result, ERROR_STRING_SIZE, "Maximum length for title is %d\n",
	    MAX_TITLE_LENGTH);
    return -1;
  }

  p = strrchr(filename, '.');
  if (p == NULL || !(!strcmp(p+1, "pdf") || !strcmp(p+1, "doc"))) {
    snprintf(result, ERROR_STRING_SIZE, "Paper must have pdf or doc file extension\n");

    return -1;
  }

  in.paper.number = NULL;

  in.paper.content = malloc(sizeof(data));
  if (in.paper.content == NULL) {
    snprintf(result, ERROR_STRING_SIZE, "Unable to allocate necessary memory");
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

  client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS,
		       "tcp");

  if (client == NULL) {
    snprintf(result, ERROR_STRING_SIZE, "Error connecting to %s", hostname);
    return -1;
  }

  out = add_proc_1(&in, client);

  if (out == NULL) {
    snprintf(result, ERROR_STRING_SIZE, "Error querying %s", hostname);
    clnt_destroy(client);
    return -1;
  }

  if (out->result == STATUS_FAILURE) {
    snprintf(result, ERROR_STRING_SIZE, "Error adding paper: %s\n", out->add_out_u.reason);
    clnt_destroy(client);
    return -1;
  }

  snprintf(result, ERROR_STRING_SIZE, "%d", *(out->add_out_u.paper.number));
  clnt_destroy(client);
  
  return 1;
}

void show_form(void) {
  fprintf(cgiOut, "<form id=\"uploadpaper\" method=\"post\" action=\"paperload.cgi\" enctype=\"multipart/form-data\">\n");
  fprintf(cgiOut, "\t<label for=\"file\">Paper:</lable>\n");
  fprintf(cgiOut, "\t<input type=\"file\" name=\"file\" id=\"file\">\n");

  fprintf(cgiOut, "<br>");

  fprintf(cgiOut, "\t<label for=\"author\">Author:</lable>\n");
  fprintf(cgiOut, "\t<input type=\"text\" maxlength=\"255\" name=\"author\" id=\"author\">\n");

  fprintf(cgiOut, "<br>");

  fprintf(cgiOut, "\t<label for=\"title\">Paper title:</lable>\n");
  fprintf(cgiOut, "\t<input type=\"text\" maxlength=\"255\" name=\"title\" id=\"title\">\n");

  fprintf(cgiOut, "<br>");

  fprintf(cgiOut, "<input type=\"submit\" value=\"Upload paper\">\n");

  fprintf(cgiOut, "</form>\n");
}


void handle_upload(char *filename) {
  char author[MAX_AUTHOR_LENGTH];
  char title[MAX_TITLE_LENGTH];
  char *buffer, *add_result;
  int filesize = 0, read_bytes = 0;
  cgiFilePtr file;
  add_result = malloc(ERROR_STRING_SIZE);

  if(cgiFormFileSize("file", &filesize) != cgiFormSuccess) {
    error_message("Error determining file size - unable to store paper");
    return;
  }

  buffer = malloc(filesize * sizeof(char));
  if (buffer == NULL) {
    error_message("Unable to allocate necessary memory");
    return;
  }

  if (cgiFormFileOpen("file", &file) != cgiFormSuccess) {
    error_message("Could not process paper (1)");
    return;
  }

  if (cgiFormFileRead(file, buffer, filesize, &read_bytes) != cgiFormSuccess) {
    error_message("Could not process paper (2)");
    return;
  }

  if (cgiFormFileClose(file) != cgiFormSuccess) {
    error_message("Could not process paper (3)");
    return;
  }

  if (cgiFormString("author", author, MAX_AUTHOR_LENGTH) == cgiFormNotFound) {
    error_message("Omitted author");
    return;
  }

  if (cgiFormString("author", author, MAX_AUTHOR_LENGTH) == cgiFormTruncated) {
    error_message("Author size too large");
    return;
  }

  if(strlen(author) == 0) {
    error_message("Omitted author");
    return;
  }

  if (cgiFormString("title", title, MAX_TITLE_LENGTH) == cgiFormNotFound) {
    error_message("Omitted paper title");
    return;
  }

  if (cgiFormString("title", title, MAX_TITLE_LENGTH) == cgiFormTruncated) {
    error_message("Paper title too large");
    return;
  }

  if(strlen(title) == 0) {
    error_message("Omitted title");
    return;
  }

  if (add_paper(HOSTNAME, author, title, filename, buffer, filesize, add_result) == -1) {
    error_message(add_result);
  } else {
    fprintf(cgiOut, "<p>Added paper %s.</p>\n", add_result);
  }
  free(add_result);
  free(buffer);
}

int cgiMain() {
  time_t current_time;
  char filename[255];
  cgiHeaderContentType("text/html");
  cgiFormResultType form_result;

  fprintf(cgiOut, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.o\rg/TR/html4/strict.dtd\">\n");
  fprintf(cgiOut, "<html lang=\"en\">\n\n<head>\n\t<title>\n\t\tConference Website - Upload a paper\n\t</title>\n\n</head>\n\n");
  fprintf(cgiOut, "<body>\n\n<h1>Conference Website</h1><hr><h2>Upload a paper</h2>\n");

  form_result = (cgiFormFileName("file", filename, sizeof(filename)));
  
  if(form_result == cgiFormSuccess) {
    handle_upload(filename);
  } else if(form_result == cgiFormTruncated) {
    fprintf(cgiOut, "<p class=\"error\">Filename length too large, so file type cannot be determined. Assuming .doc.</p>\n");
    handle_upload(filename);
  }

  show_form();

  time(&current_time);

  fprintf(cgiOut, "<hr><address>Conference Website - %s</address>\n", ctime(&current_time));
  fprintf(cgiOut, "</body>\n</html>\n");

  return 0;
}


