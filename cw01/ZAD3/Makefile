
static:
	#make ../ZAD1/Makefile all
	gcc test.c -I ../ZAD1 -L ../ZAD1 -l wcstore -o test
shred:
	gcc test.c -I ../ZAD1 -L ../ZAD1 -l wcstore -o test


clean:
	rm -f *.o
	rm -fr ./tests

create_tests:
	rm -fr ./tests
	mkdir ./tests
	mkdir ./tests/small_files
	mkdir ./tests/medium_files
	mkdir ./tests/large_files
	for file in {1..10}; do \
		dd if=/dev/urandom of=./tests/small_files/$${file}.txt  bs=4K count=128   ;\
		dd if=/dev/urandom of=./tests/medium_files/$${file}.txt bs=4K count=1024  ;\
		dd if=/dev/urandom of=./tests/large_files/$${file}.txt  bs=4K count=4096  ;\
	done
do_tests:
	make static
	make do_test exec=

do_test:
	@echo "small files 1 file wc count" > $(raport_file)
	$(exec) wc_files 1 ./tests/small_files/1.txt >> $(raport_file)
	@echo "small files 5 file wc count" >> $(raport_file)
	$(exec) wc_files 5 ./tests/small_files/{1..5}.txt >> $(raport_file)
	@echo "small files 10 file wc count" >> $(raport_file)
	$(exec) wc_files 10 ./tests/small_files/{1..10}.txt >> $(raport_file)
	@echo "medium files 1 file wc count" >> $(raport_file)
	$(exec) wc_files 1 ./tests/medium_files/1.txt >> $(raport_file)
	@echo "medium files 5 file wc count" >> $(raport_file)
	$(exec) wc_files 5 ./tests/medium_files/{1..5}.txt >> $(raport_file)
	@echo "medium files 10 file wc count" >> $(raport_file)
	$(exec) wc_files 10 ./tests/medium_files/{1..10}.txt >> $(raport_file)
	@echo "large files 1 file wc count" >> $(raport_file)
	$(exec) wc_files 1 ./tests/large_files/1.txt >> $(raport_file)
	@echo "large files 5 file wc count" >> $(raport_file)
	$(exec) wc_files 5 ./tests/large_files/{1..5}.txt >> $(raport_file)
	@echo "large files 10 file wc count" >> $(raport_file)
	$(exec) wc_files 10 ./tests/large_files/{1..10}.txt >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "testing block delete" >> $(raport_file)
	@echo "small files file block delete" >> $(raport_file)
	$(exec) wc_files 1 ./tests/small_files/1.txt wc_files 5 ./tests/small_files/{1..5}.txt wc_files 10 ./tests/small_files/{1..10}.txt delete_block 0 delete_block 0 delete_block 0 >> $(raport_file)
	@echo "medium files file block delete" >> $(raport_file)
	$(exec) wc_files 1 ./tests/medium_files/1.txt wc_files 5 ./tests/medium_files/{1..5}.txt wc_files 10 ./tests/medium_files/{1..10}.txt delete_block 0 delete_block 0 delete_block 0 >> $(raport_file)
	@echo "large files file block delete" >> $(raport_file)
	$(exec) wc_files 1 ./tests/large_files/1.txt wc_files 5 ./tests/large_files/{1..5}.txt wc_files 10 ./tests/large_files/{1..10}.txt delete_block 0 delete_block 0 delete_block 0 >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "" >> $(raport_file)
	@echo "testing storing and freeing large blocks many times" >> $(raport_file)
	@echo "storing and freeing small file 1000 times"
	$(exec) store_file_repeated 1000 ./tests/small_files/1.txt >> $(raport_file)
	@echo "storing and freeing medium file 1000 times"
	$(exec) store_file_repeated 1000 ./tests/medium_files/1.txt >> $(raport_file)
	@echo "storing and freeing large file 1000 times"
	$(exec) store_file_repeated 1000 ./tests/large_files/1.txt >> $(raport_file)