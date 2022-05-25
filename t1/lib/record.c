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
    printf("QUANTIDADE DE VEICULOS: %d\n", record.amt);
    printf("\n");
}


void write_header(FILE *stream, bool is_fixed) {
    // Ao abrir para escrita, status deve ser 0 (arq. inconsistente/desatualizado)
    fwrite("0", 1, 1, stream);

    if (is_fixed) {
        int top = -1; // Nao ha registros removidos ainda
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
        long int next_byte_offset = 0;
        fwrite(&next_byte_offset, 8, 1, stream);
    }

    // Quantidade de registros logicamente marcados como removidos
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

    // Tratando campos de tamanho variavel ---
    int city_space = 0, brand_space = 0, model_space = 0; // Alocando espaco necessario
    if (strlen(record.city)) { // Somente incluir no reg. efetivamente se existir
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

    // Acabou o registro
    if (!is_fixed)
        return;

    // Precisa completar o reg. de tam. fixo com lixo
    for (int i = 19 + city_space + brand_space + model_space; i < 97; i++) {
        fwrite("$", 1, 1, stream);
    }
}

void read_record(FILE *stream, data record) {

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