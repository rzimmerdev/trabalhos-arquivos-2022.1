#include <string.h>

#include "record.h"
#include "table.h"

void write_header(FILE *stream, bool is_fixed) {
    // Ao abrir para escrita, status deve ser 0 (arq. inconsistente/desatualizado)
    fwrite(BAD_STATUS, 1, 1, stream);

    if (is_fixed) {
        int top = EMPTY; // Nao ha registros removidos ainda
        fwrite(&top, 4, 1, stream);
    } else {
        long int top = EMPTY;
        fwrite(&top, 8, 1, stream);
    }

    char *header_description[11] = {"LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
                                    "CODIGO IDENTIFICADOR: ", "ANO DE FABRICACAO: ", "QUANTIDADE DE VEICULOS: ",
                                    "ESTADO: ", "0", "NOME DA CIDADE: ", "1", "MARCA DO VEICULO: ",
                                    "2", "MODELO DO VEICULO: "};
    for (int i = 0; i < 11; i++) {
        fwrite(header_description[i], 1, strlen(header_description[i]), stream);
    }

    if (is_fixed) {
        int next_rrn = 0;
        fwrite(&next_rrn, 4, 1, stream);
    } else {
        long int next_byte_offset = 0;
        fwrite(&next_byte_offset, 8, 1, stream);
    }

    // Quantidade de registros logicamente marcados como removidos
    int removed_records = 0;
    fwrite(&removed_records, 4, 1, stream);
}

int select_table(FILE *stream, bool is_fixed) {

    if (getc(stream) == '0') {
        return ERROR;
    }

    fseek(stream, is_fixed ? NEXT_RRN_b : NEXT_BYTEOFFSET_b, SEEK_SET);

    int next_rrn;
    long int next_byteoffset;

    if (is_fixed)
        fread(&next_rrn, 4, 1, stream);
    else
        fread(&next_byteoffset, 8, 1, stream);

    fseek(stream, is_fixed ? FIXED_HEADER : VARIABLE_HEADER, SEEK_SET);
    while ((is_fixed && (next_rrn-- > 0)) || (!is_fixed && ftell(stream) < next_byteoffset)) {

        data scanned = fread_record(stream, is_fixed);

        if (scanned.removed == IS_REMOVED)
            printf("Registro inexistente.\n");
        else
            printf_record(scanned);

        free_record(scanned);
    }
    return SUCCESS;
}


int select_where(FILE *stream, data template, header header_template, bool is_fixed) {

    int total_found = 0;
    while ((is_fixed && (header_template.next_rrn-- > 0)) || (!is_fixed && ftell(stream) < header_template.next_byteoffset)) {

        data record = fread_record(stream, is_fixed);

        if ((template.year != -1 && (record.year == -1 || template.year != record.year)) ||
            (template.id != -1 && (record.id == -1 || template.id != record.id)) ||
            (template.total != -1 && (record.total == -1 || template.total != record.total))) {

            free_record(record);
            continue;
        }

        if ((template.city && !record.city) || (template.city && record.city && strcmp(template.city, record.city)) ||
            (template.brand && !record.brand) || (template.brand && record.brand && strcmp(template.brand, record.brand)) ||
            (template.model && !record.model) || (template.model && record.model && strcmp(template.model, record.model))) {
            free_record(record);
            continue;
        }

        if (record.removed == '0') {
            printf_record(record);
            total_found++;
        }
        else printf("Registro inexistente.\n");
    }

    return total_found > 0 ? SUCCESS : NOT_FOUND;
}