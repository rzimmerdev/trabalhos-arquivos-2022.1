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

record *type1_data = create_record(data1_field_amt, data1_field_sizes, data1_type_sizes);

// Criando o cabecalho do arquivo de tipo 2 (header2) ---

int header2_field_amt = 15;
// Tamanho de cada campo no registro:
int header2_field_sizes[] = {1, 1, 40, 22, 19, 24, 8, 1, 16, 1, 18, 1, 19, 1, 1};
int header2_type_sizes[] = {1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 4};

record *type2_header = create_record(header2_field_amt, header2_field_sizes, header2_type_sizes);

// Criando o cabecalho do arquivo de tipo 2 (data2) ---

int data2_field_amt = 16;
// Tamanho de cada campo no registro:
int data2_field_sizes[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE, 1, 1, VAR_SIZE};
int data2_type_sizes[] = {1, 4, 8, 4, 4, 4, 2, 4, 1, 1, 4, 1, 1, 4, 1, 1};

record *type2_data = create_record(data2_field_amt, data2_field_sizes, data2_type_sizes);

#endif