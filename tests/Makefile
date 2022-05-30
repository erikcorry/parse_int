.PHONY: run

run: test_parse_int
	./test_parse_int
	
test_parse_int: test_parse_int.c ../parse_int.h
	gcc -O3 -o test_parse_int test_parse_int.c
