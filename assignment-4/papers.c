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

/*
  TODO: Check XDR structures for NULL pointers, we should not trust rpcgen too
  much.
*/
void paper_listing(char *hostname) {

  CLIENT *client;
  list_out *out;
  document_list papers;
  int paper_count = 0;

  client = clnt_create(hostname, PAPERSTORAGE_PROG, PAPERSTORAGE_VERS,
		       "tcp");

  if (client == NULL) {
    fprintf(cgiOut, "<p class=\"error\">Error connecting to %s</p>\n\n", hostname);
    clnt_pcreateerror("");
    return;
  }

  out = list_proc_1(NULL, client);

  if (out == NULL) {
    fprintf(cgiOut, "<p class=\"error\">Error querying %s</p>\n\n", hostname);
    clnt_perror(client, "");
    clnt_destroy(client);
    return;
  }

  papers = out->papers;

  while (papers) {
    paper_count++;
    fprintf(cgiOut, "<p>Paper %d<br>  Author: ``%s''<br> Title:  ``<a href=\"paperview.cgi?paper=%d\">%s</a>''</p>\n",
	   *(papers->item.number),
	   papers->item.author, paper_count, papers->item.title);
    papers = papers->next;
  }

  if (paper_count == 1) {
    fprintf(cgiOut, "<p>1 paper stored</p>\n");
  } else {
    fprintf(cgiOut, "<p>%d papers stored\n</p>", paper_count);
  }

  clnt_destroy(client);

}


int cgiMain() {
  time_t current_time;

  cgiHeaderContentType("text/html");

  fprintf(cgiOut, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n");
  fprintf(cgiOut, "<html lang=\"en\">\n\n<head>\n\t<title>\n\t\tPapers stored at paperserver\n\t</title>\n\n</head>\n\n");
  fprintf(cgiOut, "<body>\n\n<h1>Conference Website</h1><hr><h2>Stored papers</h2>\n");
  
  paper_listing(HOSTNAME);

  time(&current_time);

  
  fprintf(cgiOut, "<hr><address>Conference Website - %s</address>\n</body>\n</html>\n", ctime(&current_time));
  return 0;
}


