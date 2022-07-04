CC=gcc -Wall
all: main

main: main.o
	$(CC) main.o -o main

main.o: main.c
	$(CC) -g -c main.c -o main.o

clean:
	rm -f *.o

#test: main
#	@echo "testy" > raport.txt
#	./main SIGINFO >> raport.txt
#	@echo "-------" >> raport.txt
#	./main NOCLDSTOP >> raport.txt
#	@echo "-------" >> raport.txt
#	#./main RESETHAND >> raport.txt
#	@echo "-------" >> raport.txt
#	@echo "jak widać wykorzystuąc SA_SIGINFO otrzymujemy dużo więcej informacji o sygnale niż tylko jego numer" >> raport.txt
#	@echo "SA_NOCLDSTOP powoduje, że proces nie jest powiadamiany o wstrzymywaniu i wznawianiu działania procesu potomnego" >> raport.txt
#	@echo "SA_RESETHAND powoduje, że handler jest rejestrowany tylko na jeden raz, po którym powraca do domyślnego" >> raport.txt