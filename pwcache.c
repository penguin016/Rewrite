#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>

#define	HASH(x)		((x) & (64 - 1))

static struct usr_buf
{
    struct usr_buf *next;
    uid_t uid;
    char usrname[20];
} *usr_buf[64];

char *user_from_uid(uid_t uid)
{
    struct usr_buf **p;
    struct passwd *pw;

    p = &usr_buf[HASH(uid)];
    while(*p)
    {
        if((*p)->uid == uid)
            return ((*p)->usrname);
        p = &(*p)->next;
    }
    *p = (struct usr_buf*)malloc(sizeof(struct usr_buf));
    (*p)->uid = uid;
    pw = getpwuid(uid);
    if(!pw || strlen(pw->pw_name) >= 20)
        sprintf((*p)->usrname, pw->pw_name);
    else
        strcpy((*p)->usrname, pw->pw_name);
    (*p)->next = NULL;
    return ((*p)->usrname);
}

static struct grp_buf
{
    struct grp_buf *next;
    uid_t gid;
    char grpname[20];
} *grp_buf[64];

char *grp_from_gid(uid_t gid)
{
    struct grp_buf **p;
    struct group *gr;

    p = &grp_buf[HASH(gid)];
    while(*p)
    {
        if((*p)->gid == gid)
            return ((*p)->grpname);
        p = &(*p)->next;
    }
    *p = (struct grp_buf*)malloc(sizeof(struct grp_buf));
    (*p)->gid = gid;
    gr = getgrgid(gid);
    if(!gr || strlen(gr->gr_name) >= 20)
        sprintf((*p)->grpname, gr->gr_name);
    else
        strcpy((*p)->grpname, gr->gr_name);
    (*p)->next = NULL;
    return ((*p)->grpname);
}
