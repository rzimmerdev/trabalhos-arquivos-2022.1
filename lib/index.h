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


/*
 * Create an index file based on an origin stream of records, with given is_fixed file encoding.
 * Resulting index file is saved to stream of type index_stream.
 */
void create_index(FILE *origin_stream, FILE *index_stream, bool is_fixed);


// Frees all variable sized fields within a variable of type index_array
void free_index_array(index_array *index);


/*
 * Transforms an index binary file into a variable of type index_array,
 * containing all read pairs of indices, writting them in the read order to the struct's array field.
 */
index_array index_to_array(char *filename, bool is_fixed);


/*
 * Converts an array of type index_array containing all index node pairs (id; rrn or byteoffset)
 * into its respective binary file with given is_fixed file encoding.
 */
void array_to_index(index_array index, bool is_fixed);


/*
 * Performs a binary search to find a node within the index array by its id,
 * or if node is not found return an empty node.
 */
index_node find_by_id(index_array index, int id);


/*
 *  Removes an index node (id; rrn or byteoffset pair) from the array of indexes,
 *  as to perform all index operations on RAM
 */
void remove_from_index_array(index_array *index, int id);


/*
 * Inserts a new index node (id; rrn or byteoffset pair) into the array of indexes,
 * as to perform all index operations on RAM
 */
void insert_into_index_array(index_array *index, index_node to_insert);


#endif //INDEX_H
