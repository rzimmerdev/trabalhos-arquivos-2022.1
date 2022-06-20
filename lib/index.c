#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "record.h"
#include "table.h"


// TODO: Add comments

void create_index(FILE *o_stream, FILE *d_stream, bool is_fixed) {

    header placeholder = {.status = BAD_STATUS};

    fwrite(placeholder.status, sizeof(char), 1, d_stream);

    header table_header = fread_header(o_stream, is_fixed);

    for (int current_rrn = 0; current_rrn < table_header.next_rrn; current_rrn++) {


    }
}