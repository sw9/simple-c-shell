#ifndef __LIST_H__
#define __LIST_H__
#include <sys/types.h>


typedef struct node_tag {
  pid_t pid;
  char *string;
  int print;
  int pipe;
  int fore;
  struct node_tag *prev, *next;
} node;

node *lookup (node *first, pid_t pid);
node *add (node *first, pid_t pid, char* string, int print, int pipe, int fore);
node *delete (node *first, node * nd);

#endif
