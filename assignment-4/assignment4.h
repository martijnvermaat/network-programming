#ifndef __ASSIGNMENT4_H
#define __ASSIGNMENT4_H


#define DEBUG 1

#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif

#define ERROR_STRING_SIZE 128
#define HOSTNAME "localhost"
#define REQUEST_SIZE 10
#define READ_BUFFER_SIZE 20

#endif
