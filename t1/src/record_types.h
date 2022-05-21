#ifndef RECORD_TYPES_H
#define RECORD_TYPES_H

#define VAR_SIZE -1 // Para campos cujos tamanhos nao sabemos (sao variaveis)
#define int_size

#include "../lib/record.h"

record *create_data_fixed();

record *create_data_variable();

void csv_to_record_fixed(FILE *csv, FILE *dest);

void csv_to_record_variable();

#endif