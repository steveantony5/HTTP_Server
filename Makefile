#makefile for server

CC = gcc
Flags = -Wall -g

server : server.c

	$(CC) $(Flags) server.c -o server

clean:
	rm -f server

run:
	./server 8000

