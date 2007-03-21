#ifndef __ASSIGNMENT3_H
#define __ASSIGNMENT3_H


#include "paperstorage.h"


#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif


#define SERVER_PROC(name) name##_out *name##_proc_1_svc(name##_in *in, struct svc_req *rqstp)


#endif
