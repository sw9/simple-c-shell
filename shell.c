#include <stdio.h>
#include <unistd.h>
#include "launcher.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include "list.h"
#include <sys/types.h> 

/**
 * Main program execution
 */



int main( int argc, char *argv[] )
{
  node * first = NULL;

  struct sigaction new_action;
  new_action.sa_handler = SIG_IGN;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;
  sigaction (SIGTTOU, &new_action, NULL);
  sigaction (SIGTTIN, &new_action, NULL);
  sigaction (SIGTERM, &new_action, NULL);
  sigaction (SIGTSTP, &new_action, NULL);
  sigaction (SIGINT, &new_action, NULL);

  char string[1027] = "";
  int br;
  pid_t last_stopped = -1;
  pid_t last_executed = -1;

  string[1026] = '\0';	   /* ensure that string is always null-terminated */
  fprintf(stderr, "$ ");
  while ((br = read( STDIN_FILENO, string, 1026)) > 0) {
    if (br > 1025) {
      printf( "The line you've entered exceeds the 1024 char limit\n" );
      printf("$ ");
      fflush(stdout);

      continue;
    }
    
    string[br-1] = '\0';   /* remove trailing \n */
    /* tokenize string */

    launch(string, &last_stopped, &last_executed, &first);

    //printf("shell is reasserting terminal control\n");
    tcsetpgrp(STDIN_FILENO, getpgid(0));
    
    //get rid of all zombies
    node * nd = first;
    while (nd != NULL) {
      pid_t pid = nd->pid;
      int status;
      while ((waitpid(-pid, &status, WUNTRACED|WNOHANG)) > 0) {
	if (WIFSTOPPED(status)) {
	  last_stopped = pid;
	  node * x = lookup(first, pid);
	  if (x != NULL) {
	    x->print = 1;
	  }
	}

	if (WIFEXITED(status) || WIFSIGNALED(status)) {
	  node * x = lookup(first, pid);
	  if (x != NULL) {
	    x->pipe = x->pipe - 1;
	  }
	}
      }
      
      nd = nd->next;
    }

    node * x = first;
    while (x != NULL) {
      node * next = x->next;
      //printf("%s \t %d \t %d \n", x->string, x->pipe, x->print);

      if (x->pipe == -1) {
        if (!x->fore) {
          printf("Finished: %s\n", x->string);
        }
        first = delete(first, x);
      } else {
        if (x->print) {
          printf("Stopped: %s\n", x->string);
          x->print = 0;   
        }
      }
      x = next; 
    }

    printf("$ ");
    fflush(stdout);

  }

  node * x = first;
  while (x != NULL) {
    x = delete(x, x);
  }

  printf( "\nBye!\n" );
  return 0;			/* all's well that end's well */
}
