#ifndef SELECT_H_INCLUDED
#define SELECT_H_INCLUDED

#include "readproc.h"
#include "parse.h"

int nextpid(proc_t *p);
easy_proc* findUsrProc(char *usr);

#endif // SELECT_H_INCLUDED
