#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o segel.o client.o queue.o ultra_queue.o thread_data.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall 

LIBS = -lpthread -lm

.SUFFIXES: .c .o 

all: server 

server: server.o request.o segel.o queue.o ultra_queue.o thread_data.o
	$(CC) $(CFLAGS) -o server server.o request.o segel.o queue.o ultra_queue.o thread_data.o $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $< -lm

clean:
	-rm -f $(OBJS) server client output.cgi
	-rm -rf public
