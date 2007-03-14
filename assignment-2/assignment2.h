#ifndef __ASSIGNMENT2_H
#define __ASSIGNMENT2_H


#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif


#define SERVER_PORT 2342
#define PENDING_CONNECTIONS_QUEUE 5
#define NB_PROC 5

#define READ_BUFFER_SIZE 20


#endif
