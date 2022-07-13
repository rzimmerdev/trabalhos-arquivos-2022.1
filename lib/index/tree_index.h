#ifndef TREE_INDEX_H
#define TREE_INDEX_H

// Valores de retorno para insercao
#define PROMOTION 1 // Quando insercao eh feita e uma chave eh promovida (no cheio/overflow)
#define NO_PROMOTION 0 // Quando insercao eh feita e nenhuma chave eh promovida (no com espaco livre)
#define INSERT_ERROR -2 // Quando uma chave sendo inserida ja existe na btree (indice de chave primaria)

#define NODE_NOT_FOUND -1 // Para identificar no folha na insercao

// Tamanhos dos registros do indice/arvore
#define BT_FIXED_SIZE 45 // para arquivo de tipo 1
#define BT_VARIABLE_SIZE 57 // para arquivo de tipo 2

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