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

#include "../lib/dataframe.h"
#include "../lib/record.h"

// Funcionalidades do trabalho
typedef enum Command_t {
    CREATE_TABLE = 1,
    SELECT = 2,
    SELECT_WHERE = 3,
    SELECT_ID = 4
} command;

int main() {
    int option;
    scanf("%d", &option);

    switch ((command)option) {
        case CREATE_TABLE:
            break;
        case SELECT:
            break;
        case SELECT_WHERE:
            break;
        case SELECT_ID:
            break;
    }

    int total_fields = 7;
    // Tamanho de cada campo no registro:
    int field_sizes[] = {-1, -1, -1, -1, -1, -1, -1};
    int type_sizes[] = {4, 4, 1, 4, 1, 1, 1};

    record *template = create_record(total_fields, field_sizes, type_sizes);

    char filename[] = "input_files/arquivoEntrada1.csv";

    FILE *fp = fopen(filename, "r");

    record *header = read_record(fp, template, ',');
    printf("%s\n", (char *) header->fields[0]);

    return 0;
}