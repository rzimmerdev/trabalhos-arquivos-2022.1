#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include "record.h"
#include "table.h"
#include "index.h"


int comp (const void * elem1, const void * elem2)
{
    long int *f = (long int *) elem1;
    long int *s = (long int *) elem2;
    if (f[0] > s[0]) return  1;
    if (f[0] < s[0]) return -1;
    return 0;
}

int compare_indexes(index_node first, index_node second) {
    if (first.id > second.id)
        return 1;
    return -(first.id < second.id);
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

void insert_index_node(index_node *array, int size, index_node *node_to_add, bool is_fixed) {
    printf("printando os nos do reg. de indice:\n");
    for (int i = 0; i < size; i++) {
        printf("id: %d\n", array[i].id);
        printf("byteoff: %ld\n", array[i].byteoffset);
    }


    // Alocar espaco para novo elemento no indice apos a insercao de reg. no arq. de dados
        array = (index_node *) realloc(array, (size + 1) * sizeof(index_node));

    // Inserir ordenado no indice array
    int insertion_position = 0;

    // Achar posicao correta
    printf("size: %d\n", size);
    printf("id do novo node: %d\n", node_to_add->id);
    printf("array[insert-pos].id:%d\n", array[insertion_position].id);
    while (insertion_position < size && node_to_add->id > array[insertion_position].id) {
        insertion_position++;
        printf("insertion pos:%d\n", insertion_position);
    }

    // "shiftada" para a direita para abrir espaco para novo no de indice
    // for (int i = size - 1; i >= insertion_position; i--) {
        // printf("array[i+1].id=%d\n", array[i + 1].id);
        // printf("array[i].id=%d\n", array[i].id);
        // array[i] = array[i - 1];
        // break;
    // }

    // array[insertion_position].id = node_to_add->id;

    // if (is_fixed) {
        // array[insertion_position].rrn = node_to_add->rrn;
        // printf("teste3\n");
    // }

    // else {
        // array[insertion_position].byteoffset = node_to_add->byteoffset;
    // }
}

void free_index_array(index_node *array) {
    free(array);
}

index_node *index_to_array(FILE *stream, int size, bool is_fixed) {

    index_node *array = malloc(sizeof(index_node) * size);

    if (is_fixed) {
        for (int i = 0; i < size; i++) {
            fread(&array[i].id, sizeof(int), 1, stream);
            fread(&array[i].rrn, sizeof(int), 1, stream);
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            fread(&array[i].id, sizeof(int), 1, stream);
            fread(&array[i].byteoffset, sizeof(long int), 1, stream);
        }
    }

    return array;
}


void array_to_index(FILE *stream, index_node *array, int size, bool is_fixed) {
    update_status(stream, BAD_STATUS);
    if (is_fixed) {
        for (int i = 0; i < size; i++) {
            fwrite(&array[i].id, sizeof(int), 1, stream);
            fwrite(&array[i].rrn, sizeof(int), 1, stream);
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            fwrite(&array[i].id, sizeof(int), 1, stream);
            fwrite(&array[i].byteoffset, sizeof(long int), 1, stream);
        }
    }
}


int binary_search(index_node *array, index_node lookup, int offset, int top) {
    int middle = (offset + top) / 2;

    if (offset > top)
        return -1;

    if (compare_indexes(array[middle], lookup) == 0)
        return middle;
    else if (compare_indexes(array[middle], lookup) == -1)
        return binary_search(array, lookup, middle + 1, top);
    else
        return binary_search(array, lookup, offset, middle - 1);
}


index_node find_by_id(char *index_filename, int id, bool is_fixed) {
    FILE *stream = fopen(index_filename, "rb+");

    fseek(stream, 0, SEEK_END);
    int size = (int) (ftell(stream) - sizeof(char)) / (is_fixed ? 8 : 12);
    fseek(stream, 1, SEEK_SET);

    index_node *array = index_to_array(stream, size, is_fixed);
    index_node to_find = {.id = id};
    int idx = binary_search(array, to_find, 0, size - 1);

    if (idx == -1) {
        free_index_array(array);

        index_node empty = {.id = id, .rrn = -1, .byteoffset = -1};
        return empty;
    }

    index_node to_return = array[idx];
    free_index_array(array);
    fclose(stream);
    return to_return;
}


index_node remove_index(char *index_filename, int id, bool is_fixed) {
    FILE *stream = fopen(index_filename, "rb");

    fseek(stream, 0, SEEK_END);
    int size = (ftell(stream) - sizeof(char)) / (is_fixed ? 8 : 12);
    fseek(stream, 1, SEEK_SET);
    index_node *array = index_to_array(stream, size, is_fixed);
    index_node to_find = {.id = id};
    int idx = binary_search(array, to_find, 0, size - 1);
    index_node to_return = array[idx];
    memmove(&(array[idx]), &(array[idx + 1]), (--size - idx) * sizeof(index_node));
    fclose(stream);

    stream = fopen(index_filename, "wb");

    array_to_index(stream, array, size, is_fixed);
    free_index_array(array);

    update_status(stream, OK_STATUS);
    fclose(stream);
    return to_return;
}