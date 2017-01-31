#include "tokenizer.h"
#include "launcher.h"
#include "pipe.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int connect_pipe(int* fd, int p) {
  
  if (p == 0) {
    close(fd[0]);
    int ret = dup2(fd[1],STDOUT_FILENO);
    
    if (ret == -1) {
      perror("dup2");
      close(fd[1]);
      return -1;
    }

  } else if (p == 1) {
    close(fd[1]);
    int ret = dup2(fd[0],STDIN_FILENO);
    
    if (ret == -1) {
      perror("dup2");
      close(fd[0]);
      return -1;
    }
  }

  return 0;
}

int pipe_syntax_check(char * string, char ** args, int arg_count) {
  int pipe_count = 0;
  int pipe_loc = 0;
  int i = 0;
  while (args[i] != NULL) {
    if (args[i][0] == '|') {
      pipe_count++;
      pipe_loc = i;
    }
    i++;
  }

  if (pipe_count == 0) {
    return 0;
  } else if (pipe_count > 1) {
    printf("Error: Only two-stage pipes are permitted \n");
    return -1;
  }

  if (pipe_loc == 0 || args[pipe_loc+1] == NULL) {
    printf("Error: Pipe is missing some arguments \n");
    return -1;
  }

  for (i = 0; i < pipe_loc; i++) {
    if (args[i][0] == '>') {
      printf("Error: Invalid combination of pipe and redirection \n");
      return -1;
    } 
  }

  for (i = pipe_loc + 1; i < arg_count; i++) {
    if (args[i][0] == '<') {
      printf("Error: Invalid combination of pipe and redirection \n");
      return -1;
    }
  }

  int ret = 0;
  while (string[ret] != '|') {
    ret++;
  }

  return ret;
}
