//
// Created by rzimmerdev on 25/05/2022.
//
#include <stdio.h>
#include <stdbool.h>
#include "record.h"

#ifndef T1_TABLE_H
#define T1_TABLE_H

typedef struct Header_t {

    char status;
    int top;
    long int big_top;

    int next_rrn;
    long int next_byteoffset;
    int next_removed;

} header;

void write_header(FILE *stream, bool is_fixed);

int select_table(FILE *stream, bool is_fixed);

int select_where(FILE *stream, data template, header header_template, bool is_fixed);

#endif //T1_TABLE_H
