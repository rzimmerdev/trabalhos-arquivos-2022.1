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


void write_fixed(FILE *dest, data record) {

    char code_city = '0', code_brand = '1', code_model = '2';

    fwrite(record.removed, 1, 1, dest);
    fwrite(&record.next, 4, 1, dest);
    fwrite(&record.id, 4, 1, dest);
    fwrite(&record.year, 4, 1, dest);
    fwrite(&record.amt, 4, 1, dest);

    if (strlen(record.state)) {
        fwrite(record.state, 1, 2, dest);
    }
    else {
        fwrite("$$", 1, 2, dest);
    }

    if (strlen(record.city)) {
        fwrite(&record.city_size, 4, 1, dest);
        fwrite(&code_city, 1, 1, dest);
        fwrite(record.city, 1, record.city_size, dest);
    }

    if (strlen(record.brand)) {
        fwrite(&record.brand_size, 4, 1, dest);
        fwrite(&code_brand, 1, 1, dest);
        fwrite(record.brand, 1, record.brand_size, dest);
    }

    if (strlen(record.model)) {
        fwrite(&record.model_size, 4, 1, dest);
        fwrite(&code_model, 1, 1, dest);
        fwrite(record.model, 1, record.model_size, dest);
    }

    for (int i = 19 + strlen(record.city) + strlen(record.brand) + strlen(record.model); i < 97; i++) {
        fwrite("$", 1, 1, dest);
    }
}


void csv_to_bin(FILE *csv, FILE *dest, bool is_fixed) {

    fwrite("0", 1, 1, dest);

    if (is_fixed) {
        int top = 0;
        fwrite(&top, 4, 1, dest);
    } else {
        long int top = 0;
        fwrite(&top, 8, 1, dest);
    }

    char *header_description[11] = {"LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
                     "CODIGO IDENTIFICADOR: ", "ANO DE FABRICACAO: ", "QUANTIDADE DE VEICULOS: ",
                     "ESTADO: ", "0", "NOME DA CIDADE: ", "1", "MARCA DO VEICULO: ",
                     "2", "MODELO DO VEICULO: "};
    for (int i = 0; i < 11; i++) {
        fwrite(header_description[i], 1, strlen(header_description[i]), dest);
    }

    if (is_fixed) {
        int next_rrn = 0;
        fwrite(&next_rrn, 4, 1, dest);
    } else {
        long int byte_offset = 0;
        fwrite(&byte_offset, 8, 1, dest);
    }

    int removed_records = 0;
    fwrite(&removed_records, 4, 1, dest);

    for (int i = 0; i < 7; i++)
        free(fscan_until(csv, ','));

    char is_eof;
    while (!feof(csv)) {
        if ((is_eof = getc(csv)) == EOF)
            continue;
        ungetc(is_eof, csv);

        data record;

        fscanf(csv, "%d,", &record.id);

        char *year = fscan_until(csv, ',');
        if (strlen(year)) {
            record.year = atoi(year);
        } else {
            record.year = -1;
        }
        free(year);

        record.city = fscan_until(csv, ',');
        record.city_size = strlen(record.city);

        char *amt = fscan_until(csv, ',');
        if (strlen(amt)) {
            record.amt = atoi(amt);
        } else {
            record.amt = -1;
        }
        free(amt);

        char *state = fscan_until(csv, ',');
        if (strlen(state)) {
            record.state[0] = state[0]; record.state[1] = state[1];
        } else {
            record.state[0] = '$'; record.state[1] = '$';
        }
        free(state);

        record.brand = fscan_until(csv, ',');
        record.brand_size = strlen(record.brand);
        record.model = fscan_until(csv, ',');
        record.model_size = strlen(record.model);
        write_fixed(dest, record);
        printf_record(record);
    }
}


void printf_record(data record) {

    printf("MARCA DO VEICULO: %s\n", record.brand);
    printf("MODELO DO VEICULO: %s\n", record.model);
    printf("ANO DE FABRICACAO: %d\n", record.year);
    printf("NOME DA CIDADE: %s\n", record.city);
    printf("QAUNTIDADE DE VEICULOS: %d\n", record.amt);
    printf("\n");
}