#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/dir.h>
#include <sys/types.h>
#include "readproc.h"
#include "select.h"
#include "parse.h"
void init_daemon(); // 守护进程初始化



int main()
{


   /* proc_t *p = (proc_t*)malloc(sizeof(proc_t));
    //p->flags = 64;
    strcpy(p->path, "/proc/17330");
    p = simple_readproc(p);
    printf("pid: %d\n", p->tid);
    printf("ppid: %d\n", p->ppid);
    printf("pcpu: %u\n", p->pcpu);
    printf("state: %c\n", p->state);
    printf("start time: %ld\n", p->start_time);
    printf("size: %ld\n", p->size);
    printf("resident: %ld\n", p->resident);
    printf("share: %ld\n", p->share);
    printf("trs: %ld\n", p->trs);
    printf("lrs: %ld\n", p->lrs);
    printf("drs: %ld\n", p->drs);
    printf("dt: %ld\n", p->dt);
    printf("cmdline: %s\n", *p->cmdline);
    printf("vm_size: %lu\n", p->vm_size);
    printf("vm_lock: %lu\n", p->vm_lock);
    printf("vm_rss: %lu\n", p->vm_rss);
    printf("vm_data: %lu\n", p->vm_data);
    printf("vm_stack: %lu\n", p->vm_stack);
    printf("vm_exe: %lu\n", p->vm_exe);
    printf("vm_lib: %lu\n", p->vm_lib);
    printf("rtprio: %lu\n", p->rtprio);
    printf("sched: %lu\n", p->sched);
    printf("vsize: %lu\n", p->vsize);
    printf("rss_rlim: %lu\n", p->rss_rlim);
    printf("pgrp: %d\n", p->pgrp);
    printf("session: %d\n", p->session);
    printf("tgid: %d\n", p->tgid);
    printf("tty: %d\n", p->tty);
    printf("euser: %s\n", p->euser);
    printf("ruser: %s\n", p->ruser);
    printf("suser: %s\n", p->suser);
    printf("rgroup: %s\n", p->rgroup);
    printf("egroup: %s\n", p->egroup);
    printf("sgroup %s\n", p->sgroup);
    printf("fgroup: %s\n", p->fgroup);

    //proc_t *q = (proc_t*)malloc(sizeof(proc_t));
    //strcpy(p->path, "/proc/6834");
  //  q = simple_readproc(q);

    /*DIR* proc;
    struct direct *dirtp;

    if((proc = opendir("/proc")) == NULL)
        printf("can't open %s\n", p->path);
    else
    {
        while((dirtp = readdir(proc)) != NULL)
        {
            if(!isFullChar(dirtp->d_name))
            printf("%s\n", dirtp->d_name);
        }
        closedir(proc);
    }

   // int i = nextpid(p);
    //while(i != 0)
    //{
    //    printf("path: %s\n", p->path);
    //    printf("%d\n", p->tid);
    //    strcpy(p->path, "/proc/1");
    //    i = nextpid(p);
    //}
    //;

*/
    char *name = "wsx";
    easy_proc *p = findUsrProc(name);
    while(p->next)
    {
        p = p->next;
        printf("%d %s %s\n", p->pid, p->susr, *p->cmdline);

    }


    return 0;
}
