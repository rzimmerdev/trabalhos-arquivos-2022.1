/*
 * Nomes (G1): Danielle Modesti e Rafael Zimmer
 * nUSPs: 12543544 e 12542612
 * Disciplina: Organizacao de Arquivos - semestre (2022.1)
 * Trabalho 2 — frota de veículos no Brasil.
 *
 * Este trabalho visa inserir, remover e atualizar dados em arquivos binarios de
 * acordo com organizacoes de campos e registros diferentes, bem como desenvolver
 * funcionalidades para recuperar dados desses arquivos usando indices. 
 * 
 * As funcionalidades do trabalho 1 fornecem uma base a partir da qual as do
 * trabalho 2 sao desenvolvidas :)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/utils.h"
#include "commands.h"

// TODO: Add more comments

// Available op codes to be used for eight different commands
typedef enum Command_t {
    CREATE_TABLE = 1,
    SELECT = 2,
    SELECT_WHERE = 3,
    SELECT_ID = 4,
    CREATE_INDEX = 5,
    REMOVE_RECORDS = 6,
    INSERT_RECORDS = 7,
    UPDATE_RECORDS = 8
} command;

// Possible filetype encodings to be used when reading or writing binary files
typedef enum Filetype_t {
    FIXED = 1,
    VARIABLE = 0
} filetypes;


int main() {
    int option, filetype;
    scanf("%d ", &option);

    char *file_type_str = scan_word();
    if (file_type_str[4] == '1') {
        filetype = FIXED;
    }
    
    else {
        filetype = VARIABLE;
    }

    free(file_type_str);

    switch ((command)option) {
        case CREATE_TABLE: {
            char *csv_filename = scan_word();
            char *out_filename = scan_word();

            int status = create_table_command(csv_filename, out_filename, filetype);
            free(csv_filename);

            if (status != ERROR_CODE)
                binarioNaTela(out_filename);
            else
                printf("Falha no processamento do arquivo.");
            free(out_filename);

            break;
        }
        case SELECT: {
            char *bin_filename = scan_word();

            int status = select_command(bin_filename, filetype);
            free(bin_filename);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.");
            break;
        }
        case SELECT_WHERE: {

            char *bin_filename = scan_word();
            int total_parameters; scanf("%d ", &total_parameters);

            int status = select_where_command(bin_filename, total_parameters, filetype);
            free(bin_filename);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            break;
        }
        case SELECT_ID: {

            char *bin_filename = scan_word();
            int rrn; scanf("%d", &rrn);

            int status = select_rrn_command(bin_filename, rrn);
            free(bin_filename);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            break;
        }
        case CREATE_INDEX: {

            char *data_filename = scan_word();
            char *index_filename = scan_word();
            char data_path[4 + (int) strlen(data_filename)];
            strcpy(data_path, "bin/");
            strcat(data_path, data_filename);
            char index_path[4 + (int) strlen(index_filename)];
            strcpy(index_path, "bin/");
            strcat(index_path, index_filename);

            int status = create_index_command(data_path, index_path, filetype);
            free(data_filename);
            free(index_filename);

            if (status == SUCCESS_CODE)
                binarioNaTela(index_path);
            else if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            break;
        }

        case REMOVE_RECORDS: {

            char *data_filename = scan_word();
            char *index_filename = scan_word();
            char data_path[4 + (int) strlen(data_filename)];
            strcpy(data_path, "bin/");
            strcat(data_path, data_filename);
            char index_path[4 + (int) strlen(index_filename)];
            strcpy(index_path, "bin/");
            strcat(index_path, index_filename);

            int total_filters; scanf("%d ", &total_filters);
            int status = delete_records_command(data_path, index_path, total_filters, filetype);
            free(data_filename);
            free(index_filename);

            if (status == SUCCESS_CODE) {
                binarioNaTela(data_path);
                binarioNaTela(index_path);
            }
            else if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            break;
        }

        case INSERT_RECORDS: {
            char *data_filename = scan_word();
            char *index_filename = scan_word();
            
            char *data_path = (char *) malloc((5 + (int)strlen(data_filename)) * sizeof(char));
            data_path[0] = '\0';

            char *index_path = (char *) malloc((5 + (int)strlen(index_filename)) * sizeof(char));
            index_path[0] = '\0';

            strcpy(data_path, "bin/");
            strcat(data_path, data_filename);
            strcpy(index_path, "bin/");
            strcat(index_path, index_filename);

            int total_insertions; scanf("%d ", &total_insertions);

            int status = insert_records_command(data_path, index_path, total_insertions, filetype);

            if (status != ERROR_CODE) {
                binarioNaTela(data_path);
            }

            else {
                printf("Falha no processamento do arquivo.");
            }

            free(data_filename);
            free(index_filename);
            free(data_path);
            free(index_path);

            break;
        }

        case UPDATE_RECORDS: {
            char *data_filename = scan_word();
            char *index_filename = scan_word();
            char data_path[4 + (int) strlen(data_filename)];
            strcpy(data_path, "bin/");
            strcat(data_path, data_filename);
            char index_path[4 + (int) strlen(index_filename)];
            strcpy(index_path, "bin/");
            strcat(index_path, index_filename);

            int total_updates; scanf("%d ", &total_updates);

            int status = update_records_command(data_path, index_path, total_updates, filetype);
            free(data_filename);
            free(index_filename);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            break;
        }
    }

    return EXIT_SUCCESS;
}