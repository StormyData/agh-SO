all: main

main: main.o
	gcc -Wall -pthread -lm main.o -o main

main.o: main.c
	gcc -c -Wall -pthread -lm main.c -o main.o

clean:
	rm -f *.o