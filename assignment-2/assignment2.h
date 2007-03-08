#ifndef __ASSIGNMENT2_H

#define DEBUG 1

#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif


#define SERVER_PORT 2342
#define NB_PROC 1

#endif
