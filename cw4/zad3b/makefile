CC=gcc -Wall -I .
all: sender catcher

sender: sender.o
	$(CC) sender.o -o sender

sender.o: sender.c
	$(CC) -g -c sender.c -o sender.o

catcher: catcher.o
	$(CC) catcher.o -o catcher

catcher.o: catcher.c
	$(CC) -g -c catcher.c -o catcher.o

clean:
	rm -f *.o
