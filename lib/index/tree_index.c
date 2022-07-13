#include <stdlib.h>
#include <stdio.h>

#include "../record.h"
#include "tree_index.h"

/*
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
}*/

// Para posicionar o ponteiro do arquivo de indice no no desejado
void seek_node(FILE *b_tree, int rrn, bool is_fixed) {
    int byte_offset = 0;

    if (is_fixed) {
        // Calculate RRN based on the generic formulae below:
        byte_offset = rrn * BT_FIXED_SIZE + BT_FIXED_SIZE;
    }

    else {
        byte_offset = rrn * BT_VARIABLE_SIZE + BT_VARIABLE_SIZE;
    }

    // Navigate to byte_offset calculated position
    fseek(b_tree, byte_offset, SEEK_SET);
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
 */
void insert_into_tree(FILE *b_tree, bool is_fixed, int curr_rrn, key to_insert, key *promoted, int *prom_right_child) {
    // Eh aqui que se deve inserir a chave (no-folha - construcao bottom up)
    if (curr_rrn == NODE_NOT_FOUND) {
        *promoted = to_insert; // Para subir um nivel na recursao e inserir
        prom_right_child = NULL; // Porque eh no-folha

        return PROMOTION;
    }

    /* Se a pagina nao eh no-folha, chamar a funcao recursivamente ate que ela encontre
     * uma chave com o valor que queremos inserir (ja existente na tree) ou chegue ao no
     * folha para que a insercao do novo no seja feita */
    else {
        seek_node(b_tree, curr_rrn, is_fixed);
        tree_node curr_page;
        
        

    }
}


void create_tree_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {


}
