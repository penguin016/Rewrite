#include<unistd.h>
#include<signal.h>
#include<sys/param.h>
#include<sys/types.h>
#include<sys/stat.h>

void init_daemon()
{
    int pid;
    int i;
    if(pid = fork())
    {
        exit(0); //是父进程，结束父进程
    }
    else if(pid < 0)
    {
        exit(1); //进程创建失败
    }
    //是第一个子进程，后台继续执行
    setsid(); //  第一个进程成为新的会话组组长和进程组组长

    if(pid = fork())
    {
        exit(0); // 结束第一个子进程
    }
    else if(pid < 0)
    {
        exit(1); // 进程创建失败
    }
    //是第二个子进程继续，不再是会话组组长
    for(i = 0; i < NOFILE; i++) // 关闭打开的文件描述符
    {
        close(i);
    }

    chdir("/tmp"); // 改变工作目录到/tmp
    umask(0); // 重设文件创建掩模
    return;
}
