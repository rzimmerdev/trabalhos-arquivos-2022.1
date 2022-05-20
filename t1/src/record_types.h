#ifndef RECORD_TYPES_H
#define RECORD_TYPES_H

#define VAR_SIZE -1 // Para campos cujos tamanhos nao sabemos (sao variaveis)
#define int_size

#include "../lib/record.h"

// Criando o cabecalho do arquivo de tipo 1 (header1) ---

int header1_field_amt = 15;
// Tamanho de cada campo no registro:
int header1_field_sizes[] = {1, 1, 40, 22, 19, 24, 8, 1, 16, 1, 18, 1, 19, 1, 1};
int header1_type_sizes[] = {1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4};

record *type1_header = create_record(total_fields, field_sizes, type_sizes);

// Criando os dados do arquivo de tipo 1 (data1) ---

int data1_field_amt = 15;
// Tamanho de cada campo no registro:
int data1_field_sizes[] = {1, 1, 1, 1, 1, 2, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE};
int data1_type_sizes[] = {1, 4, 4, 4, 4, 1, 4, 1, 1, 4, 1, 1, 4, 1, 1};

record *type1_data = create_record();

record *type2_header = create_record();

record *type2_data = create_record();


#endif