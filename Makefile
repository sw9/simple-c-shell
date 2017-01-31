CFLAGS=-g -Wall
CC=gcc
SRCS=tokenizer.c list.c pipe.c redirection.c launcher.c shell.c
OBJS=tokenizer.o list.o pipe.o redirection.o launcher.o shell.o
LDFLAGS=
LIBS=

all:    shell

$(SRCS):
	$(CC) $(CFLAGS) -c $*.c

.PHONY:shell
shell: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o shell $(OBJS)

.PHONY:clean
clean:
	rm -f *.o shell
