dependencies = lib/utils.o lib/record.o lib/table.o src/csv_utils.o src/commands.o
case = test_cases/2.in

all: src/main.o $(dependencies)
	gcc -o bin/main src/main.o $(dependencies) -g3

run: src/main.c
	./bin/main

valgrind: all
	make all
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/main < $(case) > saida.txt
