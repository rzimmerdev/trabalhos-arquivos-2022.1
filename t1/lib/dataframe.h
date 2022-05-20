#include "record.h"

#ifndef DATAFRAME_H
#define DATAFRAME_H

typedef struct Table_t {

    record *header;

    char type;
    int total_registries;
    record **registries;

} table;

table *read_csv(FILE *fp, char separator, record *template);

#endif //DATAFRAME_H
