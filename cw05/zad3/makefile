CC=gcc -Wall
all: producer consumer tester

consumer: consumer.o
	$(CC) consumer.o -o consumer

consumer.o: consumer.c
	$(CC) -g -c consumer.c -o consumer.o

producer: producer.o
	$(CC) producer.o -o producer

producer.o: producer.c
	$(CC) -g -c producer.c -o producer.o


tester: tester.o
	$(CC) tester.o -o tester

tester.o: tester.c
	$(CC) -g -c tester.c -o tester.o


clean:
	rm -f *.o
	rm -f out.txt*
	rm -f result{1..9}.txt
	rm -f in{1..5}.txt


test: all
	rm -f result.txt
	dd if=in1b.txt of=in1.txt bs=25 count=1
	dd if=in2b.txt of=in2.txt bs=25 count=1
	dd if=in3b.txt of=in3.txt bs=25 count=1
	dd if=in4b.txt of=in4.txt bs=25 count=1
	dd if=in5b.txt of=in5.txt bs=25 count=1

	cat in1.txt ln.txt in2.txt ln.txt in3.txt ln.txt in4.txt ln.txt in5.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 5 5 1
	cat out.txt0 ln.txt out.txt1 ln.txt out.txt2 ln.txt out.txt3 ln.txt out.txt4 > out.txt || true
	diff expected.txt out.txt > result1.txt 2>&1 || (echo "failed 1" >> result.txt)

	cat in1.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 5 1 5
	cat out.txt0 > out.txt || true
	diff expected.txt out.txt  > result2.txt 2>&1 || (echo "failed 2" >> result.txt)

	cat in1.txt ln.txt in2.txt ln.txt in3.txt ln.txt in4.txt ln.txt in5.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 5 5 5
	cat out.txt0 ln.txt out.txt1 ln.txt out.txt2 ln.txt out.txt3 ln.txt out.txt4 > out.txt || true
	diff expected.txt out.txt  > result3.txt 2>&1 || (echo "failed 3" >> result.txt)


	dd if=in1b.txt of=in1.txt bs=2500 count=1
	dd if=in2b.txt of=in2.txt bs=2500 count=1
	dd if=in3b.txt of=in3.txt bs=2500 count=1
	dd if=in4b.txt of=in4.txt bs=2500 count=1
	dd if=in5b.txt of=in5.txt bs=2500 count=1

	cat in1.txt ln.txt in2.txt ln.txt in3.txt ln.txt in4.txt ln.txt in5.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 500 5 1
	cat out.txt0 ln.txt out.txt1 ln.txt out.txt2 ln.txt out.txt3 ln.txt out.txt4 > out.txt || true
	diff expected.txt out.txt  > result4.txt 2>&1 || (echo "failed 4" >> result.txt)

	cat in1.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 500 1 5
	cat out.txt0 > out.txt || true
	diff expected.txt out.txt  > result5.txt 2>&1 || (echo "failed 5" >> result.txt)

	cat in1.txt ln.txt in2.txt ln.txt in3.txt ln.txt in4.txt ln.txt in5.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 500 5 5
	cat out.txt0 ln.txt out.txt1 ln.txt out.txt2 ln.txt out.txt3 ln.txt out.txt4 > out.txt || true
	diff expected.txt out.txt  > result6.txt 2>&1 || (echo "failed 6" >> result.txt)


	dd if=in1b.txt of=in1.txt bs=250000 count=1
	dd if=in2b.txt of=in2.txt bs=250000 count=1
	dd if=in3b.txt of=in3.txt bs=250000 count=1
	dd if=in4b.txt of=in4.txt bs=250000 count=1
	dd if=in5b.txt of=in5.txt bs=250000 count=1

	cat in1.txt ln.txt in2.txt ln.txt in3.txt ln.txt in4.txt ln.txt in5.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 8192 5 1
	cat out.txt0 ln.txt out.txt1 ln.txt out.txt2 ln.txt out.txt3 ln.txt out.txt4 > out.txt || true
	diff expected.txt out.txt  > result7.txt 2>&1 || (echo "failed 7" >> result.txt)

	cat in1.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 8192 1 5
	cat out.txt0 > out.txt || true
	diff expected.txt out.txt  > result8.txt 2>&1 || (echo "failed 8" >> result.txt)

	cat in1.txt ln.txt in2.txt ln.txt in3.txt ln.txt in4.txt ln.txt in5.txt > expected.txt
	rm -f out.txt*
	echo "" > out.txt
	./tester 8192 5 5
	cat out.txt0 ln.txt out.txt1 ln.txt out.txt2 ln.txt out.txt3 ln.txt out.txt4 > out.txt || true
	diff expected.txt out.txt  > result9.txt 2>&1 || (echo "failed 9" >> result.txt)