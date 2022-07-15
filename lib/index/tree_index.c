#include <stdio.h>

#include "../record.h"
#include "../table.h"
#include "tree_index.h"



key empty_key() {
    key empty = {.id = EMPTY, .rrn = EMPTY, .byteoffset = EMPTY};
    return empty;
}


/*
 * Reads the B-Tree header from the given index file stream.
 *
 * Args:
 *      FILE *stream: B-Tree initialized file with header field to read
 *      bool is_fixed: Encoding of the given index file stream
 *
 * Returns:
 *      Header of type tree_header, with all its parameters filled
 */
tree_header fread_tree_header(FILE *stream, bool is_fixed) {

    // Define header variable and seek to header position (file start)
    tree_header header = {};
    fseek(stream, 0, SEEK_SET);

    // Reads individual values in respective order
    fread(&header.status, sizeof(char), 1, stream);
    fread(&header.root_rrn, sizeof(int), 1, stream);
    fread(&header.next_rrn, sizeof(int), 1, stream);
    fread(&header.total_nodes, sizeof(int), 1, stream);

    // Seeks after the garbage characters at end of header, going to end of header
    fseek(stream, is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE, SEEK_SET);

    return header;
}


/*
 * Writes a variable of type tree_header to given index file stream.
 *
 * Example:
 *      >>> tree_header header = {.status = OK_STATUS, .root_rrn = EMPTY, .next_rrn = 0, .total_nodes = 0};
 *      >>> bool repeat = false;
 *      >>> write_tree_header(stream, header, is_repeat, is_fixed);
 *
 * Args:
 *      FILE *stream: B-Tree initialized file, at any given stream position
 *      tree_header header: TreeHeader with filled values to insert at start of index file
 *      bool is_repeat: Whether to rewrite garbage characters at end of header or just skip them
 *      bool is_fixed: Encoding of the given index file stream
 */
void write_tree_header(FILE *stream, tree_header header, bool is_repeat, bool is_fixed) {

    // Seek to file start to begin writing parameters
    fseek(stream, 0, SEEK_SET);

    // Writes individual values in respective order
    fwrite(&header.status, sizeof(char), 1, stream);
    fwrite(&header.root_rrn, sizeof(int), 1, stream);
    fwrite(&header.next_rrn, sizeof(int), 1, stream);
    fwrite(&header.total_nodes, sizeof(int), 1, stream);

    // If header has already been written before, and there this is a repeated write operation,
    // simply skip garbage characters at end of pre-existing header
    if (is_repeat) {
        fseek(stream, is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE, SEEK_SET);
        return;
    }

    char garbage = GARBAGE;

    // If header has not been written before inside the index file, write garbage characters
    // to fill remaining header size
    for (int i = 0; i < (is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE) - 1; i++)
        fwrite(&garbage, sizeof(char), 1, stream);
}


/*
 * Write a filled tree_node to the given B-Tree index file stream, at actual position within the stream.
 *
 * Example:
 *      >>> tree_node node = create_empty_tree_node();
 *      >>> write_node(stream, is_fixed, node);
 *
 * Args:
 *      FILE *stream: B-Tree initialized file, at desired position for new node to be written at
 *      bool is_fixed: Encoding of the given index file stream
 */
void write_node(FILE *stream, bool is_fixed, tree_node node) {
    // Write common fields, those being the node type, as well as how many keys the node has.
    fwrite(&node.type, sizeof(char), 1, stream);
    fwrite(&node.num_keys, sizeof(int), 1, stream);
    int i = 0;

    // Write key fields (id + rrn or byteoffset) into the keys array.
    // Contains the identifier, as well as the identifier locator within
    // the original data file.
    if (is_fixed) {

        // For each key, write both its id and respective rrn relative to the table file
        for (; i < 3; i++) {
            fwrite(&node.keys[i].id, sizeof(int), 1, stream);
            fwrite(&node.keys[i].rrn, sizeof(int), 1, stream);

        }
    } else {

        // For each key, write both its id and respective byteoffset relative to the table file
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
 * Reads a node from a given B-Tree file stream. Must be previously placed at seek position beforehand.
 * Returns struct with tree_node type, either with rrn keys or byteoffset keys filled, epending on the chosen encoding.
 *
 * Args:
 *      FILE *stream: B-Tree initialized file, at desired position for node to be read from
 *      bool is_fixed: Encoding of the given index file stream
 *
 * Returns:
 *      Variable of type tree_node read from index file at current stream position
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
 *      If search is successful, returns the byteoffset of the found record within the table file, or
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
    long int byteoffset = (rrn + 1) * (is_fixed ? INDEX_SIZE_FIXED : INDEX_SIZE_VARIABLE);

    // Seek to byteoffset calculated position
    fseek(stream, byteoffset, SEEK_SET);
}

// To initialize a b-tree's index node (without keys and all values are -1/invalid/EMPTY)
tree_node create_empty_tree_node() {
    tree_node new_node = {};

    key empty = empty_key();
    for (int i = 0; i < 3; i++) {
        new_node.keys[i] = empty;
        new_node.children[i] = EMPTY;
    }

    new_node.children[3] = EMPTY;
    new_node.num_keys = 0;
    new_node.type = LEAF_NODE;

    return new_node;
}

/*
 * Initializer routine to make insertions into tree - it treats the root by:
 * · Identifying or creating root page/node (if it still does not exist);
 * · Reading keys to be kept inside b-tree and calling insert_into_tree() properly, guaranteeing safe parameters' conditions;
 * · Creating a new root when insert_into_tree() partitionates the current one.
 * 
 * Args:
 *      FILE *index_stream: b-tree's index file, already created and opened
 *      header *index_header: pointer to b-tree's header (so it can be changed by insertion inside this function)
 *      bool is_fixed: Index file encoding type, can be either true for fixed sized data,
 *      or false for variable sized data.
 */
void driver_procedure(FILE *index_stream, tree_header *index_header, bool is_fixed, key to_insert) {
    // Empty b-tree -> RRN of b-tree's root page/node is -1 
    if (index_header->root_rrn == EMPTY_TREE) {
        // Updates the root node with next RRN available
        index_header->root_rrn = index_header->next_rrn;

        // Puts new received key to be inserted on root node
        tree_node new_node = create_empty_tree_node();
        new_node.num_keys = 1;
        new_node.type = ROOT_NODE;
        new_node.keys[0] = to_insert;

        // Updates number of nodes on b-tree
        (index_header->total_nodes)++;

        // Writes new node on b-tree file
        seek_node(index_stream, index_header->root_rrn, is_fixed);
        write_node(index_stream, is_fixed, new_node);

        (index_header->next_rrn)++;
        return;
    }

    // Tree is not empty. Call recursive insertion and initialize possible promoted keys (promotion
    // can be propagated and used after insert_into_tree()).
    key promoted = {};
    int prom_right_child = EMPTY;
    int return_value = insert_into_tree(index_stream, index_header, is_fixed, index_header->root_rrn, to_insert, &promoted, &prom_right_child);

    // In case promotion propagates all way to root
    if (return_value == PROMOTION) {
        // It's necessary to create a new b-tree's node/disk page
        tree_node new_node = create_empty_tree_node();
        (index_header->total_nodes)++;

        // A new root will be created
        new_node.type = ROOT_NODE;

        // New root should contain the key that was promoted up to this level
        new_node.keys[0] = promoted;

        // Connecting the new root with previous information on top of b-tree
        new_node.children[0] = index_header->root_rrn; // Previous root becomes left child
        new_node.children[1] = prom_right_child; // Right child

        (new_node.num_keys)++;

        // Updates root node
        index_header->root_rrn = (index_header->next_rrn)++;

        // Writes node into b-tree (search's beginning will change with the new root's RRN)
        seek_node(index_stream, index_header->root_rrn, is_fixed);
        write_node(index_stream, is_fixed, new_node);
    }
}

/*
 * Treatment of overflow caused by the insertion of a key inside b-tree.
 * The function:
 * · Creates a new disk page/node;
 * · Distributes keys (between already existing page and the one to be created) in the most possibly uniform way;
 * · Determinates which keys and RRNs will be promoted.
 * 
 * Args:
 *      key to_insert: new key to be inserted inside tree
 *      int inserted_r_child: reference of to be inserted key's right child
 *      tree_node *curr_page: reference of current disk page or b-tree's node
 *      key *promoted: reference of promoted key
 *      tree_node *new_page: reference of new disk page or b-tree's node.
 */
void split(key to_insert, int inserted_r_child, tree_node *curr_page, key *promoted, tree_node *new_page) {
    // Brings all keys and current disk page's `pointers`/descendants/children to a working
    // space capable of holding an extra key and an extra child 
    key all_keys[4];
    int all_children[5];

    // This information does not change (insertion is only made by right side and this is
    // the child at max left)
    all_children[0] = curr_page->children[0];

    // Copy curr page's keys
    int insert_position = 0;
    for (; insert_position < 3; insert_position++) {
        all_keys[insert_position] = curr_page->keys[insert_position];
        all_children[insert_position + 1] = curr_page->children[insert_position + 1];

        if (to_insert.id < curr_page->keys[insert_position].id)
            break;
    }

    // Shifting all info to insert new key at correct position
    for (int i = 2; i >= insert_position; i--) {
        all_keys[i + 1] = all_keys[i];
        all_children[i + 2] = all_children[i + 1];
    }

    // Now inserting new key
    all_keys[insert_position] = to_insert;
    all_children[insert_position + 1] = inserted_r_child;

    // The working space is ready and ordenated, turning possible the promotion of
    // middle key (most uniform promotion as possible)
    *promoted = all_keys[2];

    // After the split, node type varies. It's necessary to test if curr_page is
    // a root node.
    if (curr_page->type == ROOT_NODE) {
        // If curr_page is a root node at the moment and b-tree contains only
        // this node (b-tree only has root, without any children), curr_page
        // now is a leaf node after split
        if (curr_page->children[0] == NODE_NOT_FOUND) {
            curr_page->type = LEAF_NODE;
        }

        // Otherwise, if b-tree contains curr_page and other nodes (root has children),
        // curr_page turns into an intermediate node after the split.
        else {
            curr_page->type = INTERMEDIATE_NODE;
        }
    }

    // New page is inserted at same level of curr_page, so its node types are same ---
    new_page->type = curr_page->type;

    // Copy keys and children `pointers` that precede promoted key
    // --> from auxiliar page to curr index page
    curr_page->num_keys = 2;
    curr_page->keys[0] = all_keys[0];
    curr_page->keys[1] = all_keys[1];
    curr_page->keys[2] = empty_key();

    curr_page->children[0] = all_children[0];
    curr_page->children[1] = all_children[1];
    curr_page->children[2] = all_children[2];
    curr_page->children[3] = EMPTY;

    // Copy keys and children `pointers` that succede promoted key
    // --> from auxiliar page to new index page
    new_page->num_keys = 1;
    new_page->keys[0] = all_keys[3];
    new_page->keys[1] = empty_key();
    new_page->keys[2] = empty_key();

    new_page->children[0] = all_children[3];
    new_page->children[1] = all_children[4];
    new_page->children[2] = EMPTY;
    new_page->children[3] = EMPTY;
}


/*
 * Function called inside driver_procedure() and inside create_tree_index(). It begins with
 * a search by the root, following to the level of leaf nodes (top-bottom). Since the leaf 
 * node where the received key should be inserted is found (its correct position inside the b-tree),
 * processes of insertion, partition/split and key promotion propagate, beginning on the 
 * leaves' level and following to the tree's root (bottom-up construction of the tree).
 *
 * The insertion process is recursive. Its phases are:
 * - search for the correct page/node to insert new key (before recursive call);
 * - recursive call (sending inserting operation to b-tree's inferior levels)
 * - insertion, split and promotion (executed after recursive call. The propagation of these
 * occurs on recursion's returns).
 * 
 * Args:
 *      FILE *stream: pointer to b-tree's/index file
 *      tree_header *header: b-tree's header loaded on RAM
 *      bool is_fixed: data filetype
 *      int curr_rrn: RRN of current used b-tree's page (the start value is the root).
 *      It's the page to be searched (its value changes during the execution of function)
 *      key to_insert: key to be inserted
 *      key *promoted: reference of promoted key, in case insertion results on partitioning and key promotion
 *      int *prom_right_child: reference to the right child of promoted key. It's a RRN, used
 *      when a partitioning occurs, 'cause both promoted key and the newpage's RRN (created by partitioning)
 *      should be inserted in a superior level node inside b-tree.
 * 
 * Returns: 
 *     Operation result, can be PROMOTION, INSERT_ERROR or NO_PROMOTION.
 */
int insert_into_tree(FILE *stream, tree_header *header, bool is_fixed, int curr_rrn, key to_insert, key *promoted, int *prom_right_child) {
    // This is where key should be inserted (when recursion comes back, it reaches leaf node -> bottom-up construction
    // of b-tree)
    if (curr_rrn == NODE_NOT_FOUND) {
        *promoted = to_insert; // Go up a level on recursion and insert key (it has already reached the max bottom of b-tree)
        *prom_right_child = EMPTY; // A key can only be inserted on a leaf node, so it has no child

        return PROMOTION;
    }

    /* Otherwise, if the current page is not a leaf node, call function recursively until
     * - it finds a key with the value to be inserted (meaning key already exists on tree) or
     * - it reaches a leaf node so the insertion of the new node can be made.
    */
    seek_node(stream, curr_rrn, is_fixed);
    tree_node curr_page = fread_node(stream, is_fixed);

    // Search through page, looking for searched key ---
        // Search all key pairs within the node/page, based on the num_keys field also inside the node

    int position = 0;
    for (; position < curr_page.num_keys; position++) {
        // 1. Key was found - don't insert same key value
        if (to_insert.id == curr_page.keys[position].id) {
            return INSERT_ERROR;
        }

        // 2. Position where key should be was found
        if (to_insert.id < curr_page.keys[position].id) {
            break;
        }
    }

    // Searched key was not found, so look for it inside children nodes/pages using recursion ---

    // Key and RRN promoted from lower level; should be inserted on current disk pages/nodes
    key key_promoted_to_curr = {};
    int rrn_promoted_to_curr = EMPTY;
    int return_value = insert_into_tree(stream, header, is_fixed, curr_page.children[position], to_insert,
                                        &key_promoted_to_curr, &rrn_promoted_to_curr);

    if (return_value == NO_PROMOTION || return_value == INSERT_ERROR) {
        return return_value;
    }

    // Inserting key without partitioning ---
    if (curr_page.num_keys < MAX_KEY_AMT) {
        // Keys' and descendants' right shifting, making room correctly for new key's insertion 
        for (int i = curr_page.num_keys - 1; i >= position; i--) {
            curr_page.keys[i + 1] = curr_page.keys[i];
            curr_page.children[i + 2] = curr_page.children[i + 1];
        }

        // Updating key amount inside node/disk page (with the insertion made, the page
        // carries one more key)
        (curr_page.num_keys)++;

        // Updating curr page, correctly positioning key to be inserted and its right child
        curr_page.keys[position] = key_promoted_to_curr;
        curr_page.children[position + 1] = rrn_promoted_to_curr;

        // Writing just modified page/tree node, saving b-tree's changes 
        seek_node(stream, curr_rrn, is_fixed);
        write_node(stream, is_fixed, curr_page);

        return NO_PROMOTION;
    }

    // Inserting key with partitioning/split ---
    tree_node new_page = {};
    split(key_promoted_to_curr, rrn_promoted_to_curr, &curr_page, promoted, &new_page);

    // Updating total of b-tree's inserted nodes (it has increased by splitting)
    (header->total_nodes)++;

    // Promoted RRN is newpage's (indicates that newpage is the right descendant of promoted key)
    int new_page_rrn = (header->next_rrn)++;
    *prom_right_child = new_page_rrn;

    // Writing just modified pages/tree nodes, saving b-tree's changes 
    // at curr_page and new_page (split results)
    seek_node(stream, curr_rrn, is_fixed);
    write_node(stream, is_fixed, curr_page);

    seek_node(stream, new_page_rrn, is_fixed);
    write_node(stream, is_fixed, new_page);

    return PROMOTION;
}


/*
 * Create an index file based on an origin stream of records, with given is_fixed file encoding.
 * Resulting index file is saved to stream of type index_stream.
 *
 * Args:
 *      FILE *origin_stream: Input stream containing table records
 *      FILE *index_stream: Final stream to write B-Tree nodes after insertion into
 *      bool is_fixed: Index file encoding type, can be either true for fixed sized data,
 *      or false for variable sized data.
 */
void create_tree_index(FILE *origin_stream, FILE *index_stream, bool is_fixed) {

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

        // Calls driver procedure on record transformed to index
        // Driver procedure deals automatically with index with no elements, so no extra preprocessing
        // has to be done
        key to_indexate = {.id = to_insert.id, .rrn = current_rrn, .byteoffset = current_byteoffset};
        driver_procedure(index_stream, &index_header, is_fixed, to_indexate);

        // Increment counting variables, free record and continue iterating
        current_rrn += 1;
        current_byteoffset += to_insert.size + 5;
        free_record(to_insert);
    }
    index_header.status = OK_STATUS[0];

    // Update header on the index file after finishing all insertions
    write_tree_header(index_stream, index_header, true, is_fixed);
}