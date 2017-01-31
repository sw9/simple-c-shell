#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

#include "list.h"
#include <sys/types.h> 

void handler(int i);
pid_t launch_helper(char *string, int p, int*fd, pid_t id, int fore, char *string2, node * nd);
void launch(char *string, pid_t * last_stopped, pid_t * last_executed, node ** first);
void free_ptr_array(char ** arr);
char ** argument_array(char * string, char** name, int * size);
void bg(pid_t last_stopped, node **first);
pid_t fg(pid_t last_executed, node ** first);

#endif
