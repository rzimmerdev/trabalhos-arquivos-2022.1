#include <stdio.h>

#include "../record.h"
#include "../table.h"
#include "tree_index.h"


tree_header fread_tree_header(FILE *stream, bool is_fixed) {

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

    if (is_repeat) {
        fseek(stream, is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE, SEEK_SET);
        return;
    }

    char garbage = GARBAGE;

    for (int i = 0; i < (is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE) - 1; i++)
        fwrite(&garbage, sizeof(char), 1, stream);
}


void write_node(FILE *stream, bool is_fixed, tree_node node) {
    // Write common fields, those being the node type, as well as how many keys the node has.
    fwrite(&node.type, sizeof(char), 1, stream);
    fwrite(&node.num_keys, sizeof(int), 1, stream);
    int i = 0;

    // Write key fields (id + rrn or byteoffset) into the keys array.
    // Contains the identifier, as well as the identifier locator within
    // the original data file.
    if (is_fixed) {
        for (; i < 3; i++) {
            fwrite(&node.keys[i].id, sizeof(int), 1, stream);
            fwrite(&node.keys[i].rrn, sizeof(int), 1, stream);

        }
    } else {
        for (; i < 3; i++) {
            fwrite(&node.keys[i].id, sizeof(int), 1, stream);
            fwrite(&node.keys[i].byteoffset, sizeof(long int), 1, stream);
        }
    }

    // Finally, write rrn for the node's child nodes, being either a valid integer or -1 if the node has
    // no child at specific key position.
    for (i = 0; i < 4; i++) {
        fwrite(&node.children[i], sizeof(int), 1, stream);
    }
}


/*
 * Reads a node from a given B-Tree file stream.
 * Must be previously placed at seek position beforehand.
 * Returns struct with tree_node type, either with rrn keys or byteoffset keys filled,
 * depending on the chosen encoding.
 */
tree_node fread_node(FILE *stream, bool is_fixed) {

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

/*
 * Searches a B-Tree recursively up until a leaf node is found, in which case the function
 * returns a NOT_FOUND warning. If a node is otherwise found, the rrn_found and pos_found values
 * localize the found node position inside the tree, as well as its position locator inside the original data file.
 *
 * Example:
 *      >>> key identifier = {.id = 7}
 *      >>> int rrn_found = 0, pos_found = 0;
 *      >>> tree_search_identifier(stream, identifier, &rrn_found, &pos_found, is_fixed);
 *      rrn_found == 7, pos_found == 2;
 *
 * Args:
 *      FILE *stream: Input stream containing B-Tree nodes
 *      key identifier: Placeholder key type, with id parameter to search for
 *      int *rrn_found: Relative record number of the found index inside the B-Tree
 *      int *pos_found: Variable to use when recursively finding the index position within the nodes
 *      bool is_fixed: Index file encoding type, can be either true for fixed sized data,
 *      or false for variable sized data
 *
 * Returns:
 *      If search is successful, returns the RRN of the found record within the table file, or
 *      NOT_FOUND if no index was found with given identifier
 */
long int tree_search_identifier(FILE *stream, key identifier, int *rrn_found, int *pos_found, bool is_fixed) {

    // If rrn is -1, meaning a leaf node has been reached, return a register NOT_FOUND warning.
    if (*rrn_found == -1)
        return NOT_FOUND;

    // Calculate the current node byteoffset based on the current searched node
    long int byteoffset = (*rrn_found + 1) * (is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE);

    // Seek to calculated byteoffset, and read node at the specific position to search identifier at
    fseek(stream, byteoffset, SEEK_SET);
    tree_node current = fread_node(stream, is_fixed);

    // Search all key pairs within the node, based on the num_keys field also inside the node
    for (*pos_found = 0; *pos_found < current.num_keys; (*pos_found)++) {
        int key_id = current.keys[*pos_found].id;
        // If any identifiers matches with the desired one, return a SUCCESS_CODE
        if (identifier.id == key_id)
            return is_fixed ? (current.keys[*pos_found].rrn * FIXED_REG_SIZE + FIXED_HEADER) : current.keys[*pos_found].byteoffset;
        // Otherwise, keep iterating until not at desired sorted key position
        if (identifier.id < key_id)
            break;
    }

    // Set rrn_found value to the next position to search for, and call the
    // search function recursively
    *rrn_found = current.children[*pos_found];
    return tree_search_identifier(stream, identifier, rrn_found, pos_found, is_fixed); 
}


/*
 * Seeks stream to relative record offset within the given stream file.
 *
 * Example:
 *      >>> seek_node(stream, 2, false);
 *      Seeks to byteoffset 90 within the stream file. (45b node size for fixed index, and 57b for variable)
 *
 * Args:
 *      FILE *stream: Input stream containing B-Tree nodes
 *      int rrn: Relative register number, into which position to seek to
 *      bool is_fixed: Index file encoding type, can be either true for fixed sized data,
 *      or false for variable sized data.
 */
void seek_node(FILE *stream, int rrn, bool is_fixed) {

    // Calculate byteoffset value based on the predetermined size of each index node
    long int byteoffset = (rrn + 1) * (is_fixed ? INDEX_SIZE_FIXED: INDEX_SIZE_VARIABLE);

    // Seek to byteoffset calculated position
    fseek(stream, byteoffset, SEEK_SET);
}


// Funcao para inicializar um no de indice de arvore B
tree_node create_empty_tree_node() {
    tree_node new_node = {};

    key empty = {.id = -1, .rrn = -1, .byteoffset = -1};
    for (int i = 0; i < 3; i++) {
        new_node.keys[i] = empty;
        new_node.children[i] = -1;
    }

    new_node.children[3] = -1;
    new_node.num_keys = 0;
    new_node.type = LEAF_NODE;

    return new_node;
}

/*
 * Rotina inicializadora e de tratamento da raiz.
 * - Identifica ou cria a pagina da raiz;
 * - Le chaves para serem armazenadas na arvore-B e chama a insercao de forma apropriada;
 * - Cria uma nova raiz quando a insercao particionar a raiz corrente.
 * 
 * Parametros:
 * - FILE *index_stream -> arquivo de indice da arvore-B, ja criado e aberto
 * - header *index_header -> referencia para o cabecalho da arvore B
 * - bool is_fixed -> tipo de arquivo de dados
 */
void driver_procedure(FILE *index_stream, tree_header *index_header, bool is_fixed, key to_insert) {
    // Arvore vazia -> entao o RRN do no/pagina raiz do indice arvore B vale -1
    if (index_header->root_rrn == EMPTY_TREE) {
        // Atualize o no
        index_header->root_rrn = index_header->next_rrn;

        // Coloque a chave no no raiz
        tree_node new_node = create_empty_tree_node();
        new_node.num_keys = 1;
        new_node.type = ROOT_NODE;
        new_node.keys[0] = to_insert;

        (index_header->total_nodes)++;

        // Escrever no na arvore
        seek_node(index_stream, index_header->root_rrn, is_fixed);
        write_node(index_stream, is_fixed, new_node);

        (index_header->next_rrn)++;
        return;
    }

    key promoted = {};
    int prom_right_child = -1;
    int return_value = insert_into_tree(index_stream, index_header, is_fixed, index_header->root_rrn, to_insert, &promoted, &prom_right_child);

    // Se a promocao expandiu ate a raiz
    if (return_value == PROMOTION) {
        // Criar nova pagina de no da arvore
        tree_node new_node = create_empty_tree_node();
        (index_header->total_nodes)++;
        new_node.type = ROOT_NODE;

        // O novo no deve conter a chave promovida ate esse nivel
        new_node.keys[0] = promoted;
        new_node.children[0] = index_header->root_rrn; // Filho a esquerda
        new_node.children[1] = prom_right_child; // Filho a direita

        (new_node.num_keys)++;

        // Atualize o no raiz
        index_header->root_rrn = (index_header->next_rrn)++;

        // Escrever no na arvore
        seek_node(index_stream, index_header->root_rrn, is_fixed);
        write_node(index_stream, is_fixed, new_node);
    }
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
 * - int inserted_r_child -> referencia para filho a direita da nova chave a ser inserida
 * - tree_node *curr_page -> referencia da pagina de disco corrente
 * - key *promoted -> referencia da chave promovida
 * - tree_node *new_page -> referencia para nova pagina de disco.
 * 
 */
void split(key to_insert, int inserted_r_child, tree_node *curr_page, key *promoted, tree_node *new_page) {
    // Trazer todas as chaves e 'ponteiros'/descendentes da pagina de disco atual para
    // um espaco capaz de segurar uma chave e um filho extra
    key all_keys[4];

    for (int i = 0; i < 4; i++) {
        all_keys[i].id = -1;
        all_keys[i].byteoffset = -1;
        all_keys[i].rrn = -1;
    }

    int all_children[5];

    // Essa informacao nao muda
    all_children[0] = curr_page->children[0];

    // Copiar chaves da pagina atual
    int insert_position = -1;
    bool found_position = false;
    for (int i = 0; i < 3; i++) {
        all_keys[i].id = curr_page->keys[i].id;
        all_keys[i].byteoffset = curr_page->keys[i].byteoffset;
        all_keys[i].rrn = curr_page->keys[i].rrn;
        all_children[i + 1] = curr_page->children[i + 1];

        if (!found_position && to_insert.id < curr_page->keys[i].id) {
            insert_position = i;
            found_position = true;
        }
    }

    // Se nao achou a posicao ainda, significa que a chave a ser inserida tem o maior id
    // de todos e deve ser inserida no fim
    if (!found_position) {
        insert_position = 3;
    }

    // Shiftar para inserir a nova chave na pos. correta
    for (int i = 2; i >= insert_position; i--) {
        all_keys[i + 1].id = all_keys[i].id;
        all_keys[i + 1].byteoffset = all_keys[i].byteoffset;
        all_keys[i + 1].rrn = all_keys[i].rrn;

        all_children[i + 2] = all_children[i + 1];
    }


    // Inserindo nova chave
    all_keys[insert_position] = to_insert;
    all_children[insert_position + 1] = inserted_r_child;

    // Promover chave que esta no meio
    *promoted = all_keys[2];

    // Com o split, o tipo do no varia
    // Testar se curr_page eh no-raiz
    if (curr_page->type == ROOT_NODE) {
        // Se a arvore so contiver curr_page (so tem raiz, sem quaisquer filhos)
        if (curr_page->children[0] == NODE_NOT_FOUND) {
            // Se torna um no folha com o split
            curr_page->type = LEAF_NODE;
        }

        // Se tem curr_page como raiz, com filhos,
        else {
            // Se torna no intermediario com o split
            curr_page->type = INTERMEDIATE_NODE;
        }
    }
    // A nova pagina eh inserida na mesma altura da curr_page, entao seus
    // tipos sao iguais
    new_page->type = curr_page->type;

    // Copiar chaves e 'ponteiros' para filhos que antecedem a chave promovida
    // -> da pagina auxiliar para a pagina atual do indice
    curr_page->keys[0] = all_keys[0];
    curr_page->children[1] = all_children[1];

    curr_page->keys[1] = all_keys[1];
    curr_page->children[2] = all_children[2];

    curr_page->keys[2].id = -1;
    curr_page->keys[2].rrn = -1;
    curr_page->keys[2].byteoffset = -1;

    curr_page->children[3] = -1;
    curr_page->num_keys = 2;

    // Copiar chaves e 'ponteiros' para filhos que sucedem a chave promovida
    // -> da pagina auxiliar para a nova pagina do indice
    new_page->children[0] = all_children[3];

    new_page->keys[0] = all_keys[3];
    new_page->children[1] = all_children[4];

    new_page->keys[1].id = -1;
    new_page->keys[1].rrn = -1;
    new_page->keys[1].byteoffset = -1;
    new_page->children[2] = -1;

    new_page->keys[2].id = -1;
    new_page->keys[2].rrn = -1;
    new_page->keys[2].byteoffset = -1;
    new_page->children[3] = -1;
    new_page->num_keys = 1;
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
 * - FILE *stream -> ponteiro para arquivo da arvore
 * tree_header header -> cabecalho de arvore em RAM
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
int insert_into_tree(FILE *stream, tree_header *header, bool is_fixed, int curr_rrn, key to_insert, key *promoted, int *prom_right_child) {
    // Eh aqui que se deve inserir a chave (no-folha - construcao bottom up)
    if (curr_rrn == NODE_NOT_FOUND) {
        *promoted = to_insert; // Para subir um nivel na recursao e inserir
        *prom_right_child = -1; // Porque eh no-folha

        return PROMOTION;
    }
    /* Se a pagina nao eh no-folha, chamar a funcao recursivamente ate que ela encontre
     * uma chave com o valor que queremos inserir (ja existente na tree) ou chegue ao no
     * folha para que a insercao do novo no seja feita */
    seek_node(stream, curr_rrn, is_fixed);
    tree_node curr_page = fread_node(stream, is_fixed);

    // Pesquisar a pagina, procurando a chave de busca

    // Search all key pairs within the node/page, based on the num_keys field also inside the node
    int position = 0;
    for (; position < curr_page.num_keys; position++) {
        // Chave encontrada - nao inserir duplicata
        if (to_insert.id == curr_page.keys[position].id) {
            return INSERT_ERROR;
        }

        // Onde a chave deveria estar
        if (to_insert.id < curr_page.keys[position].id) {
            break;
        }
    }

    // A chave de busca nao foi encontrada, portanto procure a chave de busca no filho.

    // Chave e RRN promovidos do nivel inferior para serem inseridos na pagina de disco atual
    key key_promoted_to_curr = {};
    int rrn_promoted_to_curr = -1;
    int return_value = insert_into_tree(stream, header, is_fixed, curr_page.children[position], to_insert,
                                        &key_promoted_to_curr, &rrn_promoted_to_curr);

    if (return_value == NO_PROMOTION || return_value == INSERT_ERROR) {
        return return_value;
    }

    // Insercao da chave sem particionamento
    if (curr_page.num_keys < MAX_KEY_AMT) {
        // Shift das chaves e dos descendentes para a direita, abrindo espaco corretamente
        // para a insercao da nova chave
        for (int i = curr_page.num_keys - 1; i >= position; i--) {
            curr_page.keys[i + 1] = curr_page.keys[i];
            curr_page.children[i + 2] = curr_page.children[i + 1];
        }

        // Atualiza o numero de chaves (com a insercao, eh adicionada uma chave a mais na pagina)
        (curr_page.num_keys)++;

        // Atualizar chave a ser inserida e seu filho direito
        curr_page.keys[position] = key_promoted_to_curr;
        curr_page.children[position + 1] = rrn_promoted_to_curr;

        // Escrever a pagina/no
        seek_node(stream, curr_rrn, is_fixed);
        write_node(stream, is_fixed, curr_page);

        return NO_PROMOTION;
    }

    // Insercao da chave com particionamento/split
    tree_node new_page = {};
    split(key_promoted_to_curr, rrn_promoted_to_curr, &curr_page, promoted, &new_page);

    // Atualizar total de nos inseridos na arvore (aumentou pelo split)
    (header->total_nodes)++;

    // O RRN promovido eh o de newpage (new_page eh a descendente direita da chave promovida)
    int new_page_rrn = (header->next_rrn)++;
    *prom_right_child = new_page_rrn;

    // Escrever paginas/nos
    // (curr_page e new_page)
    seek_node(stream, curr_rrn, is_fixed);
    write_node(stream, is_fixed, curr_page);

    seek_node(stream, new_page_rrn, is_fixed);
    write_node(stream, is_fixed, new_page);

    return PROMOTION;
}

void create_tree_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {
    /*
     * Create an index file based on an origin stream of records, with given is_fixed file encoding.
     * Resulting index file is saved to stream of type index_stream.
     */

    // Read header from within origin_stream to iterate
    // over available records according to total records available
    header table_header = fread_header(origin_stream, is_fixed);
    tree_header index_header = {.status = BAD_STATUS[0], .next_rrn = 0, .root_rrn = EMPTY, .total_nodes = 0};
    write_tree_header(index_stream, index_header, false, is_fixed);

    int current_rrn = 0;
    long int current_byteoffset = VARIABLE_HEADER;

    // Iterate over origin stream while not at pre-calculated end position, which
    // depends on the selected encoding type
    while ((is_fixed && (current_rrn < table_header.next_rrn)) || (!is_fixed && current_byteoffset < table_header.next_byteoffset)) {

        // Reads an individual record from stream, and skips if it is
        // logically marked as removed
        data to_insert = fread_record(origin_stream, is_fixed);
        if (to_insert.removed == IS_REMOVED) {
            current_rrn += 1;
            current_byteoffset = ftell(origin_stream);

            free_record(to_insert);
            continue;
        }

        key to_indexate = {.id = to_insert.id, .rrn = current_rrn, .byteoffset = current_byteoffset};
        driver_procedure(index_stream, &index_header, is_fixed, to_indexate);
        current_rrn += 1;
        current_byteoffset += to_insert.size + 5;
        free_record(to_insert);
    }
    index_header.status = OK_STATUS[0];

    write_tree_header(index_stream, index_header, true, is_fixed);
}