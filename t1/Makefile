dependencies = lib/record.o src/record_types.o lib/utils.h
case = test_cases/3.in

all: src/main.o $(dependencies)
	gcc -o bin/main src/main.o $(dependencies) -g3

run: src/main.c
	make all
	./bin/main

valgrind: all
	make all
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/main
