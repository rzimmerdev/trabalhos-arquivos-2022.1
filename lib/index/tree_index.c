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
        char garbage = GARBAGE;
        
        fwrite(&garbage, sizeof(char), 1, stream);
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

        // If any identifiers matches with the desired one, return a SUCCESS_CODE
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

// Para posicionar o ponteiro do arquivo de indice no no desejado
void seek_node(FILE *b_tree, int rrn, bool is_fixed) {
    int byte_offset = 0;

    if (is_fixed) {
        // Calculate RRN based on the generic formulae below:
        byte_offset = rrn * INDEX_SIZE_FIXED + INDEX_SIZE_FIXED;
    }

    else {
        byte_offset = rrn * INDEX_SIZE_VARIABLE + INDEX_SIZE_VARIABLE;
    }

    // Navigate to byte_offset calculated position
    fseek(b_tree, byte_offset, SEEK_SET);
}

/*
 * Tratamento do overflow causado pela insercao de uma chave.
 * - Cria uma nova pagina;
 * - Distribui as chaves o mais uniformemente possivel entre a pag. ja existente
 * e aquela a ser criada;
 * - Determina quais chaves e RRNs serao promovidos.
 * 
 * Parametros:
 * - key to_insert -> nova chave a ser inserida
 * - int *inserted_r_child -> referencia para filho a direita da nova chave a ser inserida
 * - tree_node *curr_page -> referencia da pagina de disco corrente
 * - key *promoted -> referencia da chave promovida
 * - int *prom_r_child -> filho a direita da chave promovida
 * - tree_node *new_page -> referencia para nova pagina de disco.
 * - tree_node *new_rrn -> referencia para rrn da nova pagina de disco.
 * 
 */
void split(key to_insert, int *inserted_r_child, tree_node *curr_page, key *promoted, int *prom_r_child, tree_node *new_page, int *new_rrn) {
    // Trazer todas as chaves e 'ponteiros'/descendentes da pagina de disco atual para
    // um espaco capaz de segurar uma chave e um filho extra
    key all_keys[4];
    int all_children[5];

    // Essa informacao nao muda
    all_children[0] = curr_page->children[0];

    // Inserir ordenado dentro do espaco
    for (int i = 0; i < 5; i++) {
        if (to_insert.id > curr_page->keys[i].id) {
            all_keys[i] = curr_page->keys[i];
            all_children[i + 1] = curr_page->children[i + 1];
        }

        else {
            all_keys[i] = to_insert;
            all_children[i + 1] = inserted_r_child;
        }
    }

    // O RRN da nova pagina sera o filho direito da chave promovida (pois insercao sempre a direita)
    *new_rrn = *prom_r_child;
    
    // Copiar chaves e 'ponteiros' para filhos que antecedem a chave promovida
    // -> da pagina auxiliar para a pagina atual do indice
    int i = 0;
    while (promoted->id > all_keys[i].id) {
        curr_page->keys[i] = all_keys[i];
        curr_page->children[i + 1] = all_children[i + 1];
        i++;
    }

    // Copiar chaves e 'ponteiros' para filhos que sucedem a chave promovida
    // -> da pagina auxiliar para a nova pagina do indice
    int i = 0;
    while (promoted->id < all_keys[i].id) {
        new_page->keys[i] = all_keys[i];
        new_page->children[i + 1] = all_children[i + 1];
        i++;
    }
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
 * - FILE *b_tree -> ponteiro para arquivo da arvore
 * - bool is_fixed -> determina o tipo de arquivo de dados trabalhado (1 ou 2)
 * - int curr_rrn -> rrn da pagina da arvore B atualmente em uso (no inicio, a raiz). Eh a pagina
 * a ser pesquisada
 * - key to_insert -> chave a ser inserida
 * - key *promoted -> referencia da chave promovida, caso a insercao resulte no
 * particionamento e na promocao da chave
 * - int *prom_right_child -> referencia para o filho direito da chave promovida (promoted).
 * Eh um RRN, usado para quando ocorrer particionamento, pois nao somente a chave promovida deve
 * ser inserida em um no de nivel mais alto da arvore, como tambem deve-se inserir o RRN da nova
 * pagina criada no particionamento.
 * 
 * 
 * Retorno: int -> pode ser PROMOTION, INSERT_ERROR ou NO_PROMOTION.
 */
int insert_into_tree(FILE *b_tree, bool is_fixed, int curr_rrn, key to_insert, key *promoted, int *prom_right_child) {
    // Eh aqui que se deve inserir a chave (no-folha - construcao bottom up)
    if (curr_rrn == NODE_NOT_FOUND) {
        *promoted = to_insert; // Para subir um nivel na recursao e inserir
        prom_right_child = NULL; // Porque eh no-folha

        return PROMOTION;
    }

    /* Se a pagina nao eh no-folha, chamar a funcao recursivamente ate que ela encontre
     * uma chave com o valor que queremos inserir (ja existente na tree) ou chegue ao no
     * folha para que a insercao do novo no seja feita */
    seek_node(b_tree, curr_rrn, is_fixed);
    tree_node curr_page = read_node(b_tree, is_fixed);
    
    // Pesquisar a pagina, procurando a chave de busca
        // Precisa usar uma parte da f. de tree_search_identifier

    // Search all key pairs within the node/page, based on the num_keys field also inside the node
    int position = -1;
    for (int i = 0; i < curr_page.num_keys; i++) {
        position = i; // Onde a chave ocorreu ou deveria estar

        // Chave encontrada - nao inserir duplicata
        if (to_insert.id == curr_page.keys[i].id) {

            return INSERT_ERROR;
        }

        // Onde a chave deveria estar
        if (to_insert.id < curr_page.keys[i].id) {
            break;
        }
    }

    // A chave de busca nao foi encontrada, portanto procure a chave de busca no
    // no filho.
    int return_value = insert_into_tree(b_tree, is_fixed, curr_page.children[position], to_insert, promoted, prom_right_child);
    
    if (return_value == NO_PROMOTION || return_value == INSERT_ERROR) {
        return return_value;
    }

    // Insercao da chave sem particionamento
    if (curr_page.num_keys < MAX_KEY_AMT) {
        curr_page.keys[position] = to_insert; 

        return NO_PROMOTION; 
    }

    // Insercao da chave com particionamento/split
    int to_insert_right_child = -1;
    tree_node new_page = {};
    int new_page_rrn = -1;
    split(to_insert, &to_insert_right_child, &curr_page, promoted, &prom_right_child, &new_page, &new_page_rrn);

    // Escrever paginas/nos 
    // (curr_page e new_page)
    // alocar e inicializar nova pagina no arquivo da B tree para a new_page
    
    return PROMOTION;
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
