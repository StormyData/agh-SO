CC=gcc -Wall
all: main_nftw main_dir

main_nftw: main_nftw.o
	$(CC) main_nftw.o -o main_nftw

main_nftw.o: main_nftw.c
	$(CC) -c main_nftw.c -o main_nftw.o

main_dir: main_dir.o
	$(CC) main_dir.o -o main_dir

main_dir.o: main_dir.c
	$(CC) -c main_dir.c -o main_dir.o

test: main_dir main_nftw
	@echo "test parent directory"
	./main_nftw ..
	./main_dir ..
	@echo "test /dev"
	./main_nftw /dev
	./main_dir /dev
	@echo "test /tmp"
	./main_nftw /tmp
	./main_dir /tmp
clean:
	rm -f *.o