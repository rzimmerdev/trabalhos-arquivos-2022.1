#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include "../lib/utils.h"
#include "../lib/record.h"
#include "../lib/table.h"

#include "csv_utils.h"


int create_table_command(char *csv_filename, char *out_filename, bool filetype) {

    // Ler a entrada
    FILE *csvfile_ptr = fopen(csv_filename, "r"); // Recuperar dados do csv
    FILE *binfile_ptr = fopen(out_filename, "wb"); // Escrever no binario

    // Testando ponteiros de arquivo
    if (!(csvfile_ptr && binfile_ptr)) {
        return -1;
    }

    // Recuperando dados do csv e transferindo-os para o arquivo binario
    csv_to_bin(csvfile_ptr, binfile_ptr, filetype);

    fclose(csvfile_ptr);
    fclose(binfile_ptr);

    return 1;
}

int select_command(char *bin_filename, bool filetype) {

    // Ler a entrada
    FILE *file_ptr = fopen(bin_filename, "rb");

    if (!file_ptr) {
        return ERROR;
    }

    int status = select_table(file_ptr, filetype);

    fclose(file_ptr);

    return status;
}


typedef struct { char *key; int val; } table_dict;

static table_dict column_lookup[] = {
        { "id", 0 }, { "ano", 1 }, { "qtt", 2 }, { "sigla", 3 },
        { "cidade", 4}, { "marca", 5}, { "modelo", 6}
};


int lookup(char *key) {

    for (int i = 0; i < 7; i++) {
        table_dict *mapped = &column_lookup[i];

        if (strcmp(mapped->key, key) == 0)
            return mapped->val;
    }

    return -1;
}

int select_where_command(char *bin_filename, int total_parameters, bool is_fixed) {

    FILE *file_ptr = fopen(bin_filename, "rb");

    if (!file_ptr) {
        return ERROR;
    }

    header template_header = {};

    if (is_fixed) {
        get_file_size(file_ptr, &template_header.next_rrn, is_fixed);
        fseek(file_ptr, FIXED_HEADER, SEEK_SET);
    }
    else {
        get_file_size(file_ptr, &template_header.next_byteoffset, is_fixed);
        fseek(file_ptr, VARIABLE_HEADER, SEEK_SET);
    }


    data template = {.id = -1, .year = -1, .total = -1};
    for (int i = 0; i < total_parameters; i++) {
        char *column = scan_word();
        int fixed;
        char *variable;
        if (!strcmp(column, "id") || !strcmp(column, "ano") || !strcmp(column, "qtt"))
            scanf("%d ", &fixed);
        else {
            variable = scan_word_quoted(); getchar();
        }

        switch (lookup(column)) {
            case 0: {
                template.id = fixed;
                break;
            }
            case 1: {
                template.year = fixed;
                break;
            }
            case 2: {
                template.total = fixed;
                break;
            }
            case 3: {
                memcpy(template.state, variable, strlen(variable));
                free(variable);
                break;
            }
            case 4: {
                template.city = variable;
                break;
            }
            case 5: {
                template.brand = variable;
                break;
            }
            case 6: {
                template.model = variable;
                break;
            }
        }

        free(column);
    }

    int status = select_where(file_ptr, template, template_header, is_fixed);

    free_record(template);

    fclose(file_ptr);

    return status;
}

int select_id_command(char *bin_filename, int rrn) {

    // Ler a entrada ---
    // Esta func. so esta disponivel para arquivos de reg. fixos

    // Acessar o arquivo para recuperar dados
    FILE *file_ptr = fopen(bin_filename, "rb");

    if (!file_ptr) {
        return ERROR;
    }

    // Verificar se o RRN eh valido ---
    fseek(file_ptr, 174, SEEK_SET);
    int next_rrn; fread(&next_rrn, 4, 1, file_ptr);

    int byte_offset = rrn * 97 + 182; // O arquivo fixo tem 97 bytes de tamanho

    if ((rrn < 0) || (rrn > next_rrn)) {
        return NOT_FOUND;
    }

    fseek(file_ptr, byte_offset, SEEK_SET);

    data record = fread_record(file_ptr, true);
    fclose(file_ptr);

    if (record.removed == '0') {
        printf_record(record);
        free_record(record);
        return SUCCESS;
    }
    else
        return NOT_FOUND;
}