CC=gcc -Wall
all: main_fork main_exec

main_fork: main_fork.o
	$(CC) main_fork.o -o main_fork

main_fork.o: main_fork.c
	$(CC) -g -c main_fork.c -o main_fork.o

main_exec: main_exec.o
	$(CC) main_exec.o -o main_exec

main_exec.o: main_exec.c
	$(CC) -g -c main_exec.c -o main_exec.o

clean:
	rm -f *.o

test: main_fork main_exec
	@echo "--------------------fork-------------------" > raport2.txt
	@echo "ignore" >> raport2.txt
	./main_fork ignore >> raport2.txt
	@echo "handler" >> raport2.txt
	./main_fork handler >> raport2.txt
	@echo "mask" >> raport2.txt
	./main_fork mask >> raport2.txt
	@echo "pending" >> raport2.txt
	./main_fork pending >> raport2.txt
	@echo "--------------------exec-------------------" >> raport2.txt
	@echo "ignore" >> raport2.txt
	./main_exec ignore >> raport2.txt
	@echo "mask" >> raport2.txt
	./main_exec mask >> raport2.txt
	@echo "pending" >> raport2.txt
	./main_exec pending >> raport2.txt
	@echo "----------------------------------" >> raport2.txt
	@echo "jak widać w przypadkach fork i exec:" >> raport2.txt
	@echo "sygnały ignorowane są dalej ignorowane po wywołanu fork i exec" >> raport2.txt
	@echo "sygnały maskowane są dalej maskowane po wywołanu fork i exec" >> raport2.txt
	@echo "sygnały oczekujące są dalej oczekujące po wywołanu exec ale nie fork" >> raport2.txt
	@echo "handlery sygnałów są te same w przypadku fork, " >> raport2.txt
	@echo "w przypadku exec kod handlera przestaje istnieć, więc nie może być przezkzany" >> raport2.txt
