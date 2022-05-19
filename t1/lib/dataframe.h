#ifndef DATAFRAME_H
#define DATAFRAME_H

typedef struct Table_t {

    registry *header;

    char type;
    int total_registries;
    registry **registries;

} table;

table *read_csv(FILE *fp, char separator, registry *template);

#endif //DATAFRAME_H
