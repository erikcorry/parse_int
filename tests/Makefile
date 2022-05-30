.PHONY: run
.PHONY: run32
.PHONY: run64

run: run32 run64

run64: test_parse_int
	./test_parse_int

run32: test_parse_int_32
	./test_parse_int_32
	
test_parse_int: test_parse_int.c ../parse_int.h Makefile
	$(CC) -Wall -O3 -o test_parse_int test_parse_int.c

test_parse_int_32: test_parse_int.c ../parse_int.h Makefile
	$(CC) -Wall -m32 -O3 -o test_parse_int_32 test_parse_int.c

clean:
	rm -f test_parse_int test_parse_int_32
