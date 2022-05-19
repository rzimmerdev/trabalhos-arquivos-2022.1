dependencies = lib/registry.o lib/dataframe.o
case = test_cases/3.in

all: src/main.o $(dependencies)
	gcc -o main src/main.o $(dependencies)

run: src/main.c
	make all
	./main

valgrind: all
	make all
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main < $(case)
