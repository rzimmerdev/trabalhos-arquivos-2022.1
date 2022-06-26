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
    long int current_byteoffset = VARIABLE_HEADER;

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
        current_byteoffset += to_indexate.size + 5;
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


void free_index_array(index_array *index) {
    free(index->array);
}


index_array index_to_array(char *filename, bool is_fixed) {
    FILE *stream = fopen(filename, "rb");
    fseek(stream, 0, SEEK_END);
    int size = (ftell(stream) - sizeof(char)) / (is_fixed ? 8 : 12);
    fseek(stream, 1, SEEK_SET);

    index_node *array = malloc(sizeof(index_node) * size);
    index_array index = {.array = array, .size = size, .filename = filename};

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

    fclose(stream);

    return index;
}


void array_to_index(index_array index, bool is_fixed) {
    FILE *stream = fopen(index.filename, "wb");

    index_node *array = index.array;

    update_status(stream, BAD_STATUS);
    if (is_fixed) {
        for (int i = 0; i < index.size; i++) {
            fwrite(&array[i].id, sizeof(int), 1, stream);
            fwrite(&array[i].rrn, sizeof(int), 1, stream);
        }
    }
    else {
        for (int i = 0; i < index.size; i++) {
            fwrite(&array[i].id, sizeof(int), 1, stream);
            fwrite(&array[i].byteoffset, sizeof(long int), 1, stream);
        }
    }
    update_status(stream, OK_STATUS);
    fclose(stream);
}


int binary_search(index_array index, index_node lookup, int offset, int top) {
    int middle = (offset + top) / 2;

    if (offset > top)
        return middle;

    if (compare_indexes(index.array[middle], lookup) == 0)
        return middle;
    else if (compare_indexes(index.array[middle], lookup) == -1)
        return binary_search(index, lookup, middle + 1, top);
    else
        return binary_search(index, lookup, offset, middle - 1);
}


index_node find_by_id(index_array index, int id) {
    index_node to_find = {.id = id};
    int idx = binary_search(index, to_find, 0, index.size - 1);

    if (idx == -1) {
        index_node empty = {.id = id, .rrn = -1, .byteoffset = -1};
        return empty;
    }

    index_node to_return = index.array[idx];
    return to_return;
}


void remove_from_index_array(index_array *index, int id) {
    index_node to_remove = {.id = id};
    int idx = binary_search(*index, to_remove, 0, index->size - 1);

    index->size--;
    memmove(&(index->array[idx]), &(index->array[idx + 1]), (index->size - idx) * sizeof(index_node));
    index->array = realloc(index->array, sizeof(index_node) * index->size);
}


void insert_into_index_array(index_array *index, index_node to_insert) {
    int idx = binary_search(*index, to_insert, 0, index->size - 1);

    index->size++;
    index->array = realloc(index->array, sizeof(index_node) * index->size);
    memmove(&(index->array[idx + 1]), &(index->array[idx]), (index->size - idx - 1) * sizeof(index_node));

    index->array[idx + 1] = to_insert;
}