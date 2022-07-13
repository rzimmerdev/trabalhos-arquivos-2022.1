#ifndef TREE_INDEX_H
#define TREE_INDEX_H


// Tamanhos dos registros do indice/arvore
#define INDEX_SIZE_FIXED 45 // para arquivo de tipo 1
#define INDEX_SIZE_VARIABLE 57 // para arquivo de tipo 2

// Valores de retorno para insercao
#define PROMOTION 1 // Quando insercao eh feita e uma chave eh promovida (no cheio/overflow)
#define NO_PROMOTION 0 // Quando insercao eh feita e nenhuma chave eh promovida (no com espaco livre)
#define INSERT_ERROR -2 // Quando uma chave sendo inserida ja existe na btree (indice de chave primaria)

#define NODE_NOT_FOUND -1 // Para identificar no folha na insercao

// Qtde maxima de chaves em um no da arvore
#define MAX_KEY_AMT 3

typedef struct TreeHeader_t {

    char status;
    int root_rrn;
    int next_rrn;
    int total_nodes;

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

tree_node read_node(FILE *stream, bool is_fixed);

long int tree_search_identifier(FILE *stream, key identifier, int *rrn_found, int *pos_found, bool is_fixed);

int insert_into_tree(FILE *b_tree, tree_header *header, bool is_fixed, int curr_rrn, key to_insert, key *promoted, int *prom_right_child);

#endif //TREE_INDEX_H
