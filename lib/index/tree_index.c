#include <stdlib.h>
#include <stdio.h>

#include "../record.h"
#include "tree_index.h"


tree_node read_node(FILE *stream, bool is_fixed) {
    /*
     * Reads a node from a given B-Tree file stream.
     * Must be previously placed at seek position beforehand.
     * Returns struct with tree_node type, either with rrn keys or byteoffset keys filled,
     * depending on the chosen encoding.
     */
    tree_node node_read = {};

    // Read common fields, those being the node type, as well as how many keys the node has.
    fread(&node_read.type, sizeof(char), 1, stream);
    fread(&node_read.num_keys, sizeof(int), 1, stream);

    int i = 0;

    // Read key fields (id + rrn or byteoffset) into the keys array.
    // Contains the identifier, as well as the identifier locator within
    // the original data file.
    if (is_fixed) {
        for (; i < 3; i++) {
            fread(&node_read.keys[i].id, sizeof(int), 1, stream);
            fread(&node_read.keys[i].rrn, sizeof(int), 1, stream);

        }
    } else {
        for (; i < 3; i++) {
            fread(&node_read.keys[i].id, sizeof(int), 1, stream);
            fread(&node_read.keys[i].byteoffset, sizeof(long int), 1, stream);
        }
    }

    // Finally, read rrn for the node's child nodes, being either a valid integer or -1 if the node has
    // no child at specific key position.
    for (i = 0; i < 4; i++) {
        fread(&node_read.children[i], sizeof(int), 1, stream);
    }

    return node_read;
}


int tree_search_identifier(FILE *stream, key identifier, int *rrn_found, int *pos_found, bool is_fixed) {
    /*
     * Searches a B-Tree recursively up until a leaf node is found, in which case the function
     * returns a NOT_FOUND warning. If a node is otherwise found, the rrn_found and pos_found values
     * allow the parent function to find the node position in the tree, as well as its position
     * locator inside the original data file.
     */
    // If rrn is -1, meaning a leaf node has been reached, return a register NOT_FOUND warning.
    if (*rrn_found == -1)
        return NOT_FOUND;

    // Calculate the current node byteoffset based on the current searched node
    long int byteoffset = (*rrn_found + 1) * (is_fixed ? 45 : 57);

    // Seek to calculated byteoffset, and read node at the specific position to search identifier at
    fseek(stream, byteoffset, SEEK_SET);
    tree_node current = read_node(stream, is_fixed);

    // Search all key pairs within the node, based on the num_keys field also inside the node
    for (*pos_found = 0; *pos_found < current.num_keys; (*pos_found)++) {

        // If any identifiers matches with the desired one, return a SUCCES_CODE
        if (identifier.id == current.keys[*pos_found].id)
            return SUCCESS_CODE;

        // Otherwise, keep iterating until not at desired sorted key position
        if (identifier.id < current.keys[*pos_found].id)
            break;
    }

    // Set rrn_found value to the next position to search for, and call the
    // search function recursively
    *rrn_found = current.children[*pos_found];
    return tree_search_identifier(stream, identifier, rrn_found, pos_found, is_fixed);
}


void insert_into_tree(FILE *stream, data record, bool is_fixed) {


}


void create_tree_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {


}
