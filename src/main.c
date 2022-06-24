/*
 * Nomes (G1): Danielle Modesti e Rafael Zimmer
 * nUSP: 12543544 e 12542612
 * Disciplina: Organizacao de Arquivos - semestre (2022.1)
 * Trabalho 1 — frota de veículos no Brasil.
 *
 * Este trabalho visa armazenar dados em arquivos binarios de
 * acordo com organizacoes de campos e registros diferentes, bem como
 * desenvolver funcionalidades para recuperar dados desses arquivos.
*/
#include <stdio.h>
#include <stdlib.h>

#include "../lib/utils.h"
#include "commands.h"

// TODO: Add more comments

// Available op codes to be used for four different commands
typedef enum Command_t {
    CREATE_TABLE = 1,
    SELECT = 2,
    SELECT_WHERE = 3,
    SELECT_ID = 4,
    CREATE_INDEX = 5,
} command;

// Possible filetype encodings to be used when reading or writting binary files
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

            char *bin_filename = scan_word();

            create_index_command(bin_filename, filetype);

            break;
        }
    }

    printf("\n");

    return 0;
}