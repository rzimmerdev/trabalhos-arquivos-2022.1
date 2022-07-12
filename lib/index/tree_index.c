#include <stdlib.h>
#include <stdio.h>

#include "../record.h"
#include "tree_index.h"


tree_node read_node(FILE *stream, bool is_fixed) {

    tree_node node_read = {};

    fread(&node_read.type, sizeof(char), 1, stream);
    fread(&node_read.num_keys, sizeof(int), 1, stream);

    int i = 0;
    if (is_fixed)
        for (; i < 3; i++) {

            fread(&node_read.keys[i], sizeof(int), 1, stream);
            fread(&node_read.rrns[i], sizeof(int), 1, stream);

        }
    else
        for (; i < 3; i++) {
            fread(&node_read.keys[i], sizeof(int), 1, stream);
            fread(&node_read.byteoffsets[i], sizeof(long int), 1, stream);
        }

    for (i = 0; i < 4; i++) {
        fread(&node_read.children[i], sizeof(int), 1, stream);
    }

    return node_read;
}


int tree_find_by_id(FILE *stream, int id, bool is_fixed) {
    fseek(stream, sizeof(char), SEEK_SET);

    int current_rrn;
    fread(&current_rrn, sizeof(int), 1, stream);
    tree_node current = {.type = '\0'};

    while (current_rrn != -1) {
        fseek(stream, current_rrn, SEEK_SET);
        current = read_node(stream, is_fixed);

        for (int i = 0; i < current.num_keys; i++) {
            if (id < current.keys[i]) {
                current_rrn = current.children[i];
                break;
            }
        }
    }

    return current_rrn;
}


void insert_into_tree(tree_index index, data record) {

}


void create_tree_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {


}
