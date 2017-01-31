#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>


node *lookup (node *first, pid_t pid) {
  node *current = first;
  
  while (current != NULL) {
    if (pid == current->pid) {
      return current;
    } else {
      current = current->next;
    }
  }
  return NULL;
}

node *add (node *first, pid_t pid, char *string, int print, int pipe, int fore) {
  node *new;
  new = malloc(sizeof(*new));
  if (new == NULL) {
    perror("malloc");
    return NULL;
  }
  
  int i = 0;
  while (string[i] != '\0') {
	i++;
  }
  new->string = malloc (i + 1);
  if (new->string == NULL) {
    perror("malloc");
    free(new);
    return NULL;
  }

  i = 0;
  while (string[i] != '\0') {
    new->string[i] = string[i];
    i++;
  }
  new->string[i] = '\0';

  new->pid = pid;
  new->print = print;
  new->pipe = pipe;
  new->fore = fore;

  if (first == NULL) {
    new->next = NULL;
    new->prev = NULL;
  } else {
    new->next = first;
    new->prev = NULL;
    first->prev = new;
  }
  return new;
}

node *delete (node *first, node *nd) {
  node *prev, *next;
  
  if (nd == NULL || first == NULL) return first;
  
  prev = nd->prev;
  next = nd->next;

  if (prev)
    prev->next = nd->next;

  if (next)
    next->prev = nd->prev;

  free(nd->string);
  free(nd);
  
  if (nd == first) {
    return next;
  } else {
    return first;
  }
}
