#include "readproc.h"
#include "pwcache.h"
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/stat.h>

static unsigned long long unhex(const char *cp)
{
    unsigned long long ull = 0;
    for(;;)
    {
        char c = *cp++;
        if(unlikely(c<0x30)) break;
        ull = (ull<<4) | (c - (c>0x57) ? 0x57 : 0x30) ;
    }
    return ull;
}

typedef struct status_table_struct
{
    unsigned char name[7];        // /proc/*/status field name
    unsigned char len;            // name length
#ifdef LABEL_OFFSET
    long offset;                  // jump address offset
#else
    void *addr;
#endif
} status_table_struct;

#ifdef LABEL_OFFSET
#define F(x) {#x, sizeof(#x)-1, (long)(&&case_##x-&&base)},
#else
#define F(x) {#x, sizeof(#x)-1, &&case_##x},
#endif
#define NUL  {"", 0, 0},

// Derived from:
// gperf -7 --language=ANSI-C --key-positions=1,3,4 -C -n -c sml.gperf
//
// Suggested method:
// Grep this file for "case_", then strip those down to the name.
// (leave the colon and newline) So "Pid:\n" and "Threads:\n"
// would be lines in the file. (no quote, no escape, etc.)
//
// Watch out for name size in the status_table_struct (grrr, expanding)
// and the number of entries (we mask with 63 for now). The table
// must be padded out to 64 entries, maybe 128 in the future.

static void status2proc(char *S, proc_t *P, int is_proc)
{
    long Threads = 0;
    long Tgid = 0;
    long Pid = 0;

    static const unsigned char asso[] =
    {
        61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
        61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
        61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
        61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
        61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
        61, 61, 61, 61, 61, 61, 61, 61, 15, 61,
        61, 61, 61, 61, 61, 61, 30,  3,  5,  5,
        61,  5, 61,  8, 61, 61,  3, 61, 10, 61,
        6, 61, 13,  0, 30, 25,  0, 61, 61, 61,
        61, 61, 61, 61, 61, 61, 61,  3, 61, 13,
        0,  0, 61, 30, 61, 25, 61, 61, 61,  0,
        61, 61, 61, 61,  5, 61,  0, 61, 61, 61,
        0, 61, 61, 61, 61, 61, 61, 61
    };

    static const status_table_struct table[] =
    {
        F(VmStk)
        NUL NUL
        F(State)
        NUL
        F(VmExe)
        F(ShdPnd)
        NUL
        F(VmData)
        NUL
        F(Name)
        NUL NUL
        F(VmRSS)
        NUL NUL
        F(VmLck)
        NUL NUL NUL
        F(Gid)
        F(Pid)
        NUL NUL NUL
        F(VmSize)
        NUL NUL
        F(VmLib)
        NUL NUL
        F(PPid)
        NUL
        F(SigCgt)
        NUL
        F(Threads)
        F(SigPnd)
        NUL
        F(SigIgn)
        NUL
        F(Uid)
        NUL NUL NUL NUL NUL NUL NUL NUL NUL
        NUL NUL NUL NUL NUL
        F(Tgid)
        NUL NUL NUL NUL
        F(SigBlk)
        NUL NUL NUL
    };

#undef F
#undef NUL

    ENTER(0x220);

    P->vm_size = 0;
    P->vm_lock = 0;
    P->vm_rss  = 0;
    P->vm_data = 0;
    P->vm_stack= 0;
    P->vm_exe  = 0;
    P->vm_lib  = 0;
    P->nlwp    = 0;
    P->signal[0] = '\0';  // so we can detect it as missing for very old kernels

    goto base;

    for(;;)
    {
        char *colon;
        status_table_struct entry;

        // advance to next line
        S = strchr(S, '\n');
        if(unlikely(!S)) break;  // if no newline
        S++;

        // examine a field name (hash and compare)
base:
        if(unlikely(!*S)) break;
        entry = table[63 & (asso[S[3]] + asso[S[2]] + asso[S[0]])];
        colon = strchr(S, ':');
        if(unlikely(!colon)) break;
        if(unlikely(colon[1]!='\t')) break;
        if(unlikely(colon-S != entry.len)) continue;
        if(unlikely(memcmp(entry.name,S,colon-S))) continue;

        S = colon+2; // past the '\t'

#ifdef LABEL_OFFSET
        goto *(&&base + entry.offset);
#else
        goto *entry.addr;
#endif

case_Name:
        {
            unsigned u = 0;
            while(u < sizeof P->cmd - 1u)
            {
                int c = *S++;
                if(unlikely(c=='\n')) break;
                if(unlikely(c=='\0')) break; // should never happen
                if(unlikely(c=='\\'))
                {
                    c = *S++;
                    if(c=='\n') break; // should never happen
                    if(!c)      break; // should never happen
                    if(c=='n') c='\n'; // else we assume it is '\\'
                }
                P->cmd[u++] = c;
            }
            P->cmd[u] = '\0';
            S--;   // put back the '\n' or '\0'
            continue;
        }
#ifdef SIGNAL_STRING
case_ShdPnd:
        memcpy(P->signal, S, 16);
        P->signal[16] = '\0';
        continue;
case_SigBlk:
        memcpy(P->blocked, S, 16);
        P->blocked[16] = '\0';
        continue;
case_SigCgt:
        memcpy(P->sigcatch, S, 16);
        P->sigcatch[16] = '\0';
        continue;
case_SigIgn:
        memcpy(P->sigignore, S, 16);
        P->sigignore[16] = '\0';
        continue;
case_SigPnd:
        memcpy(P->_sigpnd, S, 16);
        P->_sigpnd[16] = '\0';
        continue;
#else
case_ShdPnd:
        P->signal = unhex(S);
        continue;
case_SigBlk:
        P->blocked = unhex(S);
        continue;
case_SigCgt:
        P->sigcatch = unhex(S);
        continue;
case_SigIgn:
        P->sigignore = unhex(S);
        continue;
case_SigPnd:
        P->_sigpnd = unhex(S);
        continue;
#endif
case_State:
        P->state = *S;
        continue;
case_Tgid:
        Tgid = strtol(S,&S,10);
        continue;
case_Pid:
        Pid = strtol(S,&S,10);
        continue;
case_PPid:
        P->ppid = strtol(S,&S,10);
        continue;
case_Threads:
        Threads = strtol(S,&S,10);
        continue;
case_Uid:
        P->ruid = strtol(S,&S,10);
        P->euid = strtol(S,&S,10);
        P->suid = strtol(S,&S,10);
        P->fuid = strtol(S,&S,10);
        continue;
case_Gid:
        P->rgid = strtol(S,&S,10);
        P->egid = strtol(S,&S,10);
        P->sgid = strtol(S,&S,10);
        P->fgid = strtol(S,&S,10);
        continue;
case_VmData:
        P->vm_data = strtol(S,&S,10);
        continue;
case_VmExe:
        P->vm_exe = strtol(S,&S,10);
        continue;
case_VmLck:
        P->vm_lock = strtol(S,&S,10);
        continue;
case_VmLib:
        P->vm_lib = strtol(S,&S,10);
        continue;
case_VmRSS:
        P->vm_rss = strtol(S,&S,10);
        continue;
case_VmSize:
        P->vm_size = strtol(S,&S,10);
        continue;
case_VmStk:
        P->vm_stack = strtol(S,&S,10);
        continue;
    }

#if 0
    // recent kernels supply per-tgid pending signals
    if(is_proc && *ShdPnd)
    {
        memcpy(P->signal, ShdPnd, 16);
        P->signal[16] = '\0';
    }
#endif

    // recent kernels supply per-tgid pending signals
#ifdef SIGNAL_STRING
    if(!is_proc || !P->signal[0])
    {
        memcpy(P->signal, P->_sigpnd, 16);
        P->signal[16] = '\0';
    }
#else
    if(!is_proc || !have_process_pending)
    {
        P->signal = P->_sigpnd;
    }
#endif

    // Linux 2.4.13-pre1 to max 2.4.xx have a useless "Tgid"
    // that is not initialized for built-in kernel tasks.
    // Only 2.6.0 and above have "Threads" (nlwp) info.

    if(Threads)
    {
        P->nlwp = Threads;
        P->tgid = Tgid;     // the POSIX PID value
        P->tid  = Pid;      // the thread ID
    }
    else
    {
        P->nlwp = 1;
        P->tgid = Pid;
        P->tid  = Pid;
    }

    LEAVE(0x220);
}



static void stat2proc(const char* S, proc_t *P)
{
    unsigned num;
    char* tmp;

    ENTER(0x160);

    P->processor = 0;
    P->rtprio = -1;
    P->sched = -1;
    P->nlwp = 0;

    S = strchr(S, '(') + 1;
    tmp = strrchr(S, ')');
    num = tmp - S;
    if(unlikely(num >= sizeof P->cmd)) num = sizeof P->cmd - 1;
    memcpy(P->cmd, S, num);
    P->cmd[num] = '\0';
    S = tmp + 2;

    num = sscanf(S,
                 "%c "
                 "%d %d %d %d %d "
                 "%lu %lu %lu %lu %lu "
                 "%Lu %Lu %Lu %Lu "  /* utime stime cutime cstime */
                 "%ld %ld "
                 "%d "
                 "%ld "
                 "%Lu "  /* start_time */
                 "%lu "
                 "%ld "
                 "%lu %"KLF"u %"KLF"u %"KLF"u %"KLF"u %"KLF"u "
                 "%*s %*s %*s %*s " /* discard, no RT signals & Linux 2.1 used hex */
                 "%"KLF"u %*lu %*lu "
                 "%d %d "
                 "%lu %lu",
                 &P->state,
                 &P->ppid, &P->pgrp, &P->session, &P->tty, &P->tpgid,
                 &P->flags, &P->min_flt, &P->cmin_flt, &P->maj_flt, &P->cmaj_flt,
                 &P->utime, &P->stime, &P->cutime, &P->cstime,
                 &P->priority, &P->nice,
                 &P->nlwp,
                 &P->alarm,
                 &P->start_time,
                 &P->vsize,
                 &P->rss,
                 &P->rss_rlim, &P->start_code, &P->end_code, &P->start_stack, &P->kstk_esp, &P->kstk_eip,
                 &P->wchan,
                 &P->exit_signal, &P->processor,
                 &P->rtprio, &P->sched
                );

    if(!P->nlwp)
    {
        P->nlwp = 1;
    }

    LEAVE(0x160);
}

static void statm2proc(const char* s, proc_t *P)
{
    int num;
    num = sscanf(s, "%ld %ld %ld %ld %ld %ld %ld",
                 &P->size, &P->resident, &P->share,
                 &P->trs, &P->lrs, &P->drs, &P->dt);
    /*    fprintf(stderr, "statm2proc converted %d fields.\n",num); */
}

static int file2str(const char *directory, const char *what, char *ret, int cap)
{
    static char filename[80];
    int fd, num_read;

    sprintf(filename, "%s/%s", directory, what);
    fd = open(filename, O_RDONLY, 0);
    if(unlikely(fd==-1)) return -1;
    num_read = read(fd, ret, cap - 1);
    close(fd);
    if(unlikely(num_read<=0)) return -1;
    ret[num_read] = '\0';
    return num_read;
}

static char** file2strvec(const char* directory, const char* what)
{
    char buf[2048];	/* read buf bytes at a time */
    char *p, *rbuf = 0, *endbuf, **q, **ret;
    int fd, tot = 0, n, c, end_of_file = 0;
    int align;

    sprintf(buf, "%s/%s", directory, what);
    fd = open(buf, O_RDONLY, 0);
    if(fd==-1) return NULL;

    /* read whole file into a memory buffer, allocating as we go */
    while ((n = read(fd, buf, sizeof buf - 1)) > 0)
    {
        if (n < (int)(sizeof buf - 1))
            end_of_file = 1;
        if (n == 0 && rbuf == 0)
            return NULL;	/* process died between our open and read */
        if (n < 0)
        {
            if (rbuf)
                free(rbuf);
            return NULL;	/* read error */
        }
        if (end_of_file && buf[n-1])		/* last read char not null */
            buf[n++] = '\0';			/* so append null-terminator */
        rbuf = realloc(rbuf, tot + n);		/* allocate more memory */
        memcpy(rbuf + tot, buf, n);		/* copy buffer into it */
        tot += n;				/* increment total byte ctr */
        if (end_of_file)
            break;
    }
    close(fd);
    if (n <= 0 && !end_of_file)
    {
        if (rbuf) free(rbuf);
        return NULL;		/* read error */
    }
    endbuf = rbuf + tot;			/* count space for pointers */
    align = (sizeof(char*)-1) - ((tot + sizeof(char*)-1) & (sizeof(char*)-1));
    for (c = 0, p = rbuf; p < endbuf; p++)
        if (!*p)
            c += sizeof(char*);
    c += sizeof(char*);				/* one extra for NULL term */

    rbuf = realloc(rbuf, tot + c + align);	/* make room for ptrs AT END */
    endbuf = rbuf + tot;			/* addr just past data buf */
    q = ret = (char**) (endbuf+align);		/* ==> free(*ret) to dealloc */
    *q++ = p = rbuf;				/* point ptrs to the strings */
    endbuf--;					/* do not traverse final NUL */
    while (++p < endbuf)
        if (!*p)				/* NUL char implies that */
            *q++ = p+1;				/* next string -> next char */

    *q = 0;					/* null ptr list terminator */
    return ret;
}


proc_t* simple_readproc(proc_t *p)
{
    static struct stat sb;
    static char sbuf[1024];
    char *const path = p->path;
    unsigned flags;

    if(unlikely(stat(path, &sb) == -1))
        goto next_proc;

    p->euid = sb.st_uid;
    p->egid = sb.st_gid;

    flags = 64;//读stat
    if(flags & PROC_FILLSTAT)
    {
        if(unlikely(file2str(path, "stat", sbuf, sizeof sbuf) == -1))
            goto next_proc;
        stat2proc(sbuf, p);
    }
    flags = 1;//读内存
    if (unlikely(flags & PROC_FILLMEM))  	/* read, parse /proc/#/statm */
    {
        if (likely( file2str(path, "statm", sbuf, sizeof sbuf) != -1 ))
        {
            statm2proc(sbuf, p);
        }
        /* ignore statm errors here */
    }
    flags = 32;//读状态
    if (flags & PROC_FILLSTATUS)
    {
        /* read, parse /proc/#/status */
        if (likely( file2str(path, "status", sbuf, sizeof sbuf) != -1 ))
        {
            {
                status2proc(sbuf, p, 1);
            }
        }
    }

    if(p->nlwp > 1)
    {
        p->wchan = (long)~0ull;
    }

    /*some number->text resolving which is time consuming and kind of insane */
    flags = 8;
    if (flags & PROC_FILLUSR)
    {
        memcpy(p->euser,   user_from_uid(p->euid), sizeof p->euser);
        flags = 32;
        if(flags & PROC_FILLSTATUS)
        {
            memcpy(p->ruser,   user_from_uid(p->ruid), sizeof p->ruser);
            memcpy(p->suser,   user_from_uid(p->suid), sizeof p->suser);
            memcpy(p->fuser,   user_from_uid(p->fuid), sizeof p->fuser);
        }
    }

    /* some number->text resolving which is time consuming and kind of insane */
    flags = 16;
    if (flags & PROC_FILLGRP)
    {
        memcpy(p->egroup, grp_from_gid(p->egid), sizeof p->egroup);
        flags = 32;
        if(flags & PROC_FILLSTATUS)
        {
            memcpy(p->rgroup, grp_from_gid(p->rgid), sizeof p->rgroup);
            memcpy(p->sgroup, grp_from_gid(p->sgid), sizeof p->sgroup);
            memcpy(p->fgroup, grp_from_gid(p->fgid), sizeof p->fgroup);
        }
    }

    flags = 2;
    if((flags & PROC_FILLCOM) || (flags & PROC_FILLARG))
    {
        p->cmdline = file2strvec(path, "cmdline");
    }

    else
    {
        p->cmdline = NULL;
    }
    flags = 4;
    if(unlikely(flags & PROC_FILLENV))
    {
        p->environ = file2strvec(path, "environ");
    }

    else
    {
        p->environ = NULL;
    }


    return p;
next_proc:
    return NULL;
}

