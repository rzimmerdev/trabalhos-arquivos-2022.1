#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include "../lib/utils.h"
#include "../lib/record.h"
#include "../lib/table.h"
#include "../lib/index/tree_index.h"

#include "commands.h"
#include "csv_utils.h"


/*
 * Generic function to verify stream of record data and record indices.
 * Uses the verify_index boolean value to decide whether to verify already existing index file or not.
 */
int verify_stream(char *data_filename, char *index_filename, bool verify_index) {

    // ====================================================
    // Verify if data stream exists and its status is valid
    // ====================================================
    FILE *data_stream = fopen(data_filename, "rb");

    // Verify if data file exists, otherwise return error
    if (data_stream == NULL)
        return ERROR_CODE;

    // Access data stream and read status field from header
    int data_status = (read_status(data_stream) == OK_STATUS[0]);
    fclose(data_stream);

    if (data_status != SUCCESS_CODE)
        return ERROR_CODE;

    // If the verify_index parameter is false, return function as the data stream has been
    // verified to exist and not be corrupted
    if (!verify_index)
        return SUCCESS_CODE;

    // =====================================================
    // Verify if index stream exists and its status is valid
    // =====================================================
    FILE *index_stream = fopen(index_filename, "rb");

    // Verify if index file exists, otherwise return error
    if (index_stream == NULL)
        return ERROR_CODE;

    // Access data stream and read status field from header
    int index_status = (read_status(index_stream) == OK_STATUS[0]);
    fclose(index_stream);

    if (index_status != SUCCESS_CODE)
        return ERROR_CODE;

    return SUCCESS_CODE;
}


/*
 * Creates an abstract table with given input and saves result to a specific file.
 *
 * Args:
 *     char *csv_filename: CSV file to read input data from
 *     char *out_filename: Filename to which to write binary data to
 *     bool is_fixed: Selected specific encoding for resulting file, can be either FIXED (1) or VARIABLE (0)
 *
 * Returns:
 *     int: Returns SUCCESS_CODE if read and write operations were successful, and ERROR_CODE otherwise.
 */
int create_table_command(char *csv_filename, char *out_filename, bool is_fixed) {

    FILE *csvstream = fopen(csv_filename, "r");

    // Verify if input and output files can be found
    if (csvstream == NULL)
        return ERROR_CODE;

    FILE *binstream = fopen(out_filename, "wb");

    // Function to read data from CSV and write to binary encoded file, given desired encoding type.
    csv_to_bin(csvstream, binstream, is_fixed);

    fclose(csvstream);
    fclose(binstream);

    return SUCCESS_CODE;
}


/* Prints to console records read from given file, according to expected file encoding type.
 *
 * Args:
 *     char *bin_filename: Binary file which contains table to select from
 *     bool is_fixed: Expected encoding for selected file, can be either FIXED (1) or VARIABLE (0)
 *
 * Returns:
 *     int: Returns SUCCESS_CODE if table could be accessed, and ERROR_CODE otherwise.
 */
int select_command(char *bin_filename, bool is_fixed) {

    if (verify_stream(bin_filename, NULL, false) == ERROR_CODE)
        return ERROR_CODE;

    FILE *stream = fopen(bin_filename, "rb");
    int status = select_table(stream, is_fixed);
    fclose(stream);

    return status;
}


typedef struct { char *key; int val; } table_dict;


static table_dict column_lookup[] = {
        { "id", 0 }, { "ano", 1 }, { "qtt", 2 }, { "sigla", 3 },
        { "cidade", 4}, { "marca", 5}, { "modelo", 6}
};


/*
 * Function to encode column string value to specific numeric value, as to perform conditional operations.
 *
 * Args:
 *     char *key: Column name to be used as key from _table_dict_ column_lookup dictionary.
 *
 * Returns:
 *     int: Numeric value corresponding to encoded column name, or -1 if encoding doesn't exist for given value.
 */
int lookup(char *key) {
    for (int i = 0; i < 7; i++) {
        table_dict *mapped = &column_lookup[i];

        if (strcmp(mapped->key, key) == 0)
            return mapped->val;
    }
    return -1;
}


data scanf_filter(int total_parameters) {
    data template = {.id = EMPTY_FILTER, .year = EMPTY_FILTER, .total = EMPTY_FILTER, .state = EMPTY_FILTER,
                     .city = NULL, .model = NULL, .brand = NULL};
    for (int i = 0; i < total_parameters; i++) {
        char *column = scan_quote_string();
        int fixed;
        char *variable;

        int column_code = lookup(column);
        free(column);

        if (column_code == 0 || column_code == 1 || column_code == 2)
            scanf("%d ", &fixed);
        else {
            variable = scan_quote_string();
        }

        // Switches column_code to read value from console into correct template filter field.
        switch (column_code) {
            case 0: {
                template.id = fixed;
                break;
            }
            case 1: {
                template.year = fixed;
                break;
            }
            case 2: {
                template.total = fixed;
                break;
            }
            case 3: {
                // If you want to change state to null (entry for state equals NULO,
                // so you want to put $$ on that field)
                template.state[0] = GARBAGE;
                template.state[1] = GARBAGE;
                memcpy(template.state, variable, strlen(variable));
                free(variable);
                break;
            }
            case 4: {
                template.city = variable;
                break;
            }
            case 5: {
                template.brand = variable;
                break;
            }
            case 6: {
                template.model = variable;
                break;
            }
        }
    }

    return template;
}


/*
 * Prints to console all records read from given file that match all parameters read from console.
 *
 * Example:
 *      // int total_parameters = 1;
 *      >>> ano 2012
 *      >>> marca "FIAT"
 *
 *      MARCA DO VEICULO: FIAT
 *      MODELO DO VEICULO: UNO MILLE ECONOMY
 *      ANO DE FABRICACAO: 2012
 *      NOME DA CIDADE: BELO HORIZONTE
 *      QUANTIDADE DE VEICULOS: 1411
 *
 * Args:
 *     char *bin_filename: Binary file which contains table to select specific records from
 *     int total_parameters: Total parameters to be read from console and be used as filters
 *     bool is_fixed: Expected encoding for selected file, can be either FIXED (1) or VARIABLE (0)
 *
 *  Returns:
 *      int: Returns SUCCESS_CODE if any table record could be read,
 *           NOT_FOUND if it was read as removed or is non-existent, and ERROR_CODE otherwise.
 */
int select_where_command(char *bin_filename, int total_parameters, bool is_fixed) {

    if (verify_stream(bin_filename, NULL, false) == ERROR_CODE)
        return ERROR_CODE;

    FILE *stream = fopen(bin_filename, "rb");
    header file_header = fread_header(stream, is_fixed);

    // Read template filter from console, to be able to compare such filter with each
    // record in given file.
    data template = scanf_filter(total_parameters);

    int status = select_where(stream, template, file_header, is_fixed);

    free_record(template);
    fclose(stream);

    return status;
}


/*
 * Prints to console a single record inside selected fixed record encoded file with given relative record number.
 *
 * Args:
 *     char *bin_filename: Binary file which contains table to select specific record from
 *     int rrn: Relative number refering to position of record in file encoded with fixed records
 *
 * Returns:
 *      int: Returns SUCCESS_CODE if table record could be read,
 *           NOT_FOUND if it was read as removed or is non-existent, and ERROR_CODE otherwise
 */
int select_rrn_command(char *bin_filename, int rrn) {

    if (verify_stream(bin_filename, NULL, false) == ERROR_CODE)
        return ERROR_CODE;

    FILE *stream = fopen(bin_filename, "rb");
    header file_header = fread_header(stream, true);
    // Decide whether desired rrn is within expected range
    if ((rrn < 0) || (rrn >= file_header.next_rrn)) {
        return NOT_FOUND;
    }

    // Calculate RRN based on the generic formulae below:
    long int byte_offset = rrn * 97 + FIXED_HEADER;

    // Navigate to byte_offset calculated position
    fseek(stream, byte_offset, SEEK_SET);

    data record = fread_record(stream, true);
    fclose(stream);

    if (record.removed != IS_REMOVED) {
        printf_record(record);
        free_record(record);
        return SUCCESS_CODE;
    }
    else
        return NOT_FOUND;
}


/*
 * Creates an index table with index_filename, based on records read
 * from input data file, with specified file encoding
 * Indexes are created as pairs of id's and rrn's or byteoffset's, depending
 * on the input record type.
 */
int create_index_command(char *data_filename, char *index_filename, bool is_fixed) {

    // Verify integrity and existance of only the data files,
    // by marking the verify_index field as false
    if (verify_stream(data_filename, index_filename, false) == ERROR_CODE)
        return ERROR_CODE;

    // Open data file for reading
    FILE *original_stream = fopen(data_filename, "rb");

    // Open index file for writting, and update status field to
    // account for any possible file corruptions
    FILE *index_stream = fopen(index_filename, "wb");
    update_status(index_stream, BAD_STATUS);

    create_index(original_stream, index_stream, is_fixed);
    fclose(original_stream);

    // After performing operations, update status field
    // and close index file
    update_status(index_stream, OK_STATUS);
    fclose(index_stream);

    return SUCCESS_CODE;
}


/*
 * Deletes multiple records from data file, using a sequence of filters
 * to select which records to be deleted. Index file is used to
 * perform search by index, increasing speed of deletion as index
 * values are stored in primary memory (RAM)
 */
int delete_records_command(char *data_filename, char *index_filename, int total_filters, bool is_fixed) {

    // Verify integrity and existance of both data and index files
    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    // Open data stream as read-write operation,
    // and therefore update write status to bad status to keep track of possible file corruption
    FILE *data_stream = fopen(data_filename, "rb+");
    update_status(data_stream, BAD_STATUS);

    header file_header = fread_header(data_stream, is_fixed);

    // Load index file to RAM memory, by generating array with indices
    index_array index = index_to_array(index_filename, is_fixed);

    // Iterate over all filters to be read from console,
    // and perform removal operation accordingly
    for (; total_filters > 0; total_filters--) {

        // Scan filter values from console, as to perform remove operation
        // based on the WHERE parameters (filtered record removal)
        int total_parameters; scanf("%d ", &total_parameters);
        data filter = scanf_filter(total_parameters);

        fseek(data_stream, is_fixed ? FIXED_HEADER : VARIABLE_HEADER, SEEK_SET);
        remove_where(data_stream, &index, filter, &file_header, is_fixed);

        // Free filter read for each loop iteration
        free_record(filter);
    }

    write_header(data_stream, file_header, is_fixed, true);

    // Finally, close data stream and exit remove_where command
    update_status(data_stream, OK_STATUS);
    fclose(data_stream);

    // Writes index array stored in RAM back to index file,
    // thus greatly optimizing previous index operations speeds as
    // writing back to index file is performed only after all deletions are made
    array_to_index(index, is_fixed);
    free_index_array(&index);

    return SUCCESS_CODE;
}


/*
 * Inserts multiple records into data file, as well as into an index file
 * New records with variable size are inserted according to Worst Fit strategy into
 * previously removed record spaces, or into the end of the file if no previously removed space is available.
 */
int insert_records_command(char *data_filename, char *index_filename, int total_insertions, bool is_fixed) {

    // Testing the existance of both files, and expecting valid status for both
    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    FILE *data_stream = fopen(data_filename, "rb+");
    update_status(data_stream, BAD_STATUS);

    header file_header = fread_header(data_stream, is_fixed);

    // Loading index to RAM
    index_array index = index_to_array(index_filename, is_fixed);

    // Reading functionality's 7 entry format
    for (int i = 0; i < total_insertions; i++) {
        data curr_insertion = read_record_entry(is_fixed);

        insert_into(data_stream, &index, curr_insertion, is_fixed, &file_header);
        free_record(curr_insertion);
    }

    // Writing header on the file to disk
    write_header(data_stream, file_header, is_fixed, 0);

    // To finish writing on the data file, set status as OK_STATUS
    update_status(data_stream, OK_STATUS);
    fclose(data_stream);

    // Writes index array stored in RAM back to index file,
    // thus greatly optimizing previous index operations speeds as
    // writing back to index file is performed only after all deletions are made
    array_to_index(index, is_fixed);
    free_index_array(&index);
    
    return SUCCESS_CODE;
}


/*
 * Updates a sequence of records from given console input.
 * Updating method depends on the selected data encoding.
 * If the new record does not fit on the previously existing space (specifically for variabel sized records),
 * record is deleted and reinserted with updated parameters to next available space according to Worst Fit method.
 */
int update_records_command(char *data_filename, char *index_filename, int total_updates, bool is_fixed) {

    // Testing file pointers and their validity. Then, testing the consistency of the file to which they point
    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    FILE *data_stream = fopen(data_filename, "rb+");
    // To write on the data file, set status as BAD_STATUS
    update_status(data_stream, BAD_STATUS);

    header file_header = fread_header(data_stream, is_fixed);

    // Loading index to RAM
    index_array index = index_to_array(index_filename, is_fixed);

    for (int i = 0; i < total_updates; i++) {
        // Reading functionality's 8 entry format
        fseek(data_stream, 0, SEEK_SET);
        int total_parameters;

        // Read search's parameters to look out for records that have these values on
        // specified fields
        scanf("%d ", &total_parameters);
        data filter = scanf_filter(total_parameters);

        // Read update's parameters to change the records that are found because of
        // the search parameters above
        scanf("%d ", &total_parameters);

        // These are the updated values that will replace the old ones
        data params = scanf_filter(total_parameters); 

        if (is_fixed) {
            update_fixed_filtered(data_stream, &index, filter, params, &file_header);
        }

        /* For records that have variable size, test record's space using its size. If it has
         * not enough space, it will be necessary the call to removal (func. 6) in order to
         * delete the old record from that spot, then the insertion (func. 7) will be called 
         * so the updated record can have a suitable space to occupy.  
        */
        else {
            update_variable_filtered(data_stream, &index, filter, params, &file_header);
        }

        free_record(filter);
        free_record(params);
    }

    // Writing header on the file to disk
    write_header(data_stream, file_header, is_fixed, 0);

    // To finish writing on the data file, set status as OK_STATUS
    update_status(data_stream, OK_STATUS);
    fclose(data_stream);

    // Writes index array stored in RAM back to index file,
    // thus greatly optimizing previous index operations speeds as
    // writing back to index file is performed only after all deletions are made
    array_to_index(index, is_fixed);
    free_index_array(&index);

    return SUCCESS_CODE;
}


/*
 * Prints to console a given record from the table file, using an id as filter parameter.
 * Accesses the id using the B-Tree index file, and read record using the saved respective RRN or Byteoffset.
 *
 * Example:
 *      >>> int id = 7;
 *      >>> select_id_command(data_filename, index_filename, id, is_fixed);
 *      MARCA DO VEICULO: HONDA
 *      MODELO DO VEICULO: NXR160 BROS ESDD
 *      ANO DE FABRICACAO: 2020
 *      NOME DA CIDADE: MESQUITA
 *      QUANTIDADE DE VEICULOS: 21
 *
 * Args:
 *      FILE *data_filename: Table file containing records information, with fixed or variable sized records
 *      FILE *index_filename: Index file containing B-Tree nodes with a key type identifier
 *      int id: Identifier id to use when searching through B-Tree
 *      bool is_fixed: Expected encoding for selected file, can be either FIXED (1) or VARIABLE (0)
 *
 * Returns:
 *      SUCCESS_CODE if table and index could be opened, and ERROR_CODE otherwise
 */
int select_id_command(char *data_filename, char *index_filename, int id, bool is_fixed) {

    // Verify if both the table and the index files exists inside the working directory
    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    // Open the index file to search for the byteoffset of the matching record
    FILE *index_stream = fopen(index_filename, "r");
    tree_header index_header = fread_tree_header(index_stream, is_fixed);

    // Create a template identifier containing the lookup id
    key identifier = {.id = id};

    // Call the tree_search_identifier function using the defined identifier
    int rrn_found = index_header.root_rrn, pos_found = 0;
    long int byteoffset = tree_search_identifier(index_stream, identifier, &rrn_found, &pos_found, is_fixed);
    fclose(index_stream);

    // If the identifier was not found within the index file, return a warning message
    if (byteoffset == NOT_FOUND)
        return NOT_FOUND;

    // Locates the record using its byteoffset within the table file
    FILE *data_stream = fopen(data_filename, "r");
    fseek(data_stream, byteoffset, SEEK_SET);

    // After locating record, print values to console and exit function call
    data record = fread_record(data_stream, is_fixed);
    printf_record(record);
    free_record(record);

    fclose(data_stream);

    return SUCCESS_CODE;
}


/*
 * Inserts multiple records into data file, as well as into an index file (b-tree).
 * New records with variable size are inserted according to Worst Fit strategy into
 * previously removed record spaces, or into the end of the file if no previously removed space is available.
 *
 * Example:
 *      >>> int total_insertions = 2;
 *      >>> insert_into_btree_command(ata_filename, index_filename, total_insertions, is_fixed);
 *      181 2015 11 "ES" "VILA VELHA" "FORD" "KA SEL 1.5 HA"
 *      1001 2020 21 "PA" "ANANINDEUA" "RENAULT" "DUSTER ZEN 16"
 *
 * Args:
 *      FILE *data_filename: Table file containing records information, with fixed or variable sized records
 *      FILE *index_filename: Index file containing B-Tree nodes with a key type identifier
 *      int total_insertions: Amount of records to read from console and to insert into the table and index files
 *      bool is_fixed: Expected encoding for selected file, can be either FIXED (1) or VARIABLE (0)
 *
 * Returns:
 *      SUCCESS_CODE (1) if table and index could be opened, and ERROR_CODE (-1) otherwise
 */
int insert_into_btree_command(char *data_filename, char *index_filename, int total_insertions, bool is_fixed) {
    
    // Testing the existance of both files, and expecting valid status for both
    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    // Open table and index stream and update status as currently being written to
    FILE *data_stream = fopen(data_filename, "rb+");
    FILE *index_stream = fopen(index_filename, "rb+");

    update_status(data_stream, BAD_STATUS);
    update_status(index_stream, BAD_STATUS);

    // Load both table's and indexe's headers to memory
    header file_header = fread_header(data_stream, is_fixed);
    tree_header index_header = fread_tree_header(index_stream, is_fixed);

    // Reading functionality's 11 entry format
    for (int i = 0; i < total_insertions; i++) {
        data to_insert = read_record_entry(is_fixed);

        // Inserting record into data file, returning RRN or byteoffset (depending on filetype - `is_fixed`)
        // in order to indexate it in the tree.
        long int new_record_position = data_insert_into(data_stream, to_insert, is_fixed, &file_header);

        // Getting indexation info of record recently inserted on data file
        key to_indexate = {};
        to_indexate.id = to_insert.id;

        if (is_fixed) {
            to_indexate.rrn = (int) new_record_position;
        }

        else {
            to_indexate.byteoffset = new_record_position;
        }

        // Insert correctly the new record's info on index by applicating driver procedure
        // (a routine that deals with the insertion properly by considerating the tree's root state).
        driver_procedure(index_stream, &index_header, is_fixed, to_indexate);

        free_record(to_insert);
    }

    // Writing header on the data file to disk
    write_header(data_stream, file_header, is_fixed, 0);

    // To finish writing on the data file, set status as OK_STATUS
    update_status(data_stream, OK_STATUS);
    fclose(data_stream);

    // Writes index header stored in RAM back to index file,
    // thus greatly optimizing previous index operations speeds as
    // writing back to index file is performed only after all insertions are made
    write_tree_header(index_stream, index_header, true, is_fixed);
    update_status(index_stream, OK_STATUS);
    fclose(index_stream);
    
    return SUCCESS_CODE;
}


/*
 * Creates an index table with index_filename, based on records read from input data file, with specified file encoding.
 * Indexes are created as pairs of id's and rrn's or byteoffset's, depending on the input record type.
 *
 * Args:
 *      FILE *data_filename: Table file containing records information, with fixed or variable sized records
 *      FILE *index_filename: Name of the index file to be created
 *      bool is_fixed: Expected encoding for selected and output file, can be either FIXED (1) or VARIABLE (0)
 *
 * Returns:
 *      SUCCESS_CODE (1) if table file could be opened, and ERROR_CODE (-1) otherwise
 */
int create_btree_index_command(char *data_filename, char *index_filename, bool is_fixed) {

    // Verify integrity and existance of only the data files,
    // by marking the verify_index field as false
    if (verify_stream(data_filename, index_filename, false) == ERROR_CODE)
        return ERROR_CODE;

    // Open data file for reading
    FILE *original_stream = fopen(data_filename, "rb");

    // Open index file for writing, and update status field to
    // account for any possible file corruptions
    FILE *index_stream = fopen(index_filename, "wb+");
    update_status(index_stream, BAD_STATUS);

    create_tree_index(original_stream, index_stream, is_fixed);
    fclose(original_stream);

    // After performing operations, update status field
    // and close index file
    update_status(index_stream, OK_STATUS);
    fclose(index_stream);

    return SUCCESS_CODE;
}