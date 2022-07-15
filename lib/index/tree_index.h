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


/*
 * The header contains important info relative to the B-Tree (i.e., where to begin to search for keys)
 */
typedef struct TreeHeader_t {

    // Status of the index file, can be either OK (1) or BAD (0)
    char status;
    // RRN relative to the B-Tree's root (top node)
    int root_rrn;
    // RRN relative to the end of the B-Tree's file or next empty position, where new indices can be added to
    int next_rrn;
    // Total number of nodes saved within the B-Tree
    int total_nodes;

} tree_header;


/*
 * This B-Tree has 3 keys inside a single node (order m = 4), and each key contains the necessary info
 * to identify and indexate a given record
 */
typedef struct Key_t {

    // id field of the indexed record
    int id;
    // RRN of the indexed record within its original table file
    int rrn;
    // Byteoffset of the indexed record within its original table file
    long int byteoffset;
} key;


/*
 *  The B-Tree's node represents a disk page, containing 3 keys and pointing to the descendant pages/nodes (children).
 *  It is relevant to keep the type of the node (indicates how elevated or deep it is in relation to
 *  the tree's height - it can be a leaf, an intermediate one or the root) as well as the amount of
 *  keys already kept inside it (this helps to decide if a split will be made or not).
 */
typedef struct TreeNode_t {

    // Type of the node relative to the B-Tree, being either '0' for root node, '1' for intermediate and '2' for leaf
    char type;
    // Total number of valid keys stored inside the node
    int num_keys;
    // Array of indexed record keys, stored in ascending order by id
    key keys[3];
    // Children leaf pointers, relative to the node's key indexes, also stored in ascending order, relative to the keys
    int children[4];

} tree_node;


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
tree_header fread_tree_header(FILE *stream, bool is_fixed);


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
void write_tree_header(FILE *stream, tree_header header, bool is_repeat, bool is_fixed);


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
void write_node(FILE *stream, bool is_fixed, tree_node node);


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
tree_node fread_node(FILE *stream, bool is_fixed);


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
