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

void csv_to_record_fixed(FILE *csv, FILE *dest, record *header_template, record *data_template) {
    record *header = read_record(csv, header_template, ',');

}

void csv_to_record_variable() {

}