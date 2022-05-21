#include <stdio.h>
#include <stdbool.h>

#ifndef RECORD_H
#define RECORD_H

typedef struct Data_t {

    char removed;

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

void free_record(data record);
void printf_record(data record);

void write_header(FILE *dest, bool is_fixed);
void write_record(FILE *dest, data record, bool is_fixed);

#endif //RECORD_H
