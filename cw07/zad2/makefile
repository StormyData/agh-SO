CFLAGS = -Wall -lpthread -lrt
CC = gcc

all: main cook delivery

main: main.o


main.o: main.c common.h



cook: cook.o

cook.o: cook.c common.h


delivery: delivery.o

delivery.o: delivery.c common.h

clean:
	rm -f *.o