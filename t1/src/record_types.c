#include <stdio.h>
#include <stdlib.h>

#include "record_types.h"
#include "../lib/record.h"


void write_header(FILE *fp, bool is_fixed) {

    int multiplier = 2;
    if (is_fixed)
        multiplier = 1;

    int top = -1, next_rrn = 0, num_removed = 0;

    fwrite("0", 1, 1, fp);
    fwrite(&top, 4 * multiplier, 1, fp);
    fwrite("LISTAGEM DA FROTA DOS VEICULOS NO BRASIL", 1, 40, fp);
    fwrite("CODIGO IDENTIFICADOR:", 1, 22, fp);
    fwrite("ANO DE FABRICACAO: ", 1, 19, fp);
    fwrite("QUANTIDADE DE VEICULOS: ", 1, 24, fp);
    fwrite("ESTADO: ", 1, 8, fp);
    fwrite("0", 1, 1, fp);
    fwrite("NOME DA CIDADE: ", 1, 16, fp);
    fwrite("1", 1, 1, fp);
    fwrite("MARCA DO VEICULO: ", 1, 18, fp);
    fwrite("2", 1, 1, fp);
    fwrite("MODELO DO VEICULO: ", 1, 19, fp);
    fwrite(&next_rrn, 4 * multiplier, 1, fp);
    fwrite(&num_removed, 4, 1, fp);
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
    read_record(csv, csv_header, ','); free_record(csv_header);

    write_header(dest, true);

    /*record *csv_data = create_data_csv();
    record *current_row = NULL;

    do {
        read_record(csv, csv_data, ',');
        current_row = create_data_fixed();
        free(csv_data);
        csv_data = create_data_csv();
        free_record(current_row);


    } while (current_row);

    free_record(csv_data);*/
}

void csv_to_record_variable() {


}