#ifndef PWCACHE_H_INCLUDED
#define PWCACHE_H_INCLUDED

#include <sys/types.h>

char *user_from_uid(uid_t uid);
char *grp_from_gid(uid_t gid);

#endif // PWCACHE_H_INCLUDED
