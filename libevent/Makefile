CC = gcc
RM = rm -rf
CFLAGS = -g -Wall

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

LIBS=-levent

SER = server
CLI = client

$(SER) : echo_server.o
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@

$(CLI) : echo_client.o
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@

%.o : %.c
	$(CC) -c $< -o $@

clean:
	$(RM) $(OBJS) $(SER)
