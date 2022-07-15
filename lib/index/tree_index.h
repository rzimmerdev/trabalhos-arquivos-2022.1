#ifndef TREE_INDEX_H
#define TREE_INDEX_H

// Size of index/b-tree's header, depending on data filetype ---
#define INDEX_SIZE_FIXED 45 // for type 1 files
#define INDEX_SIZE_VARIABLE 57 // for type 2 files

// Insertion return values (functionality 11) ---
#define PROMOTION 1 // Insertion is made and key is promoted (with full node/node overflow)
#define NO_PROMOTION 0 // Insertion is made and no key is promoted (node still has available space)
#define INSERT_ERROR -2 // Key to be inserted is already indexated by b-tree (primary key index)

// To identify a leaf node on insertion (its children are -1) ---
// (it's used inside recursion to stop it - stop condition)
#define NODE_NOT_FOUND -1 

// Maximum amount of key inside a single b-tree node ---
#define MAX_KEY_AMT 3

// Indicates if the b-tree's root is -1 (non-existant) ---
#define EMPTY_TREE -1

// B-tree's possible node types --- 
#define ROOT_NODE '0'
#define INTERMEDIATE_NODE '1'
#define LEAF_NODE '2'

// The header contains important b-tree's info (i.e., where to begin to search for keys)
typedef struct TreeHeader_t {

    char status;
    int root_rrn;
    int next_rrn;
    int total_nodes;

} tree_header;

// This b-tree's has 3 keys inside a single node (order m = 4), and each key contains the necessary info
// to indexate a given record
typedef struct Key_t {
    int id;
    int rrn;
    long int byteoffset;
} key;

// The b-tree's node represents a disk page, containing 3 keys and pointing to the descendant pages/nodes (children).
// It is relevant to keep the type of the node (indicates how elevated or deep it is in relation to
// the tree's height - it can be a leaf, an intermediate one or the root) as well as the amount of
// keys already kept inside it (this helps deciding if a split will be made or not).
typedef struct TreeNode_t {

    char type;
    int num_keys;

    key keys[3];

    int children[4];
} tree_node;

tree_node fread_node(FILE *stream, bool is_fixed);

tree_header fread_tree_header(FILE *stream, bool is_fixed);

void write_tree_header(FILE *stream, tree_header header, bool is_repeat, bool is_fixed);

long int tree_search_identifier(FILE *stream, key identifier, int *rrn_found, int *pos_found, bool is_fixed);

/*
 * Initializer routine to make insertions into tree - it treats the root by:
 * - Identifying or creating root page/node (if it still does not exist);
 * - Reading keys to be kept inside b-tree and calling insert_into_tree() properly, guaranteeing
 * safe parameters' conditions;
 * - Creating a new root when insert_into_tree() partitionates the current one.
 * 
 * Args:
 * - FILE *index_stream -> b-tree's index file, already created and opened
 * - header *index_header -> pointer to b-tree's header (so it can be changed by insertion inside this function)
 * - bool is_fixed -> data filetype
 */
void driver_procedure(FILE *index_stream, tree_header *index_header, bool is_fixed, key to_insert);

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
 * - FILE *b_tree -> pointer to b-tree's/index file
 * - tree_header *header -> b-tree's header loaded on RAM
 * - bool is_fixed -> data filetype
 * - int curr_rrn -> RRN of current used b-tree's page (the start value is the root). It's the page
 * to be searched (its value changes during the execution of function)
 * - key to_insert -> key to be inserted
 * - key *promoted -> reference of promoted key, in case insertion results on partitioning and
 * key promotion
 * - int *prom_right_child -> reference to the right child of promoted key. It's a RRN, used
 * when a partitioning occurs, 'cause both promoted key and the newpage's RRN (created by partitioning)
 * should be inserted in a superior level node inside b-tree.
 * 
 * Returns: 
 *     int -> can be PROMOTION, INSERT_ERROR or NO_PROMOTION.
 */
int insert_into_tree(FILE *b_tree, tree_header *header, bool is_fixed, int curr_rrn, key to_insert, key *promoted, int *prom_right_child);

void create_tree_index(FILE *origin_stream, FILE *index_stream, bool is_fixed);

#endif //TREE_INDEX_H
