dependencies = lib/utils.o lib/record.o lib/table.o lib/index.o src/csv_utils.o src/commands.o
individual_case = cases1/in/22.in

cases_input = cases1/in
cases_output = cases2/out

all: src/main.o $(dependencies)
	gcc -o bin/main src/main.o $(dependencies) -g3

run: src/main.c
	./bin/main

RANGE = 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
test: all
	make all
	for CASE_ID in $(RANGE); do \
  		echo $$CASE_ID; \
		./bin/main < $(cases_input)/$${CASE_ID}.in > saida.txt; \
    	cmp --silent saida.txt $(cases_output)/$${CASE_ID}.out || echo Caso: $${CASE_ID}: Correct; \
    done

valgrind: all
	make all
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/main < $(individual_case) > saida.txt
	cat saida.txt

