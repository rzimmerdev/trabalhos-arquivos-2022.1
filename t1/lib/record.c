#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "record.h"


void free_record(data record) {
    if (record.city)
        free(record.city);
    if (record.model)
        free(record.model);
    if (record.brand)
        free(record.brand);
}


void printf_record(data record) {

    if (record.brand)
        printf("MARCA DO VEICULO: %s\n", record.brand);
    else
        printf("MARCA DO VEICULO: NAO PREENCHIDO\n");
    if (record.model)
        printf("MARCA DO VEICULO: %s\n", record.model);
    else
        printf("MARCA DO VEICULO: NAO PREENCHIDO\n");
    printf("ANO DE FABRICACAO: %d\n", record.year);
    if (record.city)
        printf("MARCA DO VEICULO: %s\n", record.city);
    else
        printf("MARCA DO VEICULO: NAO PREENCHIDO\n");
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

data fread_record(FILE *stream, bool is_fixed) {

    data record = {.city_size = 0,
                   .brand_size = 0,
            .model_size = 0,
    };

    fread(&record.removed, 1, 1, stream);

    if (record.removed == '1') {
        fseek(stream, 96, SEEK_CUR);
        return record;
    }

    if (is_fixed)
        fread(&record.next, 4, 1, stream);
    else {
        fread(&record.next, 4, 1, stream);
    }

    fread(&record.id, 4, 1, stream);
    fread(&record.year, 4, 1, stream);
    fread(&record.amt, 4, 1, stream);

    fread(record.state, 1, 2, stream);

    if (record.state[0] == '$')
        record.state[0] = '\0'; record.state[1] = '\0';

    int bytes_read = 19;

    for (int i = 0; i < 3; i++) {
        char is_empty; fread(&is_empty, 1, 1, stream);

        ungetc(is_empty, stream);
        if (is_empty == '$')
            break;

        int current_size;
        char current_code;
        fread(&current_size, 4, 1, stream);
        fread(&current_code, 1, 1, stream);

        switch (current_code) {
            case '0': {
                record.city_size = current_size;
                record.city = malloc(sizeof(char) * current_size + 1);
                fread(record.city, 1, current_size, stream); record.city[current_size] = '\0';
                break;
            }
            case '1': {
                record.brand_size = current_size;
                record.brand = malloc(sizeof(char) * current_size + 1);
                fread(record.brand, 1, current_size, stream); record.brand[current_size] = '\0';
                break;
            }
            case '2': {
                record.model_size = current_size;
                record.model = malloc(sizeof(char) * current_size + 1);
                fread(record.model, 1, current_size, stream); record.model[current_size] = '\0';
                break;
            }
        }

        bytes_read += 5 + current_size;
    }
    fseek(stream, 97 - bytes_read, SEEK_CUR);

    return record;
}


int select_table(FILE *stream, bool is_fixed) {

    if (getc(stream) == '0') {
        printf("Falha no processamento do arquivo.");
        return -1;
    }

    fseek(stream, is_fixed ? 174 : 189, SEEK_SET);

    int next_rrn; long int next_byteoffset, current_byteoffset = 0;
    if (is_fixed)
        fread(&next_rrn, 4, 1, stream);
    else
        fread(&next_byteoffset, 8, 1, stream);

    fseek(stream, is_fixed ? 182 : 189, SEEK_SET);

    while ((is_fixed && (next_rrn > 0)) || (!is_fixed && current_byteoffset < next_byteoffset)) {

        data scanned = fread_record(stream, is_fixed);

        if (scanned.removed == '1') {
            printf("Registro inexistente.\n");
        }

        next_rrn--; current_byteoffset += 97;
        printf_record(scanned);
        free_record(scanned);
    }
    return 1;
}