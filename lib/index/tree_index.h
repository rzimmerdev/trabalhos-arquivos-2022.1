//
// Created by rzimmerdev on 11/07/2022.
//

#ifndef TREE_INDEX_H
#define TREE_INDEX_H

typedef struct TreeHeader_t {

    char status;
    int root_rrn;
    int next_rrn;
    int total_nodes;

    int null_size;

} tree_header;

typedef struct Key_t {
    int id;
    int rrn;
    long int byteoffset;
} key;

typedef struct TreeNode_t {

    char type;
    int num_keys;

    key keys[3];

    int children[4];
} tree_node;

int tree_find_by_id(FILE *stream, int id, bool is_fixed);

#endif //TREE_INDEX_H
