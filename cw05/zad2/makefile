CC=gcc -Wall
all: main dummy_mail

dummy_mail: dummy_mail.o
	$(CC) dummy_mail.o -o dummy_mail

dummy_mail.o: dummy_mail.c
	$(CC) -g -c dummy_mail.c -o dummy_mail.o

main: main.o
	$(CC) main.o -o main

main.o: main.c
	$(CC) -g -c main.c -o main.o

clean:
	rm -f *.o

test: main dummy_mail
	./main ./din.txt
	./main ./dout.txt email subject
