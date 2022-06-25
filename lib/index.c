#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "record.h"
#include "table.h"


// TODO: Add comments
void create_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {

    update_status(index_stream, BAD_STATUS);
    header table_header = fread_header(origin_stream, is_fixed);

    int current_rrn = 0;
    long int current_byteoffset = ftell(origin_stream);

    while ((is_fixed && (current_rrn < table_header.next_rrn)) || (!is_fixed && current_byteoffset < table_header.next_byteoffset)) {
        data to_indexate = fread_record(origin_stream, is_fixed);
        if (to_indexate.removed == IS_REMOVED) {
            current_rrn += 1;
            current_byteoffset = ftell(origin_stream);

            free_record(to_indexate);
            continue;
        }

        fwrite(&to_indexate.id, sizeof(int), 1, index_stream);

        if (is_fixed)
            fwrite(&current_rrn, sizeof(int), 1, index_stream);
        else
            fwrite(&current_byteoffset, sizeof(long int), 1, index_stream);

        current_rrn += 1;
        current_byteoffset = ftell(origin_stream);
        free_record(to_indexate);
    }

    update_status(index_stream, OK_STATUS);
}


