CC = gcc
RM = rm -rf
CFLAGS = -g -Wall

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

LIBS=-levent

SER = server

$(SER) : $(OBJS)
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@

%.o : %.c
	$(CC) -c $< -o $@

clean:
	$(RM) $(OBJS) $(SER)
