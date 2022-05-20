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

int main() {
    int option, file_type;
    scanf("%d %d ", &option, &file_type);

    switch ((command)option) {
        case CREATE_TABLE: {
            char *csv_filename = scan_word();
            char *out_filename = scan_word();
            
            free(csv_filename);
            free(out_filename);
            break;
        }
        case SELECT:
            break;
        case SELECT_WHERE:
            break;
        case SELECT_ID:
            break;
    }

    return EXIT_SUCCESS;
}