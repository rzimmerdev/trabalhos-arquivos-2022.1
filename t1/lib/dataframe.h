#include "record.h"

#ifndef DATAFRAME_H
#define DATAFRAME_H

typedef struct Table_t {

    record *header;

    char type;
    int total_registries;
    record **registries;

} table;

table *read_mixed_file(FILE *fp, record *template, char separator);

#endif //DATAFRAME_H
