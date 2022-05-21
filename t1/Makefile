dependencies = lib/utils.o lib/record.o src/commands.o
case = test_cases/1.in

all: src/main.o $(dependencies)
	gcc -o bin/main src/main.o $(dependencies) -g3

run: src/main.c
	make all
	./bin/main < $(case)

valgrind: all
	make all
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/main < $(case)
