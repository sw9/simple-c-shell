#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "redirection.h"
#include "launcher.h"
#include "pipe.h"
#include <signal.h>
#include "list.h"
#include <sys/types.h> 

void free_ptr_array(char ** arr)
{
  int j = 0;
  while (arr[j] != NULL) {
    free(arr[j]);
    j++;
  }
  free(arr);
}

char ** argument_array(char * string, char** name, int * size) {
  TOKENIZER *tokenizer;
  TOKENIZER *count;  
  char *tok;
  char *file_name = NULL;
  char ** args;
  
  count = init_tokenizer(string);
  int arg_count = 0;

  while( (tok = get_next_token(count)) != NULL ) {
    if (arg_count == 0) {
      file_name = tok;
    } else {
      free(tok);    /* free the token now that we're done with it */
    }
    arg_count++;
  }
  free_tokenizer(count); /* free memory */

  args = malloc((arg_count +1)*sizeof(char*));    
  if (args == NULL) {
    perror("malloc");
    free(file_name);
    return NULL;
  }

  tokenizer = init_tokenizer( string );
  arg_count = 0;
  while( (tok = get_next_token( tokenizer )) != NULL ) {
    args[arg_count] = tok;
    arg_count++;
  }
  args[arg_count] = NULL;
  free_tokenizer( tokenizer ); /* free memory */


  *name = file_name;
  *size = arg_count;
  return args;
}

void handler(int i) {
  printf("Handled signal %d\n", i);
}

pid_t launch_helper(char *string, int p, int*fd, pid_t id, int fore, char *string2, node * nd) {

  char *file_name;
  char ** args;
  int arg_count;
  args = argument_array(string, &file_name, &arg_count);    
  if (args == NULL) {
    return -1;
  }

  if (file_name == NULL) {
    free(file_name);
    free_ptr_array(args);
    return -1;
  }

  if (!fore) {
    if (p == 0) {
      fprintf(stderr, "Running: %s|%s\n", string, string2);
    } else if (p == -1) {
      fprintf(stderr, "Running: %s\n", string);
    }
  }
  
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    free(file_name);
    free_ptr_array(args);
 
    return -1;
  }

  if (pid == 0) {
    node * x = nd;
    while (x != NULL) {
      x = delete(x, x);
    }

    int pgid_result;
    if (id == -1) {
      pgid_result = setpgid(0,0);
    } else {
      pgid_result = setpgid(0,id);
    }

    if (pgid_result == -1) {
      perror("pgid");
      if (p >= 0) {
        close(fd[1]);
        close(fd[0]);
      }
      free(file_name);
      free_ptr_array(args);
      exit(EXIT_FAILURE);
    }

    if (fore) {
      int pid = getpgid(0);
 
      int foreground = tcsetpgrp(STDIN_FILENO, pid);
      if (foreground == -1) {
	perror("tcgetpgrp");
	if (p >= 0) {
	  close(fd[1]);
	  close(fd[0]);
	}
	free(file_name);
	free_ptr_array(args);
	exit(EXIT_FAILURE);
      }
    }
    
    char** redirect_args = redirect(args, arg_count);
    if (redirect_args == NULL) {
      free(file_name);
      free_ptr_array(args);
      exit(EXIT_FAILURE);;
    }

    struct sigaction new_action;
    new_action.sa_handler = SIG_DFL;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction (SIGTTOU, &new_action, NULL);
    sigaction (SIGTTIN, &new_action, NULL);
    sigaction (SIGTERM, &new_action, NULL);
    sigaction (SIGTSTP, &new_action, NULL);
    sigaction (SIGINT, &new_action, NULL);
    
    if (p >= 0) {
      int ret = connect_pipe(fd, p);
      if (ret == -1) {
	free(file_name);
	free_ptr_array(redirect_args);
	exit(EXIT_FAILURE);
      }
    }

    execvp(file_name, redirect_args);

    perror("execvp");
    if (p == 0) {
      close(fd[1]);
    } else if (p == 1) {
      close(fd[0]);
    }

    free_ptr_array(redirect_args);
    free(file_name);
    exit(EXIT_FAILURE);
  }

   free(file_name);
   free_ptr_array(args);

   return pid;
}

void bg(pid_t last_stopped, node **first){
  node * x = lookup(*first, last_stopped);
  if (x != NULL) {
    printf("Running: %s\n", x->string);
    x->print = 0;
    x->fore = 0;
  }

  int ret = killpg(last_stopped, SIGCONT);
  if (ret == -1) {
    perror("killpg");
    printf("bg failed: No stopped jobs to continue\n");
  }
}

pid_t fg(pid_t last_executed, node ** first) {
  int status;
  while ((waitpid(-last_executed, &status, WNOHANG)) > 0) {
    if (WIFEXITED(status)) {
      node * x = lookup(*first, last_executed);
      if (x != NULL) {
	x->pipe = x->pipe - 1;
      }
    }
  }

  node * x = lookup(*first, last_executed);
  int ret = killpg(last_executed, SIGCONT);

  if (ret == 0) { //process group exists
    //printf("%d \t %d \t %d\n", ret, last_executed, getpgid(last_executed));
    if (x != NULL) {
      x->print = 0;
      printf("%s\n", x->string);
    }

    int foreground = tcsetpgrp(STDIN_FILENO, last_executed);    
    if (foreground == 0) {
      x->fore = 1;
      return last_executed;
    } else {
      perror("tcgetpgrp");
    }
  } else {
    perror("killpg");
    printf("fg failed: last job already terminated\n");
  }

  return -1;
}

void launch(char *string, pid_t * last_stopped, pid_t * last_executed, node ** first) {
  char *file_name;
  char ** args;
  int arg_count;

  int foreground = 1;
  pid_t pid = -1;
  
  args = argument_array(string, &file_name, &arg_count);
  if (args == NULL) {
    return;
  }
  
  if (file_name == NULL) {
    free_ptr_array(args);
    free(file_name);
    return;
  }

  if (args[0][0] == 'b' && args[0][1] == 'g' && args[0][2] == '\0') {
    if (arg_count == 1) {
      bg(*last_stopped, first);
    } else {
      printf("Invalid Syntax: bg command takes no arguments\n");
    }

    free(file_name);
    free_ptr_array(args);
    return;
  }

  if (args[0][0] == 'f' && args[0][1] == 'g' && args[0][2] == '\0') {
    if (arg_count == 1) {
      pid = fg(*last_executed, first);
    } else {
      printf("Invalid Syntax: fg command takes no arguments\n");
      free(file_name);
      free_ptr_array(args);
      return;
    }

    free(file_name);
    free_ptr_array(args);
    
  } else {

    int pipe_check = pipe_syntax_check(string, args, arg_count);
 
    if (arg_count != 0) {
      if (args[arg_count-1][0] == '&') {
	foreground = 0;
      }
    }  
    free_ptr_array(args);
    free(file_name);

    if (pipe_check == -1) {
      return;
    }

    int i = 0;
    int amp_pos = -1;
    while (string[i] != '\0') {
      if (string[i] == '&') {
	amp_pos = i;
      }
      i++;
    }
    if (amp_pos != -1) {
      string[amp_pos] = '\0';
    }

    int fd[2];
    if (pipe_check > 0) {
      int ret = pipe(fd);
      if (ret == -1) {
	perror("pipe");
	return;
      }
    }

    int has_pipes = 0;
    if (pipe_check > 0) {
      has_pipes = 1;
      string[pipe_check] = '\0';

      pid = launch_helper(string, 0, fd, -1, foreground, string+1+pipe_check, *first);
      setpgid(pid,pid);

      if (pid != -1) {
	int ret = launch_helper(string+1+pipe_check, 1, fd, pid, foreground, NULL, *first);
	setpgid(ret,pid);
      } else {
	printf("Error: Couldn't launch first part of the pipe, skipping the rest \n");
      }

      close(fd[1]);
      close(fd[0]);
    } else {
      pid = launch_helper(string, -1, NULL, -1, foreground, NULL, *first);
      setpgid(pid, pid);
    }

    if (pipe_check > 0) {
      string[pipe_check] = '|';
    }

    if (pid != -1) {
      node * new_first  = add(*first, pid, string, 0, has_pipes, foreground);
      if (new_first != NULL) {
	*first = new_first;
      }
      *last_executed = pid;
    }
  }

  if (foreground && pid != -1) {

    int status;
    while (waitpid(-pid, &status, WUNTRACED) > -1) {
      if (WIFSTOPPED(status)) {
	killpg(pid, SIGTSTP); //make sure entire process group is stopped
	*last_stopped = pid;
	node * x = lookup(*first, pid);
	if (x != NULL) {
	  x->print = 1;
	}
	return;
      }

      if (WIFEXITED(status) || WIFSIGNALED(status)) {
	node * x = lookup(*first, pid);
	if (x != NULL) {
	  x->pipe = x->pipe - 1;
	}
      }
    }
  }

  return;
}

