#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../lib/utils.h"
#include "../lib/record.h"
#include "csv_utils.h"
#include "../lib/record.h"

// Status de retorno
#define ERROR -1
#define SUCCESS 1
#define MISSING_REGISTER 0

// Informacao sobre os registros
#define FIXED_REG_SIZE 97
#define HEADER_SIZE 182

// Informacao sobre o campo 'removido'
#define IS_REMOVED '1'
#define IS_NOT_REMOVED '0'

int create_table_command(char *csv_filename, char *out_filename, bool filetype) {

    // Ler a entrada
    FILE *csvfile_ptr = fopen(csv_filename, "r"); // Recuperar dados do csv
    FILE *binfile_ptr = fopen(out_filename, "wb"); // Escrever no binario

    // Testando ponteiros de arquivo
    if (!(csvfile_ptr && binfile_ptr)) {
        return ERROR;
    }

    // Recuperando dados do csv e transferindo-os para o arquivo binario
    csv_to_bin(csvfile_ptr, binfile_ptr, filetype);

    fclose(csvfile_ptr);
    fclose(binfile_ptr);

    return SUCCESS;
}

int select_command(char *bin_filename, bool filetype) {

    // Ler a entrada
    FILE *file_ptr = NULL;
    file_ptr = fopen(bin_filename, "rb");

    if (!file_ptr) {
        return ERROR;
    }

    select_table(file_ptr, filetype);

    fclose(file_ptr);

    return SUCCESS;
}

void select_where_command() {
    // Ler a entrada
    char *bin_filename = scan_word();
    int field_amt;
    scanf("%d", &field_amt);
    char **search_fields = (char **) malloc(field_amt * sizeof(char *));
    
    for (int i = 0; i < field_amt; i++) {
        search_fields[i] = scan_word();
        printf("%s\n", search_fields[i]);
    }

    for (int i = 0; i < field_amt; i++) {
        free(search_fields[i]);
    }

    free(search_fields);
    free(bin_filename);
}

int select_id_command(char *bin_filename, int rrn) {

    // Ler a entrada --- 
    // Esta func. so esta disponivel para arquivos de reg. fixos

    // Acessar o arquivo para recuperar dados
    FILE *bin_file = fopen(bin_filename, "rb");

    if (!bin_file) {
        return ERROR;
    }

    // Verificar se o RRN eh valido ---
    fseek(bin_file, 174, SEEK_SET);
    int next_rrn; fread(&next_rrn, 4, 1, bin_file);

    int byte_offset = rrn * FIXED_REG_SIZE + HEADER_SIZE; // O arquivo fixo tem 97 bytes de tamanho

    if (rrn < 0 || rrn >= next_rrn) {
        return ERROR;
    }

    fseek(bin_file, byte_offset, SEEK_SET);

    data record = fread_record(bin_file, false);
    fclose(bin_file);

    if (record.removed == IS_NOT_REMOVED) {
        printf_record(record);
        free_record(record);
        return SUCCESS;
    }
    else
        return MISSING_REGISTER;
}