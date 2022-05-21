#include <stdio.h>
#include <stdlib.h>

#include "record_types.h"
#include "../lib/record.h"


record *create_header() {
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

record *create_header_csv() {

    int field_sizes[7] = {VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE, VAR_SIZE};
    int type_sizes[7] = {4, 4, 1, 4, 1, 1, 1};

    return create_record(7, field_sizes, type_sizes);
}

record *create_data_csv() {

    int csv_template[7] = {1, 1, VAR_SIZE, 1, VAR_SIZE, VAR_SIZE, VAR_SIZE};
    int type_template[7] = {4, 4, 1, 4, 1, 1, 1};

    return create_record(7, csv_template, type_template);
}


void csv_to_record_fixed(FILE *csv, FILE *dest) {

    record *csv_header = create_header_csv();
    read_record(csv, csv_header, ',');
    free_record(csv_header);



    record *csv_data = create_data_csv();
    record *current_row = NULL;

    do {
        read_record(csv, csv_data, ',');
        current_row = create_data_fixed();
        free(csv_data);
        csv_data = create_data_csv();
        free_record(current_row);

        save_record();

    } while (current_row);

    free_record(csv_data);
}

void csv_to_record_variable() {


}