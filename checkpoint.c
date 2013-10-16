#include "libcr.h"
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include "parse.h"
#include "checkpoint.h"

static easy_proc *cr_world = NULL;
static int cr_world_n = -1;
static char *executable;
static cr_client_id_t client_id;
static char *app_schema = NULL;

int blcr_init(char *path, char **argv, easy_proc *world, int world_n)
//注册callback函数
{
    cr_callback_id_t cr_thread_callback_id, cr_signal_callback_id;
    void *cr_thread_callback_arg = NULL;
    void *cr_signal_callback_arg = NULL;

    client_id = cr_init();
    if(client_id < 0)
        return -1;

    blcr_disable_checkpoint();

    //注册checkpoint信号句柄
    cr_thread_callback_id = cr_register_callback(cr_thread_callback,
                                                 cr_thread_callback_arg,
                                                 CR_THREAD_CONTEXT);
    cr_signal_callback_id = cr_register_callback(cr_signal_callback,
                                                 cr_signal_callback_arg,
                                                 CR_SIGNAL_CONTEXT);

    cr_world = world;
    cr_world_n = world_n;
    executable = strdup(path);

    return 0;
}

void blcr_disable_checkpoint(void)
{
    cr_enter_cs(client_id);
    return;
}

void blcr_enable_checkpoint(void)
{
    cr_leave_cs(client_id);
    return;
}

int blcr_checkpoint(void)
{
    pid_t pid;
    int status;

    //fork()一个子进程向所有进程发送checkpoint请求
    if((pid = fork()) < 0)
    {
        printf("fork() ERROR\n");
        return -1;
    }
    else if(pid == 0)
    {
        //子进程
        cr_signal_app(cr_world, cr_world_n);
        exit();

    }
    //父进程
    if(waitpid(pid, &status, 0) != pid)
    {
        printf("waitpid failed\n");
        return -1;
    }

    if((WIFEXITED(status) !=0) && (WEXITSTATUS(status) !=0))
    {
        printf("bad exit status form child: %d.\n", WEXITSTATUS(status));
        return -1;
    }

    //创建一个app_schema存入内存，用于restart
    app_schema = create_app_schema(cr_world, cr_world_n);
    if(app_schema == NULL)
    {
        printf("couldn't make app schema!\n");
        return -1;
    }
    return 0;
}

int blcr_continue(void)
{
    return 0;
}

static int cr_thread_callback(void *argv)
{
    int ret;

    if(blcr_checkpoint() != 0)
        printf("checkpoint failed!\n");

    ret = cr_checkpoint(0);
    if(ret < 0)
    {
        //失败处理
        /*.......
          .......
          .......
          .......
        */
    }
    else if(ret != 0)
    {
        //做restart工作
    }
    else
    {
        //做continue工作
        blcr_continue();
    }

    return 0;

}

static int cr_signal_callback(void *argv)
{
    int ret;
    ret = cr_checkpoint(0);

    if(ret < 0)
    {
        //失败处理
        /*.......
          .......
          .......
          .......
        */
    }
    else if(ret != 0)
    {
        //做restart工作
    }
    else
    {
        //做continue工作
        //blcr_continue();
    }

    return 0;
}

static int cr_signal_app(easy_proc *cr_world, int cr_world_n)
{

}

static int create_app_schema(easy_proc *cr_world, int cr_world_n)
{

}








