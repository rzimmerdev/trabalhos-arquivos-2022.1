#include <stdio.h>
#include <stdbool.h>

#ifndef INDEX_H
#define INDEX_H


typedef struct Index_t {

    int id;
    int rrn;
    long int byteoffset;

} index_node;


typedef struct IndexArray_t {

    int size;
    char *filename;
    index_node *array;

} index_array;


void create_index(FILE *origin_stream, FILE *index_stream, bool is_fixed);

void insert_index_node(index_node *array, int size, index_node *node_to_add, bool is_fixed);

void free_index_array(index_array *index);

index_array index_to_array(char *filename, bool is_fixed);
void array_to_index(index_array index, bool is_fixed);

index_node find_by_id(index_array index, int id);

void remove_from_index_array(index_array *index, int id);
void insert_into_index_array(index_array *index, index_node to_insert);

int binary_search(index_array index, index_node lookup, int offset, int top);

#endif //INDEX_H
