#include <stdio.h>
#include <stdlib.h>

#include "registry.h"
#include "dataframe.h"


table *read_csv(FILE *fp, char separator, registry *template) {

    table *dataframe = malloc(sizeof(table));

    registry *header = read_registry(fp, template->total_fields, template->field_sizes,
                                     template->type_sizes, ',');

    dataframe->header = header;
    dataframe->total_registries = BUFFER_SIZE;
    dataframe->registries = malloc(sizeof(registry *) * dataframe->total_registries);

    char current_char;
    int i = 0;
    while (fscanf(fp, "%c", &current_char) != EOF) {

        ungetc(current_char, fp);

        if (dataframe->total_registries <= i) {
            dataframe->total_registries += BUFFER_SIZE;
            dataframe->registries = realloc(dataframe->registries, sizeof(registry *) * dataframe->total_registries);
        }

        dataframe->registries[i++] = read_registry(fp, template->total_fields, template->field_sizes,
                                                   template->type_sizes, separator);
    }

    dataframe->total_registries = i;
    dataframe->registries = realloc(dataframe->registries, sizeof(registry *) * dataframe->total_registries);

    return dataframe;
}