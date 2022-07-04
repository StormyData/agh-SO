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

test: main_lib main_sys
	@echo "timing lib version" > pomiar_zad2.txt
	(time ./main_lib s testfile.txt) 2>> pomiar_zad2.txt
	@echo "timing sys version" >> pomiar_zad2.txt
	(time ./main_sys s testfile.txt) 2>> pomiar_zad2.txt
	@echo "test grep nof lines"
	grep s testfile.txt | wc -l