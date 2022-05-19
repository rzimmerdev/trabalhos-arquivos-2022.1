#include <stdio.h>

#ifndef REGISTRY_H
#define REGISTRY_H

#define BUFFER_SIZE 32

typedef struct Registry_t {

    int total_fields;
    int *field_sizes;
    int *type_sizes;

    void **fields;

} registry;

registry *create_registry(int total_fields, int *field_sizes, int *type_sizes);

registry *read_registry(FILE *stream, int total_fields, int *field_sizes, int *type_sizes, char separator);

#endif //REGISTRY_H
