dependencies = lib/utils.o lib/record.o lib/table.o lib/index.o src/csv_utils.o src/commands.o
individual_case = 6

cases_input = cases2/in
cases_output = cases2/out

all: src/main.o $(dependencies)
	gcc -o main src/main.o $(dependencies) -g3

clean:
	rm lib/*.o src/*.o main saida.txt comparison.txt index.bin

run: src/main.c
	./bin/main

copy: all
	cp files2/* bin/ -r

valgrind: all
	make copy
	make all
	@cat $(cases_input)/$(individual_case).in
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./main < $(cases_input)/$(individual_case).in > saida.txt
	@cat saida.txt
	@echo Expected output:
	@cat $(cases_output)/$(individual_case).out

