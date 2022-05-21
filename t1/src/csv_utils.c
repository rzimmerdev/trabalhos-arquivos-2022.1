#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../lib/record.h"


data scan_record_csv(FILE *fp, bool is_fixed, int *last_rrn, long int *next_byteoffset) {

    data record = {};

    if (is_fixed)
        record.next = -1;
    else {
        record.big_next = -1;
    }
    record.removed = '0';
    fscanf(fp, "%d,", &record.id);

    char *year = fscan_until(fp, ',');
    if (strlen(year)) {
        record.year = atoi(year);
    } else {
        record.year = -1;
    }
    free(year);

    record.city = fscan_until(fp, ',');
    record.city_size = strlen(record.city);

    char *amt = fscan_until(fp, ',');
    if (strlen(amt)) {
        record.amt = atoi(amt);
    } else {
        record.amt = -1;
    }
    free(amt);

    char *state = fscan_until(fp, ',');
    if (strlen(state)) {
        record.state[0] = state[0]; record.state[1] = state[1];
    } else {
        record.state[0] = '$'; record.state[1] = '$';
    }
    free(state);

    record.brand = fscan_until(fp, ',');
    record.brand_size = strlen(record.brand);
    record.model = fscan_until(fp, ',');
    record.model_size = strlen(record.model);

    if (!is_fixed) {
        record.size = 22;
        if (record.city_size) record.size += 5 + record.city_size;
        if (record.brand_size) record.size += 5 + record.brand_size;
        if (record.model_size) record.size += 5 + record.model_size;
        *next_byteoffset += record.size;
    } else
        *last_rrn += 1;

    return record;
}

void csv_to_bin(FILE *csv, FILE *dest, bool is_fixed) {

    write_header(dest, is_fixed);

    for (int i = 0; i < 7; i++)
        free(fscan_until(csv, ','));

    char is_eof;

    int last_rrn = 0;
    long int next_byteoffset = 0;

    while (!feof(csv)) {
        if ((is_eof = getc(csv)) == EOF)
            return;
        ungetc(is_eof, csv);

        data record = scan_record_csv(csv, is_fixed, &last_rrn, &next_byteoffset);

        write_record(dest, record, is_fixed);
        free_record(record);
    }

    fseek(dest, is_fixed ? 177 : 178, SEEK_SET);

    if (is_fixed)
        fwrite(&last_rrn, 4, 1, dest);
    else
        fwrite(&next_byteoffset, 8, 1, dest);

    fseek(dest, 0, SEEK_SET);
    fwrite("1", 1, 1, dest);
}