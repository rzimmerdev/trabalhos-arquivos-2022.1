#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include "record.h"
#include "table.h"
#include "index.h"


int compare_long_int (const void * elem1, const void * elem2) {
    /*
     * Compares two void pointers of type (long int, long int),
     * to be passed tostandard library qsort function.
     */
    long int *f = (long int *) elem1;
    long int *s = (long int *) elem2;
    // After casting is performed, compare first element corresponding to ID field
    if (f[0] > s[0]) return  1;
    if (f[0] < s[0]) return -1;
    return 0;
}


int compare_indexes(index_node first, index_node second) {
    /*
     * Compares two struct variables of type index_node,
     * to be used when sorting or traversing array of index_node type.
     */
    if (first.id > second.id)
        return 1;
    return -(first.id < second.id);
}


void create_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {
    /*
     * Create an index file based on an origin stream of records, with given is_fixed file encoding.
     * Resulting index file is saved to stream of type index_stream.
     */

    // Read header from within origin_stream to iterate
    // over available records according to total records available
    header table_header = fread_header(origin_stream, is_fixed);

    int current_rrn = 0;
    long int current_byteoffset = VARIABLE_HEADER;

    // Create buffer of indices to read and write to desired file.
    int buffer = BUFSIZ, total_indexes = 0;
    long int (*indexes)[2] = malloc(buffer * sizeof(long int [2]));

    // Iterate over origin stream while not at pre-calculated end position, which
    // depends on the selected encoding type
    while ((is_fixed && (current_rrn < table_header.next_rrn)) || (!is_fixed && current_byteoffset < table_header.next_byteoffset)) {

        // Reads an individual record from stream, and skips if it is
        // logically marked as removed
        data to_indexate = fread_record(origin_stream, is_fixed);
        if (to_indexate.removed == IS_REMOVED) {
            current_rrn += 1;
            current_byteoffset = ftell(origin_stream);

            free_record(to_indexate);
            continue;
        }

        // Otherwise, decide if array buffer needs to be incrementated and
        // reallocate memory if needed
        if (buffer <= total_indexes) {
            buffer += BUFSIZ;
            indexes = realloc(indexes, sizeof(long int [2]) * buffer);
        }

        // Accomodate read index variable to available position within the array
        // and decide which field to store (if record is fixed, store rrn, else store its byteoffset)
        indexes[total_indexes][0] = (long int) to_indexate.id;
        indexes[total_indexes][1] = (long int) is_fixed ? current_rrn : current_byteoffset;
        total_indexes++;

        current_rrn += 1;
        current_byteoffset += to_indexate.size + 5;
        free_record(to_indexate);
    }

    // Sort indices in RAM memory, as time efficiency is greatly optimized when
    // compared to sorting in secondary memory
    qsort(indexes, total_indexes, sizeof(long int [2]), compare_long_int);

    // After sorting is complete, iterate over read indices
    // and write to desired output file in order
    for (int i = 0; i < total_indexes; i++) {

        int rrn = (int) indexes[i][1];
        long int byteoffset = indexes[i][1];

        fwrite((int *) indexes[i], sizeof(int), 1, index_stream);

        // Based on input file encoding, write either the index rrn position
        // or its byteoffset on the origin stream file
        if (is_fixed)
            fwrite(&rrn, sizeof(int), 1, index_stream);
        else
            fwrite(&byteoffset, sizeof(long int), 1, index_stream);
    }

    // Free auxiliary index array
    free(indexes);
}


void free_index_array(index_array *index) {
    // Frees all variable sized fields within a variable of type index_array
    free(index->array);
}


index_array index_to_array(char *filename, bool is_fixed) {
    /*
     * Transforms an index binary file into a variable of type index_array,
     * containing all read pairs of indices, writting them in the read order to the struct's array field.
     */

    // Opens stream as a read operation to access all indices without changes
    FILE *stream = fopen(filename, "rb");

    // Calculate size of the final to be generated index array, based on file size and file encoding
    fseek(stream, 0, SEEK_END);
    int size = (ftell(stream) - sizeof(char)) / (is_fixed ? 8 : 12);
    fseek(stream, 1, SEEK_SET);

    // Create index structure object based on file encoding type
    // and calculated index size
    index_node *array = malloc(sizeof(index_node) * size);
    index_array index = {.array = array, .size = size, .filename = filename};

    // Read all indices based on size variable and file encoding type
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

    // Close stream and return created index_array variable
    fclose(stream);

    return index;
}


void array_to_index(index_array index, bool is_fixed) {
    /*
     * Converts an array of type index_array containing all index node pairs (id; rrn or byteoffset)
     * into its respective binary file with given is_fixed file encoding.
     */

    // Open original index file as a "write only" stream, as to overwrite any existing ids
    // Update file status as a write operation is being performed
    FILE *stream = fopen(index.filename, "wb");
    update_status(stream, BAD_STATUS);

    index_node *array = index.array;
    // For all nodes within the array, write the pair in order, after the status header
    // and based on the desired file encoding saved inside the index structure
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

    // Update status and close file
    update_status(stream, OK_STATUS);
    fclose(stream);
}


int binary_search(index_array index, index_node lookup, int offset, int top) {
    /*
     * Sample binary search function to find an element of type index_node
     * with desired id within the given index_array list of indexes.
     */

    // Locate centered element (exactly at position max + min / 2)
    int middle = (offset + top) / 2;

    // If there is no centered element (calculated when the minimum position [offset] is bigger than the
    // maximum position [top]) return the last found element which is at ~middle~
    if (offset > top)
        return middle;

    // Else, decide if recursive binary search is to be performed on the left half or the right half, or even
    // if the centered element has the same id as the desired element, in which case to return its position
    if (compare_indexes(index.array[middle], lookup) == 0)
        return middle;
    else if (compare_indexes(index.array[middle], lookup) == -1)
        return binary_search(index, lookup, middle + 1, top);
    else
        return binary_search(index, lookup, offset, middle - 1);
}


// Performs a binary search to find a node within the index array by its id, or if node is not found
// return an empty node.
index_node find_by_id(index_array index, int id) {

    // Performs a binary search to find next ordered position of index inside the array.
    index_node to_find = {.id = id};
    int idx = binary_search(index, to_find, 0, index.size - 1);

    if (idx == -1) {
        index_node empty = {.id = id, .rrn = -1, .byteoffset = -1};
        return empty;
    }

    index_node to_return = index.array[idx];
    return to_return;
}


// Removes an index node (id; rrn or byteoffset pair) from the array of indexes,
// as to perform all index operations on RAM
void remove_from_index_array(index_array *index, int id) {

    index_node to_remove = {.id = id};
    // Finds the position of the index to be removed within the array using a binary search
    // with the given id
    int idx = binary_search(*index, to_remove, 0, index->size - 1);

    index->size--;

    // Contracts array as to overwrite desired record index
    memmove(&(index->array[idx]), &(index->array[idx + 1]), (index->size - idx) * sizeof(index_node));
    index->array = realloc(index->array, sizeof(index_node) * index->size);
}


// Inserts a new index node (id; rrn or byteoffset pair) into the array of indexes,
// as to perform all index operations on RAM
void insert_into_index_array(index_array *index, index_node to_insert) {

    // Performs a binary search to find next ordered position of index inside the array.
    int idx = binary_search(*index, to_insert, 0, index->size - 1);

    index->size++;

    // Expands array to insert element inside.
    index->array = realloc(index->array, sizeof(index_node) * index->size);
    memmove(&(index->array[idx + 1]), &(index->array[idx]), (index->size - idx - 1) * sizeof(index_node));
    // After expansion, insert element into ordered position
    index->array[idx + 1] = to_insert;
}
