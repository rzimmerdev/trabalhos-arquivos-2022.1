#include <stdlib.h>
#include <stdio.h>

#include "commands.h"

void create_table_command() {
    // Ler a entrada
    char *csv_filename = scan_word();
    char *out_filename = scan_word();

    free(csv_filename);
    free(out_filename);
}

void select_command() {
    // Ler a entrada
    char *bin_filename = scan_word();

    free(bin_filename);
}

void select_where_command() {
    // Ler a entrada
    char *bin_filename = scan_word();
    int field_amt;
    scanf("%d", &field_amt);

    free(bin_filename);
}

void select_id_command() {
    // Ler a entrada
    char *bin_filename = scan_word();
    int RRN;
    scanf("%d", &RRN);

    free(bin_filename);
}