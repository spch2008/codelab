CC = g++
RM = rm -rf
CFLAGS = -std=c++11 -lpthread -O2

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all : gcc_order.o cpu_order.o atomic_gcc_order.o
	$(CC) gcc_order.o $(CFLAGS) -o gcc_order
	$(CC) cpu_order.o $(CFLAGS) -o cpu_order
	$(CC) atomic_gcc_order.o $(CFLAGS) -o atomic_gcc_order

%.o : %.cpp
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(OBJS) gcc_order cpu_order atomic_gcc_order
