dependencies = lib/utils.o lib/record.o lib/table.o lib/index/index.o lib/index/tree_index.o src/csv_utils.o src/commands.o
individual_case = 9

trabalho = 3

cases_input = cases$(trabalho)/in
cases_output = cases$(trabalho)/out

all: src/main.o $(dependencies)
	gcc -o main src/main.o $(dependencies) -g3

clean:
	rm lib/*.o src/*.o main saida.txt comparison.txt index.bin

run: src/main.c
	./main

copy: all
	rm bin/*
	cp files$(trabalho)/* bin/ -r

valgrind: all
	make copy
	make all
	@cat $(cases_input)/$(individual_case).in
	@cd bin; valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ../main < ../$(cases_input)/$(individual_case).in > saida.txt
	@cat bin/saida.txt

