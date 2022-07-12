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
        for (; i < 4; i++) {

            fread(&node_read.keys[i], sizeof(int), 1, stream);
            fread(&node_read.rrns[i], sizeof(int), 1, stream);

        }
    else
        for (; i < 4; i++) {
            fread(&node_read.keys[i], sizeof(int), 1, stream);
            fread(&node_read.byteoffsets[i], sizeof(long int), 1, stream);
        }

    for (i = 0; i < 4; i++) {
        fread(&node_read.children[i], sizeof(int), 1, stream);
    }

    return node_read;
}


data tree_find_by_id(tree_index index, int id) {

}


void insert_into_tree(tree_index index, data record) {

}


void create_tree_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {


}
