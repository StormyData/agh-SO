all: static shared clean

shared:
	gcc -fPIC -c wc_store.c -o wc_store_s.o -Wall
	gcc -shared wc_store_s.o -o wc_store.so -Wall
static:
	gcc -c wc_store.c -Wall
	ar rcs libwcstore.a wc_store.o
clean:
	rm -f *.o