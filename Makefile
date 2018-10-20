#makefile for server

CC = gcc
Flags = -Wall

server : server.c

	$(CC) $(Flags) server.c -o server

clean:
	rm -f server
