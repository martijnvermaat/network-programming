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


void setFileName(int number, char *mimeType) {
  char *p;
  p = strrchr(mimeType, '/');

  if (!strcmp(p+1, "pdf")) {
    fprintf(cgiOut, "Content-Disposition: attachment; filename=\"paper-%d.pdf\"\r\n", number);
  } else {
    fprintf(cgiOut, "Content-Disposition: attachment; filename=\"paper-%d.doc\"\r\n", number);
  }
  
}

void error_message(char *message) {

  cgiHeaderContentType("text/html");

  fprintf(cgiOut, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n");
  fprintf(cgiOut, "<html lang=\"en\">\n\n<head>\n\t<title>\n\t\tPaper fetch error\n\t</title>\n\n</head>\n\n");
  fprintf(cgiOut, "<body>\n\n<h2>Error fetching paper</h2>\n");

  fprintf(cgiOut, "<p class=\"error\">%s</p>\n\n", message);

  fprintf(cgiOut, "</body>\n</html>\n");
}

/*
  TODO: Check XDR structures for NULL pointers, we should not trust rpcgen and
        the server too much.
  TODO: Reuse more code of all methods (or just of fetch/details)
*/
char *fetch_paper(char *hostname, int number) {

  CLIENT *client;
  get_in in;
  get_out *out;
  char *error;
  char *mimeType = "application/pdf";

  error = malloc(ERROR_STRING_SIZE * sizeof(char));

  client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS,
		       "tcp");

  if (client == NULL) {
    snprintf(error, ERROR_STRING_SIZE, "Error connecting to %s", hostname);
    clnt_pcreateerror("");
    return error;
  }

  in.number = number;
  in.representation = DETAILED;
  out = get_proc_1(&in, client);

  if (out == NULL) {
    snprintf(error, ERROR_STRING_SIZE, "Error querying %s", hostname);
    clnt_perror(client, "");
    clnt_destroy(client);
    return error;
  }

  if (out->result == STATUS_FAILURE) {
    snprintf(error, ERROR_STRING_SIZE, "Error requesting paper details: %s",
	    out->get_out_u.reason);
    clnt_destroy(client);
    return error;
  }

  setFileName(number, mimeType);
  cgiHeaderContentType(mimeType);
  if (fwrite(out->get_out_u.paper.content->data_val,
	     out->get_out_u.paper.content->data_len, 1, cgiOut) != 1) {
    clnt_destroy(client);
    return error;
  }

  clnt_destroy(client);
  return error;
}


int cgiMain() {

  char *fetch_error;
  int paper_id = -1;

  cgiFormInteger("paper", &paper_id, -1);

  if (paper_id == -1) {
    error_message("Supplied invalid paper id");
  } else {
    fetch_error = fetch_paper(HOSTNAME, paper_id);
    if (strlen(fetch_error) > 0) {
      error_message(fetch_error);
      free(fetch_error);
    }
  }

  return 0;
}


