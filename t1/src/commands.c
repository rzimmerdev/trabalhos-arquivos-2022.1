#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../lib/utils.h"
#include "../lib/record.h"
#include "csv_utils.h"
#include "../lib/record.h"


int create_table_command(char *csv_filename, char *out_filename, bool filetype) {
    // Ler a entrada

    FILE *csvfile_ptr = fopen(csv_filename, "r"); // Recuperar dados do csv
    FILE *binfile_ptr = fopen(out_filename, "wb"); // Escrever no binario

    if (!(csvfile_ptr && binfile_ptr)) { // Testando ponteiros de arquivo
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    // Recuperando dados do csv e transferindo-os para o arquivo binario
    csv_to_bin(csvfile_ptr, binfile_ptr, filetype);

    fclose(csvfile_ptr);
    fclose(binfile_ptr);

    return 1;
}

void select_command(bool filetype) {
    // Ler a entrada
    char *bin_filename = scan_word();

    FILE *file_ptr = NULL;
    file_ptr = fopen(bin_filename, "rb");

    if (!file_ptr) {
        printf("macaco");
    }

    select_table(file_ptr, filetype);

    if (bin_filename) {
        free(bin_filename);
    }
    fclose(file_ptr);
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

int select_id_command() {
    // Ler a entrada --- 
    // Esta func. so esta disponivel para arquivos de reg. fixos

    char *bin_filename = scan_word();
    printf("bin_filename: %s\n", bin_filename);
    int RRN;
    scanf("%d", &RRN);
    printf("RRN: %d\n", RRN);

    // Acessar o arquivo para recuperar dados
    FILE *src_bin_file = fopen(bin_filename, "rb");
    free(bin_filename);

    if (!src_bin_file) {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    // Verificar se o RRN eh valido ---
    int header_size = 182;
    int byte_offset = RRN * 97 + header_size; // O arquivo fixo tem 97 bytes de tamanho

    fseek(src_bin_file, 174, SEEK_SET);
    int next_available_RRN;
    fread(&next_available_RRN, 4, 1, src_bin_file);
    printf("next_available_RRN: %d\n", next_available_RRN);
    printf("byteoffset: %d\n", byte_offset);

    if (byte_offset >= next_available_RRN * 97 || RRN < 0) {
        printf("Registro inexistente.\n");
        return -1;
    }

    fseek(src_bin_file, byte_offset, SEEK_SET);

    data record = {}; // Inicializando reg. estatico

    char code_city, code_brand, code_model;

    fread(&record.removed, 1, 1, src_bin_file);

    if (record.removed == '1') {
        printf("Registro inexistente.\n");
        return -1;
    }

    fread(&record.next, 4, 1, src_bin_file);
    fread(&record.id, 4, 1, src_bin_file);
    fread(&record.year, 4, 1, src_bin_file);
    fread(&record.amt, 4, 1, src_bin_file);
    printf("qtd de veiculos: %d\n", record.amt);
    fread(&record.state, 1, 2, src_bin_file);
    printf("sigla: %s\n", record.state);

    // Checar se existe cidade, pois isso afeta outros campos
    // Nao terminei, esperando vc fazer a func. 2 bjs

    // printf_record(record);

    fclose(src_bin_file);

    return 1;
}