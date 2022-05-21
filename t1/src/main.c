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
#include <string.h>

#include "../lib/record.h"
#include "../lib/utils.h"

// Funcionalidades do trabalho
typedef enum Command_t {
    CREATE_TABLE = 1,
    SELECT = 2,
    SELECT_WHERE = 3,
    SELECT_ID = 4
} command;

typedef enum File_type_t {
    TYPE_1 = 1,
    TYPE_2 = 2
} file_type;

int main() {
    int option, file_type;
    scanf("%d ", &option);
    
    char *file_type_str = scan_word();
    if (file_type_str[4] == '1') {
        file_type = TYPE_1;
    }
    
    else {
        file_type = TYPE_2;
    }

    free(file_type_str);

    switch ((command)option) {
        case CREATE_TABLE: {
            char *csv_filename = scan_word();
            char *out_filename = scan_word();

            free(csv_filename);
            free(out_filename);
            break;
        }
        case SELECT: {
            char *bin_filename = scan_word();

            free(bin_filename);
            break;
        }
        case SELECT_WHERE: {
            char *bin_filename = scan_word();
            int field_amt;
            scanf("%d", &field_amt);

            free(bin_filename);
            break;
        }
        case SELECT_ID: {
            char *bin_filename = scan_word();
            int RRN;
            scanf("%d", &RRN);

            free(bin_filename);
            break;
        }
    }

    return EXIT_SUCCESS;
}