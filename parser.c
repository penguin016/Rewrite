#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include "parse.h"
#include "readproc.h"

easy_proc *parser(proc_t *p)
{
    easy_proc *q = (easy_proc*)malloc(sizeof(easy_proc));
    q->pid = p->tid;
    q->ppid = p->ppid;
    q->uid = p->euid;
    q->gid = p->tgid;
    q->tty = p->tty;
    strcpy(q->eusr, p->euser);
    strcpy(q->rusr, p->ruser);
    strcpy(q->susr, p->suser);
    strcpy(q->fusr, p->fuser);
    strcpy(q->group, p->egroup);
    q->cmdline = p->cmdline;
    return q;
}

