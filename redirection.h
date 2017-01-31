#ifndef __REDIRECTION_H__
#define __REDIRECTION_H__

char **  redirect(char ** args, int arg_count);
char ** remove_redirects(char ** args, int arg_count, int to_count, int to_pos, int from_count, int from_pos);

#endif
