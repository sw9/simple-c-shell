#include <fcntl.h>
#include <unistd.h> 
#include <stdio.h>
#include "redirection.h"
#include <stdlib.h>

char **  redirect(char ** args, int arg_count) {
  int i;
  int to_count = 0;
  int to_pos = 0;

  int from_count = 0;
  int from_pos = 0;
  for (i = 0; i < arg_count; i++) {
    if (args[i][0] == '>') {
      to_count++;
      to_pos = i;
    } else if (args[i][0] == '<') {
      from_count++;
      from_pos = i;
    }
  }
  
  if (to_count > 1 || from_count >1) {
    printf("Error: Multiple redirections of the same type\n");
    return NULL;
  } else if (to_count == 0 && from_count == 0) {
    return remove_redirects(args, arg_count, to_count, to_pos, from_count, from_pos);
  }

  if (to_count == 1) {
    if (to_pos + 1 >= arg_count) {
      printf("Error: Missing redirection arguments\n");
      return NULL;
    }
    
    char x = args[to_pos + 1][0];
    
    if (x == '|' || x  == '&' || x == '<' || x == '>') {
      printf("Error: Missing redirection arguments\n");
      return NULL;
    }
  }

  if (from_count == 1) {
    if (from_pos + 1 >= arg_count) {
      printf("Error: Missing redirection arguments\n");
      return NULL;
    }

    char x = args[from_pos + 1][0];
    if (x == '|' || x  == '&' || x == '<' || x == '>') {
      printf("Error: Missing redirection arguments\n");
      return NULL;
    }
  }
  
  int f1 = -1;
  if (to_count == 1) {
    f1 = open(args[to_pos + 1],O_WRONLY|O_CREAT|O_TRUNC,0644);
    
    if (f1 == -1) {
      perror("open");
      return NULL;
    } else {
      int ret = dup2(f1,STDOUT_FILENO);

      if (ret == -1) {
	perror("dup2");
	close(f1);
	return NULL;
      }
    }
  }

  int f2 = -1;
  if (from_count == 1) {
    f2 = open(args[from_pos + 1], O_RDONLY);

    if (f2 == -1) {
      if (f1 != -1) {
	close(f1);
      }
 
      perror("open");
      return NULL;
    } else {
      int ret = dup2(f2,STDIN_FILENO);
      if (ret == -1) {
	if (f1 != -1) {
	  close(f1);
	}

	perror("dup2");
	close(f2);
	return NULL;
      }
    }
  }

  char ** new_args = remove_redirects(args, arg_count, to_count, to_pos, from_count, from_pos);

  if (new_args == NULL) {
    if (f1 != -1) {
      close(f1);
    }

    if (f2 != -1) {
      close(f2);
    }
  }

  return new_args;
}

char ** remove_redirects(char ** args, int arg_count, int to_count, int to_pos, int from_count, int from_pos) {

  int remove = 2*( to_count + from_count);

  char ** ret = malloc((arg_count - remove +1)*sizeof(char*));

  if (ret == NULL) {
    perror("malloc");
    return NULL;
  }

  int i = 0;
  int j = 0;
  while (args[i] != NULL) {
    
    int remove = 0;
    if (to_count == 1) {
      if (i == to_pos || i == to_pos + 1) {
	remove = 1;
      }
    }

    if (from_count == 1) {
      if (i == from_pos || i == from_pos + 1) {
	remove = 1;
      }
    }
    
    if (remove) {
      free(args[i]);
    } else {
      ret[j] = args[i];
      j++;
    }

    i++;
  }

  ret[arg_count - remove] = NULL;
  free(args);
  return ret;
}
