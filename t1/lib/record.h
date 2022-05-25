#include <stdio.h>
#include <stdbool.h>

#ifndef RECORD_H
#define RECORD_H

#define BAD_STATUS "0"

// Constantes do Cabecalho
#define NEXT_RRN_b 174
#define NEXT_BYTEOFFSET_b 178
#define FIXED_HEADER 182
#define VARIABLE_HEADER 190

// Constantes para os campos.
#define EMPTY -1
#define GARBAGE '$'
#define CITY_CODE '0'
#define BRAND_CODE '1'
#define MODEL_CODE '2'

// Constantes para os registradores
#define FIXED_MINIMUM 19
#define VARIABLE_MINIMUM 27
#define FIXED_REG_SIZE 97
#define IS_REMOVED '1'

// Status de retorno
#define ERROR -1
#define SUCCESS 1
#define NOT_FOUND 0


typedef struct Data_t {

    char removed;

    int size;
    long int big_next;

    int next;

    int id;
    int year;
    int total;
    char state[2];

    int city_size;
    char *city;

    int brand_size;
    char *brand;

    int model_size;
    char *model;

} data;


char *fscan_until(FILE *stream, char separator);

void free_record(data record);
void printf_record(data record);

void get_file_size(FILE *stream, void *size, bool is_fixed);

void write_header(FILE *dest, bool is_fixed);
void write_record(FILE *dest, data record, bool is_fixed);

data fread_record(FILE *stream, bool is_fixed);

#endif //RECORD_H
