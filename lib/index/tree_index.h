//
// Created by rzimmerdev on 11/07/2022.
//

#ifndef FAST_INDEX_H
#define FAST_INDEX_H

typedef struct TreeHeader_t {

    char status;
    int root_rrn;
    int next_rrn;
    int total_nodes;

    int null_size;

} tree_header;


typedef struct TreeNode_t {

    char type;
    int num_keys;

    int keys[4];

    int rrns[4];
    long int byteoffsets[4];

    int children[4];
} tree_node;

int tree_find_by_id(FILE *stream, int id, bool is_fixed);

#endif //FAST_INDEX_H
