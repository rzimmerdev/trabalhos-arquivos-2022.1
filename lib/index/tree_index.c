#include <stdlib.h>
#include <stdio.h>

#include "../record.h"
#include "../table.h"
#include "tree_index.h"


tree_header read_header(FILE *stream, bool is_fixed) {

    tree_header header = {};
    fseek(stream, 0, SEEK_SET);

    fread(&header.status, sizeof(char), 1, stream);
    fread(&header.root_rrn, sizeof(int), 1, stream);
    fread(&header.next_rrn, sizeof(int), 1, stream);
    fread(&header.total_nodes, sizeof(int), 1, stream);

    fseek(stream, is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE, SEEK_SET);

    return header;
}


void write_tree_header(FILE *stream, tree_header header, bool is_repeat, bool is_fixed) {

    fseek(stream, 0, SEEK_SET);

    fwrite(&header.status, sizeof(char), 1, stream);
    fwrite(&header.root_rrn, sizeof(int), 1, stream);
    fwrite(&header.next_rrn, sizeof(int), 1, stream);
    fwrite(&header.total_nodes, sizeof(int), 1, stream);

    if (!is_repeat) {
        fseek(stream, is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE, SEEK_SET);
        return;
    }

    for (int i = 0; i < (is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE); i++) {
        fwrite(&GARBAGE, sizeof(char), 1, stream);
    }
}


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


long int tree_search_identifier(FILE *stream, key identifier, int *rrn_found, int *pos_found, bool is_fixed) {
    /*
     * Searches a B-Tree recursively up until a leaf node is found, in which case the function
     * returns a NOT_FOUND warning. If a node is otherwise found, the rrn_found and pos_found values
     * allow the parent function to find the node position in the tree, as well as its position
     * locator inside the original data file.
     */
    // If rrn is -1, meaning a leaf node has been reached, return a register NOT_FOUND warning.
    if (*rrn_found == -1)
        return (long int) NOT_FOUND;

    // Calculate the current node byteoffset based on the current searched node
    long int byteoffset = (*rrn_found + 1) * (is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE);

    // Seek to calculated byteoffset, and read node at the specific position to search identifier at
    fseek(stream, byteoffset, SEEK_SET);
    tree_node current = read_node(stream, is_fixed);

    // Search all key pairs within the node, based on the num_keys field also inside the node
    for (*pos_found = 0; *pos_found < current.num_keys; (*pos_found)++) {

        // If any identifiers matches with the desired one, return a SUCCES_CODE
        if (identifier.id == current.keys[*pos_found].id)
            return is_fixed ? (current.keys[*pos_found].rrn * FIXED_REG_SIZE + FIXED_HEADER) : current.keys[*pos_found].byteoffset;
        // Otherwise, keep iterating until not at desired sorted key position
        if (identifier.id < current.keys[*pos_found].id)
            break;
    }

    // Set rrn_found value to the next position to search for, and call the
    // search function recursively
    *rrn_found = current.children[*pos_found];
    return tree_search_identifier(stream, identifier, rrn_found, pos_found, is_fixed);
}

/*
 * Inicia-se com uma pesquisa que desce ate o nivel dos nos folhas. Uma vez
 * escolhido o no folha no qual a nova chave deve ser inserida, os processos de 
 * insercao, particionamento/split e promocao propagam-se em direcao a raiz
 * (construcao bottom-up).
 *
 * A insercao conta com um procedimento recursivo. As fases da funcionalidade sao:
 * - busca pela pagina (pesquisa da pagina antes da chamada recursiva);
 * - chamada recursiva (move a operacao para niveis inferiores da arvore)
 * - insercao, split e promotion - executados apos chamada recursiva. A propagacao
 * destes ocorre no retorno da recursao.
 * 
 * Parametros:
 * - int curr_rrn -> rrn da pagina da arvore B atualmente em uso (no inicio, a raiz). Eh a pagina
 * a ser pesquisada
 * - key to_insert -> chave a ser inserida
 * - key *promoted -> referencia da chave promovida, caso a insercao resulte no
 * particionamento e na promocao da chave
 * - int *promoted_right_child -> referencia para o filho direito da chave promovida (promoted).
 * Eh um RRN, usado para quando ocorrer particionamento, pois nao somente a chave promovida deve
 * ser inserida em um no de nivel mais alto da arvore, como tambem deve-se inserir o RRN da nova
 * pagina criada no particionamento.
 * 
 */
int insert_into_tree(int curr_rrn, key to_insert, key *promoted, int *promoted_right_child) {
    // Eh aqui que se deve inserir a chave (no-folha - construcao bottom up)
    if (!curr_rrn) {
        *promoted = to_insert; // Para subir um nivel na recursao e inserir
        promoted_right_child = NULL; // Porque eh no-folha

        return PROMOTION;
    }

    // Se a pagina nao eh no-folha, chamar a funcao recursivamente ate que ela encontre
    // uma chave com o valor que queremos inserir (ja existente na tree) ou chegue ao no
    // folha para que a insercao do novo no seja feita
    else {
        
    }
}


void create_tree_index(FILE *stream, char *index_filename, header *data_header, bool is_fixed) {
    int current_rrn = 0;

    FILE *index_stream = fopen(index_filename, "wb+");
    tree_header header = {.status = 0, .root_rrn = 0, .total_nodes = 0, .next_rrn = 0};
    write_tree_header(stream, header, false, is_fixed);

    int total_nodes = 0;
    while (current_rrn < (data_header->next_rrn - 1) || (ftell(stream) < data_header->next_byteoffset - 1)) {

        data current_record = fread_record(stream, is_fixed);

        key to_insert = {.id = current_record.id, .rrn = current_rrn, .byteoffset = ftell(stream)};
        // TODO: Use to_insert as a key to insert within the index file
        total_nodes++;
    }
    header.status = 1;
    header.total_nodes = total_nodes;
    header.next_rrn = total_nodes * (is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE);
    write_tree_header(stream, header, true, is_fixed);
    fclose(index_stream);
}
