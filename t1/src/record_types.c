#include <stdio.h>
#include <stdlib.h>

#include "record_types.h"
#include "../lib/record.h"
#include "../lib/dataframe.h"

record *create_header_fixed() {
    // Criando o cabecalho do arquivo de tipo 1 ---

    int field_amt = 15;
    // Tamanho de cada campo no registro:
    int field_sizes[] = {1, 1, 40, 22, 19, 24, 8, 1, 16, 1, 18, 1, 19, 1, 1};
    int type_sizes[] = {1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4};

    return create_record(field_amt, field_sizes, type_sizes);
}

record *create_header_variable() {
    // Criando o cabecalho do arquivo de tipo 2 ---

    int field_amt = 15;
    // Tamanho de cada campo no registro:
    int field_sizes[] = {1, 1, 40, 22, 19, 24, 8, 1, 16, 1, 18, 1, 19, 1, 1};
    int type_sizes[] = {1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 4};

    return create_record(field_amt, field_sizes, type_sizes);
}

record *create_data_fixed() {
    // Criando os dados do arquivo de tipo 1 ---

    int field_amt = 15;
    // Tamanho de cada campo no registro:
    int field_sizes[] = {1, 1, 1, 1, 1, 2, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE};
    int type_sizes[] = {1, 4, 4, 4, 4, 1, 4, 1, 1, 4, 1, 1, 4, 1, 1};

    return create_record(field_amt, field_sizes, type_sizes);
}

record *create_data_variable() {
    // Criando os dados do arquivo de tipo 2 ---

    int field_amt = 16;
    // Tamanho de cada campo no registro:
    int field_sizes[] = {1, 1, 1, 1, 1, 1, 2, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE};
    int type_sizes[] = {1, 4, 8, 4, 4, 4, 1, 4, 1, 1, 4, 1, 1, 4, 1, 1};

    return create_record(field_amt, field_sizes, type_sizes);
}

record *csv_header_template() {

    int field_sizes[7] = {VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE};
    int type_sizes[7] = {4, 4, 1, 4, 1, 1, 1};

    return create_record(7, field_sizes, type_sizes);
}

record *csv_data_template() {

    int csv_template[7] = {1, 1, VAR_SIZE, 1, VAR_SIZE, VAR_SIZE, VAR_SIZE};
    int type_template[7] = {4, 4, 1, 4, 1, 1, 1};

    return create_record(7, csv_template, type_template);
}


void csv_to_record_fixed(FILE *csv, FILE *dest, record *header_template, record *data_template) {

    record *header = read_record(csv, header_template, ',');

    record *current_row = NULL;

    do {

    } while (current_row);
}

void csv_to_record_variable() {


}