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

    return reg;
}


record *read_record(FILE *fp, record *template, char separator) {

    record *reg = create_record(template->total_fields, template->field_sizes, template->type_sizes);

    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < template->total_fields; i++) {

        if (template->field_sizes[i] == -1)
            reg->fields[i] = fscan_until(fp, separator);
        else {
            reg->fields[i] = malloc(template->type_sizes[i] * template->field_sizes[i]);
            fread(reg->fields[i], template->type_sizes[i], template->field_sizes[i], fp);
        }
    }

    return reg;
}


void save_record(FILE *dest, record *to_save) {

    for (int i = 0; i < to_save->total_fields; i++) {

        if (to_save->field_sizes[i] == -1) {

            fwrite(to_save->fields[i], strlen((char *) to_save->fields[i]), to_save->type_sizes[i], dest);
        }

        else {

            fwrite(to_save->fields[i], to_save->field_sizes[i], to_save->type_sizes[i], dest);
        }
    }
}


void free_record(record *to_free, bool is_template) {

    if (!is_template)
        for (int i = 0; i < to_free->total_fields; i++) {

            free(to_free->fields[i]);
        }

    free(to_free->fields);
    free(to_free);
}