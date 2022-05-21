#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "record.h"


char *fscan_until(FILE *stream, char separator) {

    int buffer = BUFFER_SIZE, i = 0;
    char *ptr = malloc(sizeof(char) * buffer);

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

    return ptr;
}


record *create_record(int total_fields, int *field_sizes, int *type_sizes) {

    record *reg = malloc(sizeof(record));
    reg->total_fields = total_fields;
    reg->field_sizes = field_sizes;
    reg->type_sizes = type_sizes;
    reg->fields = malloc(total_fields * sizeof(void *));

    for (int i = 0; i < total_fields; i++) {
        if (reg->field_sizes[i] < 0)
            continue;

        reg->fields[i] = malloc(reg->type_sizes[i] * reg->field_sizes[i]);
    }

    return reg;
}


void read_record(FILE *fp, record *placeholder, char separator) {

    if (feof(fp))
        return;

    for (int i = 0; i < placeholder->total_fields; i++) {

        if (placeholder->field_sizes[i] == -1)
            placeholder->fields[i] = fscan_until(fp, separator);
        else {
            fread(placeholder->fields[i], sizeof(placeholder->type_sizes[i]), placeholder->field_sizes[i], fp);
        }
    }
}


void save_record(FILE *dest, record *to_save) {

    for (int i = 0; i < to_save->total_fields; i++) {

        if (to_save->field_sizes[i] == -1) {

            fwrite(to_save->fields[i], strlen((char *) to_save->fields[i]), to_save->field_sizes[i], dest);
        } else {

            fwrite(to_save->fields[i], sizeof(to_save->type_sizes[i]), to_save->field_sizes[i], dest);
        }
    }
}


void free_record(record *to_free) {
    for (int i = 0; i < to_free->total_fields; i++) {

        free(to_free->fields[i]);
    }

    free(to_free->fields);
    free(to_free);
}