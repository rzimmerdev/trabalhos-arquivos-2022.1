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


void get_file_size(FILE *stream, void *size, bool is_fixed) {
    if (is_fixed) {
        fseek(stream, NEXT_RRN_b, SEEK_SET);
        int next_rrn;
        fread(&next_rrn, sizeof(int), 1, stream);
        memcpy(size, &next_rrn, sizeof(int));
    } else {
        fseek(stream, NEXT_BYTEOFFSET_b, SEEK_SET);
        long int next_byteoffset;
        fread(&next_byteoffset, sizeof(long int), 1, stream);
        memcpy(size, &next_byteoffset, sizeof(long int));
    }
}


void printf_record(data record) {
    char empty[] = "NAO PREENCHIDO";
    printf("MARCA DO VEICULO: %s\n", record.brand ? record.brand : empty);
    printf("MODELO DO VEICULO: %s\n", record.model ? record.model : empty);
    if (record.year != -1)
        printf("ANO DE FABRICACAO: %d\n", record.year);
    else
        printf("ANO DE FABRICACAO: %s\n", empty);
    printf("NOME DA CIDADE: %s\n", record.city ? record.city : empty);
    printf("QUANTIDADE DE VEICULOS: %d\n", record.total);
    printf("\n");
}


void write_record(FILE *stream, data record, bool is_fixed) {

    char code_city = CITY_CODE, code_brand = BRAND_CODE, code_model = MODEL_CODE;

    fwrite(&record.removed, 1, 1, stream);
    if (is_fixed) {
        fwrite(&record.next, 4, 1, stream);
    } else {
        fwrite(&record.size, 4, 1, stream);
        fwrite(&record.big_next, 8, 1, stream);
    }
    fwrite(&record.id, 4, 1, stream);
    fwrite(&record.year, 4, 1, stream);
    fwrite(&record.total, 4, 1, stream);

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
    for (int i = FIXED_MINIMUM + city_space + brand_space + model_space; i < FIXED_REG_SIZE; i++) {
        char garbage[1] = {GARBAGE};
        fwrite(garbage, 1, 1, stream);
    }
}

data fread_record(FILE *stream, bool is_fixed) {

    data record = {.city_size = 0, .brand_size = 0, .model_size = 0};

    fread(&record.removed, 1, 1, stream);

    if (record.removed == IS_REMOVED) {
        if (is_fixed) {
            fseek(stream, 96, SEEK_CUR);
        }
        else {
            fread(&record.size, 4, 1, stream);
            fseek(stream, record.size, SEEK_CUR);
        }
        return record;
    }

    if (is_fixed)
        fread(&record.next, 4, 1, stream);
    else {
        fread(&record.size, 4, 1, stream);
        fread(&record.big_next, 8, 1, stream);
    }

    fread(&record.id, 4, 1, stream);
    fread(&record.year, 4, 1, stream);
    fread(&record.total, 4, 1, stream);

    fread(record.state, 1, 2, stream);

    if (record.state[0] == GARBAGE)
        record.state[0] = '\0'; record.state[1] = '\0';

    int bytes_read = is_fixed ? FIXED_MINIMUM : VARIABLE_MINIMUM;

    for (int i = 0; i < 3; i++) {
        if (is_fixed) {
            char is_empty; fread(&is_empty, 1, 1, stream);

            ungetc(is_empty, stream);
            if (is_empty == GARBAGE)
                break;
        }
        else if (bytes_read >= record.size) {
            break;
        }

        int current_size;
        char current_code;
        fread(&current_size, 4, 1, stream);
        fread(&current_code, 1, 1, stream);

        switch (current_code) {
            case CITY_CODE: {
                record.city_size = current_size;
                record.city = malloc(sizeof(char) * current_size + 1);
                fread(record.city, 1, current_size, stream); record.city[current_size] = '\0';
                break;
            }
            case BRAND_CODE: {
                record.brand_size = current_size;
                record.brand = malloc(sizeof(char) * current_size + 1);
                fread(record.brand, 1, current_size, stream); record.brand[current_size] = '\0';
                break;
            }
            case MODEL_CODE: {
                record.model_size = current_size;
                record.model = malloc(sizeof(char) * current_size + 1);
                fread(record.model, 1, current_size, stream); record.model[current_size] = '\0';
                break;
            }
        }

        bytes_read += 5 + current_size;
    }

    if (is_fixed)
        fseek(stream, FIXED_REG_SIZE - bytes_read, SEEK_CUR);

    return record;
}
