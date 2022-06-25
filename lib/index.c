#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "record.h"
#include "table.h"


int comp (const void * elem1, const void * elem2)
{
    long int *f = (long int *) elem1;
    long int *s = (long int *) elem2;
    if (f[0] > s[0]) return  1;
    if (f[0] < s[0]) return -1;
    return 0;
}


// TODO: Add comments
void create_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {

    update_status(index_stream, BAD_STATUS);
    header table_header = fread_header(origin_stream, is_fixed);

    int current_rrn = 0;
    long int current_byteoffset = ftell(origin_stream);

    int buffer = BUFSIZ, total_indexes = 0;
    long int (*indexes)[2] = malloc(buffer * sizeof(long int [2]));

    while ((is_fixed && (current_rrn < table_header.next_rrn)) || (!is_fixed && current_byteoffset < table_header.next_byteoffset)) {
        data to_indexate = fread_record(origin_stream, is_fixed);
        if (to_indexate.removed == IS_REMOVED) {
            current_rrn += 1;
            current_byteoffset = ftell(origin_stream);

            free_record(to_indexate);
            continue;
        }

        if (buffer <= total_indexes) {
            buffer += BUFSIZ;
            indexes = realloc(indexes, sizeof(long int [2]) * buffer);
        }

        indexes[total_indexes][0] = (long int) to_indexate.id;
        indexes[total_indexes][1] = (long int) is_fixed ? current_rrn : current_byteoffset;
        total_indexes++;

        current_rrn += 1;
        current_byteoffset = ftell(origin_stream);
        free_record(to_indexate);
    }

    qsort(indexes, total_indexes, sizeof(long int [2]), comp);

    for (int i = 0; i < total_indexes; i++) {

        int rrn = (int) indexes[i][1];
        long int byteoffset = indexes[i][1];

        fwrite((int *) indexes[i], sizeof(int), 1, index_stream);
        if (is_fixed)
            fwrite(&rrn, sizeof(int), 1, index_stream);
        else
            fwrite(&byteoffset, sizeof(long int), 1, index_stream);
    }
    free(indexes);

    update_status(index_stream, OK_STATUS);
}


