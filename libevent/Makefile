CC = gcc
RM = rm -rf
CFLAGS = -g -Wall

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

LIBS=-levent

EXEC = main

$(EXEC) : $(OBJS)
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@

%.o : %.c
	$(CC) -c $< -o $@

clean:
	$(RM) $(OBJS) $(EXEC)
