CC = g++
FLAGS = -Wall
LDFLAGS = -lprotobuf

SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)

EXEC = reader writer

reader: read.o person.pb.o
	$(CC) $^ $(FLAGS) $(LDFLAGS) -o $@

writer: write.o person.pb.o
	$(CC) $^ $(FLAGS) $(LDFLAGS) -o $@

%.o : %.cc
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXEC) $(OBJS)
