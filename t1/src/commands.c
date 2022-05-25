#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


#include "../lib/utils.h"
#include "../lib/record.h"
#include "csv_utils.h"


typedef struct { char *key; int val; } table_dict;

static table_dict column_lookup[] = {
        { "id", 0 }, { "ano", 1 }, { "qtt", 2 }, { "sigla", 3 },
        { "cidade", 4}, { "marca", 5}, { "modelo", 6}
};


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
        return -1;
    }

    select_table(file_ptr, filetype);

    fclose(file_ptr);

    return 1;
}

int select_where_command(char *bin_filename, bool is_fixed) {

    // Ler a entrada
    int columns_amt; scanf("%d", &columns_amt);

    FILE *file_ptr = fopen(bin_filename, "rb");

    int next_rrn; long int next_byteoffset;

    if (is_fixed) {
        fseek(file_ptr, 174, SEEK_SET);
        fread(&next_rrn, 4, 1, file_ptr);
    }
    else {
        fseek(file_ptr, 178, SEEK_SET);
        fread(&next_byteoffset, 8, 1, file_ptr);
    }

    for (; field_amt > 0; field_amt--) {

        char *key = scan_word();
        char *value = scan_word_quoted();

        while ((is_fixed && (--next_rrn > 0)) || (!is_fixed && ftell(file_ptr) < next_byteoffset)) {
            data record = fread_record(file_ptr, is_fixed);
            printf_record(record);
            free_record(record);
        }

        free(key); free(value);
    }

    free(bin_filename);

    return 1;
}

int select_id_command(char *bin_filename, int rrn) {

    // Ler a entrada --- 
    // Esta func. so esta disponivel para arquivos de reg. fixos

    // Acessar o arquivo para recuperar dados
    FILE *file_ptr = fopen(bin_filename, "rb");

    if (!file_ptr) {
        return -1;
    }

    // Verificar se o RRN eh valido ---
    fseek(file_ptr, 174, SEEK_SET);
    int next_rrn; fread(&next_rrn, 4, 1, file_ptr);

    int byte_offset = rrn * 97 + 182; // O arquivo fixo tem 97 bytes de tamanho

    if (rrn < 0 || rrn >= next_rrn) {
        return -1;
    }

    fseek(file_ptr, byte_offset, SEEK_SET);

    data record = fread_record(file_ptr, false);
    fclose(file_ptr);

    if (record.removed == '0') {
        printf_record(record);
        free_record(record);
        return 1;
    }
    else
        return 0;
}