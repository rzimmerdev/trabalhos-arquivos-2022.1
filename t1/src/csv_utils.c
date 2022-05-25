#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../lib/record.h"

// Informacao sobre o campo 'removido'
#define IS_REMOVED '1'
#define IS_NOT_REMOVED '0'

// Informacao para os campos do reg.
#define INITIAL_VALUE -1
#define GARBAGE '$'

data scan_record_csv(FILE *fp, bool is_fixed, int *last_rrn, long int *next_byteoffset) {
    // Criando um registro
    data record = {};

    record.removed = IS_NOT_REMOVED; // Reg. nao esta removido inicialmente

    if (is_fixed)
        record.next = INITIAL_VALUE; // Nao ha reg. removidos ainda
    else {
        record.big_next = INITIAL_VALUE;
    }
    fscanf(fp, "%d,", &record.id);

    // Tentar recuperar informacao do ano (se houver no csv, pro reg. atual)
    char *year = fscan_until(fp, ',');
    if (strlen(year)) {
        record.year = atoi(year);
    } else {
        record.year = INITIAL_VALUE; // Se nao existe essa info
    }
    free(year);

    record.city = fscan_until(fp, ',');
    record.city_size = strlen(record.city);

    char *total = fscan_until(fp, ',');
    if (strlen(total)) {
        record.total = atoi(total);
    } else {
        record.total = INITIAL_VALUE;
    }
    free(total);

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

    if (!is_fixed) { // Manipular campo referente ao tamanho do reg.
        // Tamanho minimo do reg. (desconsiderando campos de tam. variavel)
        // e contando somente a partir do campo do prox. reg. rem.
        record.size = 22;

        // Contando campos de tam. variavel:
        // + 5 para somar codigo (1 byte) e tamanho (4 bytes)
        if (record.city_size) record.size += 5 + record.city_size;
        if (record.brand_size) record.size += 5 + record.brand_size;
        if (record.model_size) record.size += 5 + record.model_size;
        *next_byteoffset += record.size + 5;
    } else
        *last_rrn += 1;

    return record;
}

void csv_to_bin(FILE *csv, FILE *dest, bool is_fixed) {

    write_header(dest, is_fixed);

    // Descartar primeira linha do CSV (somente identifica colunas para 
    // sabermos a ordem de escrever no arquivo bin.)
    for (int i = 0; i < 7; i++)
        free(fscan_until(csv, ','));

    char is_eof;

    // Para encontrar prox. byte disponivel para novo reg.
    int last_rrn = 0;
    long int next_byteoffset = VARIABLE_HEADER;

    // Escrevendo todos os reg. no binario
    while ((is_eof = getc(csv)) != EOF) {
        ungetc(is_eof, csv);

        // Recuperar dados do csv e trazer para RAM
        data record = scan_record_csv(csv, is_fixed, &last_rrn, &next_byteoffset);

        // Escrever da RAM para o arquivo binario
        write_record(dest, record, is_fixed);
        free_record(record);
    }

    // Checar esse byte - na representacao grafica esta como o byte 174 no fixo, 
    // nao 177
    fseek(dest, is_fixed ? 174 : 178, SEEK_SET);

    if (is_fixed)
        fwrite(&last_rrn, 4, 1, dest);
    else
        fwrite(&next_byteoffset, 8, 1, dest);

    // Alterando status para 1 - arquivo agora consistente
    fseek(dest, 0, SEEK_SET);
    fwrite("1", 1, 1, dest);
}