CC=gcc -Wall
all: main

main: main.o
	$(CC) main.o -o main

main.o: main.c
	$(CC) -c main.c -o main.o


clean:
	rm -f *.o

test: main
	@echo "test"
	./main 5
