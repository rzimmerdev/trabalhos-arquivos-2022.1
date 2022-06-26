#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../lib/record.h"
#include "../lib/utils.h"
#include "../lib/table.h"

// TODO: Add more comments

int csv_read_int(FILE *stream) {
    char *value = fscan_until(stream, ',');

    if (strlen(value)) {
        int casted = atoi(value);
        free(value);
        return casted;
    }

    else {
        free(value);
        return EMPTY;
    }
}


data scan_record_csv(FILE *fp, bool is_fixed, int *last_rrn, long int *next_byteoffset) {

    data record = {.removed = NOT_REMOVED};

    if (is_fixed)
        record.next = EMPTY;
    else {
        record.big_next = EMPTY;
    }

    fscanf(fp, "%d,", &record.id);

    record.year = csv_read_int(fp);

    record.city = fscan_until(fp, ',');
    record.city_size = strlen(record.city);

    record.total = csv_read_int(fp);

    char *state = fscan_until(fp, ',');
    if (strlen(state)) {
        record.state[0] = state[0]; record.state[1] = state[1];
    } else {
        record.state[0] = GARBAGE; record.state[1] = GARBAGE;
    }
    free(state);

    record.brand = fscan_until(fp, ',');
    record.brand_size = strlen(record.brand);
    record.model = fscan_until(fp, ',');
    record.model_size = strlen(record.model);

    if (!is_fixed) {
        record.size = VARIABLE_MINIMUM - 5;

        if (record.city_size) record.size += 5 + record.city_size;
        if (record.brand_size) record.size += 5 + record.brand_size;
        if (record.model_size) record.size += 5 + record.model_size;
        *next_byteoffset += record.size + 5;
    } else
        *last_rrn += 1;

    return record;
}

void csv_to_bin(FILE *csv, FILE *dest, bool is_fixed) {

    header placeholder = {.status=BAD_STATUS, .top=EMPTY, .big_top=EMPTY, .next_rrn=0,
            .next_byteoffset=0, .num_removed=0};
    write_header(dest, placeholder, is_fixed, 0);

    for (int i = 0; i < 7; i++)
        free(fscan_until(csv, ','));

    char is_eof;

    int last_rrn = 0;
    long int next_byteoffset = VARIABLE_HEADER;

    while ((is_eof = getc(csv)) != EOF) {
        ungetc(is_eof, csv);

        data record = scan_record_csv(csv, is_fixed, &last_rrn, &next_byteoffset);

        write_record(dest, record, is_fixed);
        free_record(record);
    }

    // Access NEXT_RRN or NEXT_BYTEOFFSET binary value in header
    fseek(dest, is_fixed ? NEXT_RRN_b : NEXT_BYTEOFFSET_b, SEEK_SET);

    if (is_fixed)
        fwrite(&last_rrn, 4, 1, dest);
    else
        fwrite(&next_byteoffset, 8, 1, dest);

    fseek(dest, 0, SEEK_SET);
    fwrite("1", 1, 1, dest);
}