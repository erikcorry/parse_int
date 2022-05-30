.PHONY: run

run: parse_bench
	sh -c "time ./parse_bench"
	
parse_bench: parse_bench.c ../parse_int.h
	gcc -O3 -o parse_bench parse_bench.c
