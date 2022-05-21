#include <stdlib.h>
#include <stdio.h>

#include "commands.h"
#include "record_types.h"

void create_table_command() {
    // Ler a entrada
    char *csv_filename = scan_word();
    char *out_filename = scan_word();

    
    FILE *csvfile_ptr = fopen(csv_filename, "r");
    free(csv_filename);

    FILE *binfile_ptr = fopen(out_filename, "wb");
    free(out_filename);

    if (!(csvfile_ptr && binfile_ptr)) {
        printf("Falha no processamento do arquivo.\n");
    }

    csv_to_record_fixed(csvfile_ptr, binfile_ptr);

    fclose(csvfile_ptr);
    fclose(binfile_ptr);
}

void select_command() {
    // Ler a entrada
    char *bin_filename = scan_word();

    FILE *file_ptr = NULL;
    file_ptr = fopen(bin_filename, "rb");

    if (!file_ptr) {
        printf("macaco");
    }
    

    if (bin_filename) {
        free(bin_filename);
    }
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