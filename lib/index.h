#include <stdio.h>
#include <stdbool.h>

#ifndef INDEX_H
#define INDEX_H


typedef struct Index_t {

    int id;
    int rrn;
    long int byteoffset;

} index_node;

void create_index(FILE *origin_stream, FILE *index_stream, bool is_fixed);

void free_index(index_node *array);

index_node *index_to_array(FILE *stream, int size, bool is_fixed);
void array_to_index(FILE *stream, index_node *array, int size, bool is_fixed);

index_node find_by_id(char *index_filename, int id, bool is_fixed);
index_node remove_index(char *index_filename, int id, bool is_fixed);

#endif //INDEX_H
