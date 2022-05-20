#include <stdio.h>
#include <stdlib.h>

#include "record.h"


void fscan_until(FILE *stream, char *ptr, char separator) {

    int buffer = BUFFER_SIZE, i = 0;
    ptr = malloc(sizeof(char) * buffer);

    char current_char;
    while (fscanf(stream, "%c", &current_char) != EOF && current_char != '\n' && current_char != separator) {
        if (current_char != '\r') {
            if (buffer <= i) {
                buffer += BUFFER_SIZE;
                ptr = realloc(ptr, sizeof(char) * buffer);
            }
            ptr[i++] = current_char;
        }
    }

    if (current_char == EOF)
        ungetc(current_char, stream);

    ptr = realloc(ptr, (i + 1) * sizeof(char));
    ptr[i] = '\0';
}


record *create_record(int total_fields, int *field_sizes, int *type_sizes) {

    record *reg = malloc(sizeof(record));
    reg->total_fields = total_fields;
    reg->field_sizes = field_sizes;
    reg->type_sizes = type_sizes;
    reg->fields = malloc(total_fields * sizeof(void *));

    return reg;
}


record *read_record(FILE *fp, int total_fields, int *field_sizes, int *type_sizes, char separator) {

    record *reg = create_record(total_fields, field_sizes, type_sizes);

    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < total_fields; i++) {

        if (field_sizes[i] == -1)
            fscan_until(fp, reg->fields[i], separator);
        else {
            reg->fields[i] = malloc(type_sizes[i] * field_sizes[i]);
            fread(reg->fields[i], type_sizes[i], field_sizes[i], fp);
        }
    }

    return reg;
}