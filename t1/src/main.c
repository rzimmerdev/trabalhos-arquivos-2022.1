/*
 * Nomes dos integrantes da equipe (G1): Danielle Modesti e Rafael Zimmer
 * Nos USP: 12543544 e 12542612
 * Disciplina: Organizacao de Arquivos - 3o semestre (2022.1)
 * Primeiro Trabalho Pratico - frota de veiculos no Brasil.
 *
 * Este trabalho tem como  objetivo armazenar dados em arquivos binarios de
 * acordo com organizacoes de campos e registros diferentes, bem como
 * desenvolver funcionalidades para recuperar dados desses arquivos.
 * 
*/

#include <stdio.h>
#include <stdlib.h>

#include "../lib/utils.h"
#include "../lib/record.h"
#include "commands.h"

// Funcionalidades do trabalho
typedef enum Command_t {
    CREATE_TABLE = 1,
    SELECT = 2,
    SELECT_WHERE = 3,
    SELECT_ID = 4
} command;

// Tipo de arquivo
typedef enum File_type_t {
    TYPE_1 = true,
    TYPE_2 = false
} filetypes;


int main() {
    int option, filetype;
    scanf("%d ", &option);
    
    char *file_type_str = scan_word();
    if (file_type_str[4] == '1') {
        filetype = TYPE_1;
    }
    
    else {
        filetype = TYPE_2;
    }

    free(file_type_str);

    switch ((command)option) {
        case CREATE_TABLE: {
            char *csv_filename = scan_word();
            char *out_filename = scan_word();

            int success = create_table_command(csv_filename, out_filename, filetype);

            if (success)
                binarioNaTela(out_filename);
            else
                printf("Falha no processamento do arquivo.");
            
            free(out_filename);
            free(csv_filename);
            break;
        }
        case SELECT: {
            char *bin_filename = scan_word();

            int success = select_command(bin_filename, filetype);

            if (success == -1)
                printf("Falha no processamento do arquivo.");


            free(bin_filename);

            break;
        }
        case SELECT_WHERE: {
            select_where_command();
            break;
        }
        case SELECT_ID: {

            int rrn; scanf("%d", &rrn);

            char *bin_filename = scan_word();

            int success = select_id_command(bin_filename, rrn);

            if (success == -1)
                printf("Falha no processamento do arquivo.");
            else if (success == 0)
                printf("Registro inexistente.\n");

            free(bin_filename);
        }
    }

    return EXIT_SUCCESS;
}