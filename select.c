#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/dir.h>
#include "readproc.h"
#include "parse.h"


int nextpid(proc_t *p)
{
    static struct direct *ent;//目录结构体
    char* path = p->path;
    DIR* procfs;
    if((procfs = opendir(path)) == NULL)
    {
        printf("can't open %s", "/proc");
    };
    for(;;)
    {
        ent = readdir(procfs);
        if(unlikely(unlikely(!ent) || unlikely(!ent->d_name))) return 0;
        if(likely( likely(*ent->d_name > '0') && likely(*ent->d_name <= '9') )) break;
        //printf("%s\n", ent->d_name);
    }
    p->tgid = strtoul(ent->d_name, NULL, 10);
    p->tid = p->tgid;
    memcpy(path, "/proc/", 6);
    strcpy(path+6, ent->d_name);
    //printf("%s\n", path);
    return 1;
}

int isFullChar(char *s)//判断一个字符串是否全由数字组成
{
    int i = 0;
    while(s[i] != '\0')
    {
        if(s[i] >= '0' && s[i] <= '9')
        i++;
        else
        return 1;
    }
    return 0;
}


//字符串匹配
//src:源字符串
//dst：目标字符串
//len:源字符串被比较的长度
int match(char *src,char *dst,int len)
{
    int i = 0;
    int j = 0;
    int size_dst = 0;

    //获得目标字符串的长度
    size_dst = strlen(dst);
    //如果目标字符串的长度大于len，返回失败
    if (size_dst > len)
    {
        return 0;
    }
    //开始比较
    for (i = 0;i < len;i++)
    {
        for (j = 0;j < size_dst;j++)
        {
            if (src[i + j] != dst[j])
            {
                break;
            }
        }
        if (j == size_dst)
        {
            return 1;
        }
    }

    return 0;
}

easy_proc* findUsrProc(char *usr)//找出所有用户空间的进程，返回一张进程列表
{
    DIR* proc;
    struct direct *dirtp;
    easy_proc *start = (easy_proc *)malloc(sizeof(easy_proc));
    start->pid = 0;
    start->ppid = 0;
    start->uid = 0;
    start->uid =0;
    start->tty = 0;
    start->cmdline = NULL;
    start->next = NULL;
    easy_proc *now = start;
    if((proc = opendir("/proc")) == NULL)
        printf("can't open %s\n", "/proc");
    else
    {
        while((dirtp = readdir(proc)) != NULL)
        {
            if(!isFullChar(dirtp->d_name))
            {
                proc_t *r = (proc_t*)malloc(sizeof(proc_t));
                memcpy(r->path, "/proc/", 6);
                strcpy(r->path+6, dirtp->d_name);
                r = simple_readproc(r);
                easy_proc *tem = parser(r);
                if( !(  strcmp(tem->eusr, usr) ||
                       strcmp(tem->rusr, usr) ||
                       strcmp(tem->fusr, usr) ||
                       strcmp(tem->susr, usr)  ) && tem->tty != 0)
                {
                    now->next = tem;
                    now = now->next;
                    now->next = NULL;
                }
            }
        }
        closedir(proc);
    }
    return start;
}
