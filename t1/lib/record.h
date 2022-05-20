#include <stdio.h>

#ifndef RECORD_H
#define RECORD_H

#define BUFFER_SIZE 32

typedef struct Record_t {

    int total_fields;
    int *field_sizes;
    int *type_sizes;

    void **fields;

} record;

char *fscan_until(FILE *stream, char separator);

record *create_record(int total_fields, int *field_sizes, int *type_sizes);

record *read_record(FILE *fp, record *template, char separator);

#endif //RECORD_H
