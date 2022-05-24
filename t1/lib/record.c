#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "record.h"


void free_record(data record) {

    free(record.city); free(record.model); free(record.brand);
}


void printf_record(data record) {

    printf("MARCA DO VEICULO: %s\n", record.brand);
    printf("MODELO DO VEICULO: %s\n", record.model);
    printf("ANO DE FABRICACAO: %d\n", record.year);
    printf("NOME DA CIDADE: %s\n", record.city);
    printf("QAUNTIDADE DE VEICULOS: %d\n", record.amt);
    printf("\n");
}


void write_header(FILE *stream, bool is_fixed) {
    fwrite("0", 1, 1, stream);

    if (is_fixed) {
        int top = -1;
        fwrite(&top, 4, 1, stream);
    } else {
        long int top = -1;
        fwrite(&top, 8, 1, stream);
    }

    char *header_description[11] = {"LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
                                    "CODIGO IDENTIFICADOR: ", "ANO DE FABRICACAO: ", "QUANTIDADE DE VEICULOS: ",
                                    "ESTADO: ", "0", "NOME DA CIDADE: ", "1", "MARCA DO VEICULO: ",
                                    "2", "MODELO DO VEICULO: "};
    for (int i = 0; i < 11; i++) {
        fwrite(header_description[i], 1, strlen(header_description[i]), stream);
    }

    if (is_fixed) {
        int next_rrn = 0;
        fwrite(&next_rrn, 4, 1, stream);
    } else {
        long int byte_offset = 0;
        fwrite(&byte_offset, 8, 1, stream);
    }

    int removed_records = 0;
    fwrite(&removed_records, 4, 1, stream);
}


void write_record(FILE *stream, data record, bool is_fixed) {

    char code_city = '0', code_brand = '1', code_model = '2';

    fwrite(&record.removed, 1, 1, stream);
    if (is_fixed) {
        fwrite(&record.next, 4, 1, stream);
    } else {
        fwrite(&record.size, 4, 1, stream);
        fwrite(&record.big_next, 8, 1, stream);
    }
    fwrite(&record.id, 4, 1, stream);
    fwrite(&record.year, 4, 1, stream);
    fwrite(&record.amt, 4, 1, stream);

    if (strlen(record.state)) {
        fwrite(record.state, 1, 2, stream);
    }
    else {
        fwrite("$$", 1, 2, stream);
    }

    int city_space = 0, brand_space = 0, model_space = 0;
    if (strlen(record.city)) {
        fwrite(&record.city_size, 4, 1, stream);
        fwrite(&code_city, 1, 1, stream);
        fwrite(record.city, 1, record.city_size, stream);
        city_space = record.city_size + 4 + 1;
    }

    if (strlen(record.brand)) {
        fwrite(&record.brand_size, 4, 1, stream);
        fwrite(&code_brand, 1, 1, stream);
        fwrite(record.brand, 1, record.brand_size, stream);
        brand_space = record.brand_size + 4 + 1;
    }

    if (strlen(record.model)) {
        fwrite(&record.model_size, 4, 1, stream);
        fwrite(&code_model, 1, 1, stream);
        fwrite(record.model, 1, record.model_size, stream);
        model_space = record.model_size + 4 + 1;
    }

    if (!is_fixed)
        return;

    for (int i = 19 + city_space + brand_space + model_space; i < 97; i++) {
        fwrite("$", 1, 1, stream);
    }
}


data fscanf_record(FILE *stream, bool is_fixed) {

    data record = {};

    fscanf(stream, "%c", &record.removed);

    if (record.removed)
        return record;

    fseek(stream, 96, SEEK_CUR);

    return record;
}


int print_table(FILE *stream, data *record, bool is_fixed) {

    char status;
    if ((status = getc(stream)) == '0') {
        printf("Falha no processamento do arquivo.");
        return -1;
    }
    fseek(stream, is_fixed ? 181 : 189, SEEK_SET);

    char is_eof;
    while ((is_eof = getc(stream)) != EOF) {

        // ungetc(is_eof, stream);
        // data scanned = scan_record(stream, is_fixed)

        // printf_record(scanned);

        // free_record(scanned);
        break;
    }
    return 1;
}