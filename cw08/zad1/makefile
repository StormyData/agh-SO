CFLAGS = -Wall -pthread -lm
CC = gcc


main:
	gcc -Wall -pthread -lm main.o -o main

main.o: main.c block.h numbers.h common.h


gen_random_img: gen_random_img.o

gen_random_img.o: gen_random_img.c common.h


clean:
	rm -f *.o
	rm -f *.ascii.pgm


gen_times: main gen_random_img
	./gen_random_img 512 512 small.ascii.pgm
	./gen_random_img 10000 10000 big.ascii.pgm
	@echo "" > Times.txt
	@echo "./main 1 block small.ascii.pgm" >> Times.txt
	./main 1 block small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 2 block small.ascii.pgm" >> Times.txt
	./main 2 block small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 4 block small.ascii.pgm" >> Times.txt
	./main 4 block small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 8 block small.ascii.pgm" >> Times.txt
	./main 8 block small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 16 block small.ascii.pgm" >> Times.txt
	./main 16 block small.ascii.pgm out.ascii.pgm >> Times.txt

	@echo "./main 1 block big.ascii.pgm" >> Times.txt
	./main 1 block big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 2 block big.ascii.pgm" >> Times.txt
	./main 2 block big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 4 block big.ascii.pgm" >> Times.txt
	./main 4 block big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 8 block big.ascii.pgm" >> Times.txt
	./main 8 block big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 16 block big.ascii.pgm" >> Times.txt
	./main 16 block big.ascii.pgm out.ascii.pgm >> Times.txt


	@echo "./main 1 numbers small.ascii.pgm" >> Times.txt
	./main 1 numbers small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 2 numbers small.ascii.pgm" >> Times.txt
	./main 2 numbers small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 4 numbers small.ascii.pgm" >> Times.txt
	./main 4 numbers small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 8 numbers small.ascii.pgm" >> Times.txt
	./main 8 numbers small.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 16 numbers small.ascii.pgm" >> Times.txt
	./main 16 numbers small.ascii.pgm out.ascii.pgm >> Times.txt

	@echo "./main 1 numbers big.ascii.pgm" >> Times.txt
	./main 1 numbers big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 2 numbers big.ascii.pgm" >> Times.txt
	./main 2 numbers big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 4 numbers big.ascii.pgm" >> Times.txt
	./main 4 numbers big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 8 numbers big.ascii.pgm" >> Times.txt
	./main 8 numbers big.ascii.pgm out.ascii.pgm >> Times.txt
	@echo "./main 16 numbers big.ascii.pgm" >> Times.txt
	./main 16 numbers big.ascii.pgm out.ascii.pgm >> Times.txt