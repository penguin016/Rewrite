#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <sys/types.h>
#include <stdlib.h>
#include "readproc.h"

typedef struct easy_proc
{
  pid_t pid;
  pid_t ppid;
  uid_t uid;
  gid_t gid;
  dev_t tty;
  char state;
  char eusr[20];
  char rusr[20];
  char susr[20];
  char fusr[20];
  char group[20];
  char **cmdline;
  struct easy_proc *next;
}easy_proc;

easy_proc *parser(proc_t *p);

#endif // PARSE_H_INCLUDED
