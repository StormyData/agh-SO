CC=gcc -Wall
all: main

main: main.o
	$(CC) main.o -o main

main.o: main.c
	$(CC) -g -c main.c -o main.o


clean:
	rm -f *.o
	rm -f w*.txt

test: main
	@echo "test 2500000000 segmentów dzielone na 5000 procesów" > raport2.txt
	(time ./main 5000 500000) >> raport2.txt 2>&1
	rm -f w*.txt
	@echo "test 500000000 segmentóœ dzielone na 1000 procesów" >> raport2.txt
	(time ./main 1000 500000) >> raport2.txt 2>&1
	rm -f w*.txt
	@echo "test 500000000 segmentóœ dzielone na 5000 procesów" >> raport2.txt
	(time ./main 5000 100000) >> raport2.txt 2>&1
	rm -f w*.txt
	@echo "test 25000000000 segmentów dzielone na 5000 procesów" >> raport2.txt
	(time ./main 5000 500000) >> raport2.txt 2>&1
	rm -f w*.txt
	@echo "test 25000000000 segmentów dzielone na 5000 procesów" >> raport2.txt
	(time ./main 5000 5000000) >> raport2.txt 2>&1
	rm -f w*.txt
	@echo "test 25000000 segmentów dzielone na 5 procesów" >> raport2.txt
	(time ./main 5 5000000) >> raport2.txt 2>&1
	rm -f w*.txt
	@echo "test 25000000 segmentów dzielone na 5000 procesów" >> raport2.txt
	(time ./main 5000 5000) >> raport2.txt 2>&1
	rm -f w*.txt