#ifndef __PIPE_H__
#define __PIPE_H__

int connect_pipe(int* fd, int p);
int pipe_syntax_check(char*string, char ** args, int arg_count);

#endif
