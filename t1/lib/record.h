#include <stdio.h>
#include <stdbool.h>

#ifndef RECORD_H
#define RECORD_H

#define BUFFER_SIZE 32

typedef struct Data_t {

    char removed[1];

    int size;
    long int big_next;

    int next;

    int id;
    int year;
    int amt;
    char state[2];

    int city_size;
    char *city;

    int brand_size;
    char *brand;

    int model_size;
    char *model;

} data;


char *fscan_until(FILE *stream, char separator);

void csv_to_bin(FILE *csv, FILE *dest, bool is_fixed);

void printf_record(data record);

#endif //RECORD_H
