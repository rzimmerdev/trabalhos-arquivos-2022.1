#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include "../lib/utils.h"
#include "../lib/record.h"
#include "../lib/table.h"
#include "commands.h"
#include "csv_utils.h"

// TODO: Add more comments
// TODO: Reevaluate error commands


int create_table_command(char *csv_filename, char *out_filename, bool is_fixed) {
    /* Creates an abstract table with given input and saves result to a specific file.
     *
     * Args:
     *     char *csv_filename: CSV file to read input data from
     *     char *out_filename: Filename to which to write binary data to
     *     bool is_fixed: Selected specific encoding for resulting file, can be either FIXED (1) or VARIABLE (0)
     *
     * Returns:
     *     int: Returns SUCCESS_CODE if read and write operations were successful, and ERROR_CODE otherwise.
     */
    FILE *csvfile_ptr = fopen(csv_filename, "r");

    // Verify if input and output files can be found
    if (csvfile_ptr == NULL)
        return ERROR_CODE;

    FILE *binfile_ptr = fopen(out_filename, "wb");

    // Function to read data from CSV and write to binary encoded file, given desired encoding type.
    csv_to_bin(csvfile_ptr, binfile_ptr, is_fixed);

    fclose(csvfile_ptr);
    fclose(binfile_ptr);

    return SUCCESS_CODE;
}


int select_command(char *bin_filename, bool is_fixed) {
    /* Prints to console records read from given file, according to expected file encoding type.
     *
     * Args:
     *     char *bin_filename: Binary file which contains table to select from
     *     bool is_fixed: Expected encoding for selected file, can be either FIXED (1) or VARIABLE (0)
     *
     * Returns:
     *     int: Returns SUCCESS_CODE if table could be accessed, and ERROR_CODE otherwise.
     */
    FILE *file_ptr = fopen(bin_filename, "rb");

    if (file_ptr == NULL)
        return ERROR_CODE;

    int status = select_table(file_ptr, is_fixed);

    fclose(file_ptr);

    return status;
}


typedef struct { char *key; int val; } table_dict;


static table_dict column_lookup[] = {
        { "id", 0 }, { "ano", 1 }, { "qtt", 2 }, { "sigla", 3 },
        { "cidade", 4}, { "marca", 5}, { "modelo", 6}
};


int lookup(char *key) {
    /* Function to encode column string value to specific numeric value, as to perform conditional operations.
     *
     * Args:
     *     char *key: Column name to be used as key from _table_dict_ column_lookup dictionary.
     *
     * Returns:
     *     int: Numeric value corresponding to encoded column name, or -1 if encoding doesn't exist for given value.
     */
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


int select_where_command(char *bin_filename, int total_parameters, bool is_fixed) {
    /* Prints to console all records read from given file that match all parameters read from console.
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
    FILE *file_ptr = fopen(bin_filename, "rb");

    if (file_ptr == NULL )
        return ERROR_CODE;

    header file_header = fread_header(file_ptr, is_fixed);

    if (file_header.status[0] == BAD_STATUS[0])
        return ERROR_CODE;

    // Read template filter from console, to be able to compare such filter with each
    // record in given file.
    data template = scanf_filter(total_parameters);

    int status = select_where(file_ptr, template, file_header, is_fixed);

    free_record(template);
    fclose(file_ptr);

    return status;
}


int select_rrn_command(char *bin_filename, int rrn) {
    /* Prints to console a single record inside selected fixed record encoded file with given relative record number.
     *
     * Args:
     *     char *bin_filename: Binary file which contains table to select specific record from
     *     int rrn: Relative number refering to position of record in file encoded with fixed records
     *
     * Returns:
     *      int: Returns SUCCESS_CODE if table record could be read,
     *           NOT_FOUND if it was read as removed or is non-existent, and ERROR_CODE otherwise
     */
    FILE *file_ptr = fopen(bin_filename, "rb");

    if (file_ptr == NULL)
        return ERROR_CODE;

    header file_header = fread_header(file_ptr, true);

    // Calculate RRN based on the generic formulae below:
    int byte_offset = rrn * 97 + FIXED_HEADER;

    if ((rrn < 0) || (rrn >= file_header.next_rrn)) {
        return NOT_FOUND;
    }

    // Navigate to byte_offset calculated position
    fseek(file_ptr, byte_offset, SEEK_SET);

    data record = fread_record(file_ptr, true);
    fclose(file_ptr);

    if (record.removed != IS_REMOVED) {
        printf_record(record);
        free_record(record);
        return SUCCESS_CODE;
    }
    else
        return NOT_FOUND;
}


int verify_stream(char *data_filename, char *index_filename, bool verify_index) {
    FILE *data_stream = fopen(data_filename, "rb+");

    if (data_stream == NULL)
        return ERROR_CODE;

    FILE *index_stream;
    if (verify_index)
        index_stream = fopen(index_filename, "rb");

    if (verify_index && index_stream == NULL) {
        fclose(data_stream);
        return ERROR_CODE;
    }

    int data_status = (read_status(data_stream) == OK_STATUS[0]);

    int index_status;
    if (verify_index)
        index_status = (read_status(index_stream) == OK_STATUS[0]);

    fclose(data_stream);

    if (verify_index)
        fclose(index_stream);

    if (data_status != SUCCESS_CODE)
        return ERROR_CODE;

    if (verify_index && index_status != SUCCESS_CODE)
        return ERROR_CODE;

    return SUCCESS_CODE;
}


int create_index_command(char *data_filename, char *index_filename, bool is_fixed) {

    if (verify_stream(data_filename, index_filename, false) == ERROR_CODE)
        return ERROR_CODE;
    FILE *original_file_ptr = fopen(data_filename, "rb");
    FILE *index_file_ptr = fopen(index_filename, "wb");

    create_index(original_file_ptr, index_file_ptr, is_fixed);
    fclose(original_file_ptr);
    fclose(index_file_ptr);
    return SUCCESS_CODE;
}

int delete_records_command(char *data_filename, char *index_filename, int total_filters, bool is_fixed) {

    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    FILE *data_file_ptr = fopen(data_filename, "rb+");

    // Bring index to RAM
    index_array index = index_to_array(index_filename, is_fixed);

    for (; total_filters > 0; total_filters--) {
        int total_parameters; scanf("%d ", &total_parameters);
        data filter = scanf_filter(total_parameters);

        remove_where(data_file_ptr, &index, filter, is_fixed);
        free_record(filter);
    }

    // Write on index file in disk
    array_to_index(index, is_fixed);

    free_index_array(&index);

    fclose(data_file_ptr);

    return SUCCESS_CODE;
}

data read_record_entry(bool is_fixed) {
    data entry_record = {};

    entry_record.removed = NOT_REMOVED;
    
    entry_record.next = EMPTY;
    entry_record.big_next = EMPTY;

    scanf(" %d", &entry_record.id);

    char *year = scan_quote_string();

    if (strlen(year) > 0) {
        entry_record.year = atoi(year);
    }

    else {
        entry_record.year = EMPTY;
    }

    free(year);

    char *total = scan_quote_string();

    if (strlen(total) > 0) {
        entry_record.total = atoi(total);
    }

    else {
        entry_record.total = EMPTY;
    }

    free(total);

    char *state = scan_quote_string();

    if (strlen(state) > 0) {
        entry_record.state[0] = state[0];
        entry_record.state[1] = state[1];
    }

    else {
        entry_record.state[0] = GARBAGE;
        entry_record.state[1] = GARBAGE;
    }

    free(state);

    entry_record.city = scan_quote_string();

    entry_record.city_size = strlen(entry_record.city);
    entry_record.brand = scan_quote_string();
    entry_record.brand_size = strlen(entry_record.brand);
    entry_record.model = scan_quote_string();
    entry_record.model_size = strlen(entry_record.model);

    entry_record.size = evaluate_record_size(entry_record, is_fixed);

    return entry_record;
}


int insert_records_command(char *data_filename, char *index_filename, int total_insertions, bool is_fixed) {
    // Testing file pointers and their validity. Then, testing the consistency of the file to which they point
    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    FILE *data_file_ptr = fopen(data_filename, "rb+");
    header file_header = fread_header(data_file_ptr, is_fixed);

    // Loading index to RAM
    index_array index = index_to_array(index_filename, is_fixed);

    // Reading functionality's 7 entry format
    for (int i = 0; i < total_insertions; i++) {
        data curr_insertion = read_record_entry(is_fixed);

        insert_into(data_file_ptr, &index, curr_insertion, is_fixed, &file_header);
        free_record(curr_insertion);
    }

    // Writing header on the file to disk
    write_header(data_file_ptr, file_header, is_fixed, 0);

    // To finish writing on the data file, set status as OK_STATUS
    update_status(data_file_ptr, OK_STATUS);

    // Write index on the file to disk
    array_to_index(index, is_fixed);

    fclose(data_file_ptr);
    free_index_array(&index);
    
    return SUCCESS_CODE;
}

int update_records_command(char *data_filename, char *index_filename, int total_updates, bool is_fixed) {
    // Testing file pointers and their validity. Then, testing the consistency of the file to which they point
    if (verify_stream(data_filename, index_filename, true) == ERROR_CODE)
        return ERROR_CODE;

    FILE *data_stream = fopen(data_filename, "rb+");
    header file_header = fread_header(data_stream, is_fixed);

    // To write on the data file, set status as BAD_STATUS
    update_status(data_stream, BAD_STATUS);

    // Load index to RAM
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

    // Write index on the file to disk
    array_to_index(index, is_fixed);

    free_index_array(&index);

    fclose(data_stream);
    
    return SUCCESS_CODE;
}