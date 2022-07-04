CC=gcc -Wall
all: main_sys main_lib

main_sys: main_sys.o
	$(CC) main_sys.o -o main_sys

main_sys.o: main_sys.c
	$(CC) -c main_sys.c -o main_sys.o

main_lib: main_lib.o
	$(CC) main_lib.o -o main_lib

main_lib.o: main_lib.c
	$(CC) -c main_lib.c -o main_lib.o


clean:
	rm -f *.o
	rm -f out1.txt
	rm -f out2.txt

test: main_lib main_sys
	@echo "timing lib version" > pomiar_zad1.txt
	(time ./main_lib testfile.txt out1.txt) 2>> pomiar_zad1.txt
	@echo "timing sys version" >> pomiar_zad1.txt
	(time ./main_sys testfile.txt out2.txt) 2>> pomiar_zad1.txt