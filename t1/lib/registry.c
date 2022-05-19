#include <stdio.h>
#include <stdlib.h>

typedef struct Registry_t {

    int total_fields;
    int *field_sizes;
    int *type_sizes;

    void **fields;

} registry;


void read_variable_field(void *ptr, int type_size, char separator, FILE *stream) {

    // Buffer
    int field_size = 100;
    ptr = malloc(type_size * field_size);

    char current_char;
    fscanf(fp, )
    while (fscanf())
}


registry *read_fixed(FILE *fp, int total_fields, int *field_sizes, int *type_sizes) {

    registry *reg = malloc(sizeof(registry));
    reg->total_fields = total_fields;
    reg->field_sizes = field_sizes;
    reg->type_sizes = type_sizes;

    fseek(fp, 0, SEEK_SET);

    reg->fields = malloc(total_fields * sizeof(void *));
    for (int i = 0; i < total_fields; i++) {
        reg->fields[i] = malloc(type_sizes[i] * field_sizes[i]);
        fread(reg->fields[i], type_sizes[i], field_sizes[i], fp);
    }

    return reg;

}


registry *read_mixed(FILE *fp, char separator, int total_fields, int *field_sizes, int *type_sizes) {

    registry *reg = malloc(sizeof(registry));
    reg->total_fields = total_fields;
    reg->field_sizes = field_sizes;
    reg->type_sizes = type_sizes;

    fseek(fp, 0, SEEK_SET);

    reg->fields = malloc(total_fields);

    reg->fields = malloc(total_fields * sizeof(void *));
    for (int i = 0; i < total_fields; i++) {

        if (field_sizes[i] == -1)
            read_variable_field(reg->fields[i], type_sizes[i], separator, fp);
        else {

        }
        reg->fields[i] = malloc(type_sizes[i] * field_sizes[i]);
        fread(reg->fields[i], type_sizes[i], field_sizes[i], fp);
    }

    return reg;
}