CFLAGS = -Wall -g
CC = gcc
all: server client cleaner_upper

server: server.o

server.o: server.c common.h


client: client.o

client.o: client.c common.h


cleaner_upper: cleaner_upper.o

cleaner_upper.o: cleaner_upper.c


clean:
	rm -f *.o