#ifndef CHECKPOINT_H_INCLUDED
#define CHECKPOINT_H_INCLUDED

#include "parse.h"

int blcr_init(char *path, char **argv, easy_proc *world, int world_n);
void blcr_disable_checkpoint(void);
void blcr_enable_checkpoint(void);
int blcr_checkpoint(void);
int blcr_continue(void);


#endif // CHECKPOINT_H_INCLUDED
