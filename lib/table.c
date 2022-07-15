#include <string.h>
#include <stdlib.h>

#include "record.h"
#include "table.h"
#include "index/index.h"


void write_header(FILE *stream, header placeholder, bool is_fixed, bool rewrite) {
    /* Writes empty header information into file to be used as a placeholder for future accesses.
     *
     * Args:
     *     FILE *stream: File stream to write header information to
     *     bool is_fixed: File encoding to use as reference (can be either FIXED (1) or VARIABLE (0))
     */
    if (rewrite)
        update_status(stream, placeholder.status);
    else
        update_status(stream, BAD_STATUS);

    if (is_fixed) {
        fwrite(&placeholder.top, sizeof(int), 1, stream);
    } else {
        fwrite(&placeholder.big_top, sizeof(long int), 1, stream);
    }

    if (!rewrite) {
        // Iterate through static header descriptions and write each one in order.
        char *default_description[11] = {"LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
                                         "CODIGO IDENTIFICADOR: ", "ANO DE FABRICACAO: ", "QUANTIDADE DE VEICULOS: ",
                                         "ESTADO: ", "0", "NOME DA CIDADE: ", "1", "MARCA DO VEICULO: ",
                                         "2", "MODELO DO VEICULO: "};
        for (int i = 0; i < 11; i++) {
            fwrite(default_description[i], sizeof(char), strlen(default_description[i]), stream);
        }
    }
    else
        fseek(stream, is_fixed ? NEXT_RRN_b : NEXT_BYTEOFFSET_b, SEEK_SET);


    // Set next_rrn or next_byteoffset in header to 0, as placeholder header should only be written
    if (is_fixed) {
        fwrite(&placeholder.next_rrn, sizeof(int), 1, stream);
    } else {
        fwrite(&placeholder.next_byteoffset, sizeof(long int), 1, stream);
    }

    fwrite(&placeholder.num_removed, sizeof(int), 1, stream);
}


void update_status(FILE *stream, char STATUS[]) {
    // Updates the status character inside given file, changing pointer to starting position
    fseek(stream, 0, SEEK_SET);
    fwrite(STATUS, sizeof(char), 1, stream);
}


char read_status(FILE *stream) {
    // Reads the status character from given file, changing pointer to starting position
    fseek(stream, 0, SEEK_SET);
    char status;
    fread(&status, sizeof(char), 1, stream);
    return status;
}


header fread_header(FILE *stream, bool is_fixed) {
    /*
     * Reads a single header into a newly created header variable, from within
     * the given stream file, and according to selected encoding type.
     */
    header placeholder = {};

    fseek(stream, 0, SEEK_SET); // Be sure that header is being written at file beginning

    fread(placeholder.status, sizeof(char), 1, stream);

    // Scan fields refereing to remove and insert operations, such as the next available spaces
    if (is_fixed)
        fread(&placeholder.top, sizeof(int), 1, stream);
    else
        fread(&placeholder.big_top, sizeof(long int), 1, stream);

    // Skip header description fields, as description values are fixed and are
    // not relevant when seeking header information, neither are stored within
    // the header structure variable
    fseek(stream, 169, SEEK_CUR);

    // Read either the next currently available rrn or byteoffset values, depending on the
    // header encoding type
    if (is_fixed)
        fread(&placeholder.next_rrn, sizeof(int), 1, stream);
    else
        fread(&placeholder.next_byteoffset, sizeof(long int), 1, stream);

    fread(&placeholder.num_removed, sizeof(int), 1, stream);

    return placeholder;
}


int select_table(FILE *stream, bool is_fixed) {
    /* Prints to console records read from given file, according to expected file encoding type.
     *
     * Args:
     *     FILE *stream: File stream from which to read records from
     *     bool is_fixed: File encoding to use when reading the input stream (can be either FIXED (1) or VARIABLE (0))
     *
     * Returns:
     *     int: Returns SUCCESS_CODE if table could be accessed and has elements, and ERROR_CODE otherwise.
     */

    // Verify status of table in secondary memory, and return ERROR_CODE if file is marked as corrupt
    // (status = '0', even though file was just opened).
    if (getc(stream) == '0') {
        return ERROR_CODE;
    }

    // Access NEXT_RRN or NEXT_BYTEOFFSET binary value in header
    fseek(stream, is_fixed ? NEXT_RRN_b : NEXT_BYTEOFFSET_b, SEEK_SET);

    int next_rrn;
    long int next_byteoffset;

    if (is_fixed)
        fread(&next_rrn, 4, 1, stream);
    else
        fread(&next_byteoffset, 8, 1, stream);

    // Start iterating through records after skipping header bytes.
    fseek(stream, is_fixed ? FIXED_HEADER : VARIABLE_HEADER, SEEK_SET);

    // While current relative record number is not 0 (for fixed records encoding),
    // or while current byteoffset is not larger than maximum byteoffset allowed,
    // keep reading new records.
    int total_records = 0;
    while ((is_fixed && (next_rrn-- > 0)) || (!is_fixed && ftell(stream) < next_byteoffset)) {

        data scanned = fread_record(stream, is_fixed);

        if (scanned.removed != IS_REMOVED) {
            total_records++;
            printf_record(scanned);
        }

        free_record(scanned);
    }

    if (total_records)
        return SUCCESS_CODE;
    else
        return NOT_FOUND;
}


bool compare_record(data template, data record) {
    // Verify if template filter has any of the following fixed or variable inputs selected,
    // and if it exists, compare it to its counterpart in the current record (if it's non-empty in the first place)

    if ((template.year != EMPTY_FILTER && (template.year != record.year)) ||
        (template.id != EMPTY_FILTER && (template.id != record.id)) ||
        (template.total != EMPTY_FILTER && (template.total != record.total))) {

        // Free record and continue iterating otherwise (in which case given filter doesn't match current record).
        return false;
    }

    if ((template.city && !record.city && strcmp(template.city, "") != 0) || (template.city && record.city && strcmp(template.city, record.city)) ||
        (template.brand && !record.brand && strcmp(template.brand, "") != 0) || (template.brand && record.brand && strcmp(template.brand, record.brand)) ||
        (template.model && !record.model && strcmp(template.model, "") != 0) || (template.model && record.model && strcmp(template.model, record.model)) ||
        (template.state[0] != EMPTY_FILTER && strcmp(template.state, record.state))) {

        // Free record and continue iterating otherwise. (in which case given filter doesn't match current record).
        return false;
    }

    return true;
}


int verify_record(data record, data filter) {
    /*
     * Verifies if record is marked as removed, as well as
     * verifying if both records have matching filtered fields (ignores fields marked as EMPTY_FILTER)
     */
    if (record.removed == IS_REMOVED) {
        free_record(record);
        return ERROR_CODE;
    }

    if (!compare_record(filter, record)) {
        free_record(record);
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}


int select_where(FILE *stream, data filter, header header_template, bool is_fixed) {
    /* Prints to console all records in given file that match specific record template filter.
     *
     * Args:
     *     FILE *stream: File stream to iterate through
     *     data template: Data template to use as filter for _where_ command
     *     header header_template: Use template next_rrn or next_byteoffset fields to find end of table
     *     bool is_fixed: File encoding to use when reading the input stream (can be either FIXED (1) or VARIABLE (0))
     *
     * Returns:
     *     int: Returns SUCCESS_CODE if any record could be read, and NOT_FOUND otherwise.
     */

    // Count total records found that match given filter template.
    int total_found = 0;

    // While current relative record number is not 0 (for fixed records encoding),
    // or while current byteoffset is not larger than maximum byteoffset allowed,
    // keep reading new records.
    while ((is_fixed && (header_template.next_rrn-- > 0)) || (!is_fixed && ftell(stream) < header_template.next_byteoffset)) {

        data record = fread_record(stream, is_fixed);
        // If record is marked as removed, ignore it and continue iterating.

        // Verify if template filter has any of the following fixed or variable inputs selected,
        // and if it exists, compare it to its counterpart in the current record (if it's non-empty in the first place)

        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            continue;

        printf_record(record);
        free_record(record);
        total_found++;
    }

    return total_found > 0 ? SUCCESS_CODE : NOT_FOUND;
}


void remove_fixed(FILE *stream, index_array *index, data record, int rrn, header *template) {
    /*
     * Removes a single record from a file with fixed records encoding,
     * which means it uses the stack (First Fit) method for removal and insertion.
     */
    long int offset = rrn * FIXED_REG_SIZE + FIXED_HEADER;

    remove_from_index_array(index, record.id);
    remove_record(stream, offset, &(template->top), true);

    template->top = rrn;
    free_record(record);
}


int remove_fixed_filtered(FILE *stream, index_array *index, data filter, header *template) {
    /*
     * Removes all records with matching fields to given filter,
     * considering an Worst Fit method within a file with variable sized records
     */
    int num_removed = 0;

    // If filter has an id field, search records by id
    if (filter.id != EMPTY_FILTER) {

        // Go to byteoffset of the found record, or its parent if it has not been found (in which case the
        // comparison function will return an error code, and removal will be skipped).
        int rrn = find_by_id(*index, filter.id).rrn;
        if (rrn == ERROR_CODE)
            return ERROR_CODE;

        long int byteoffset = rrn * FIXED_REG_SIZE + FIXED_HEADER;
        fseek(stream, byteoffset, SEEK_SET);

        data record = fread_record(stream, true);
        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            return ERROR_CODE;

        remove_fixed(stream, index, record, rrn, template);

        return ++num_removed;
    }

    // If record doesn't have an id field, iterate over all records within file,
    // and compare to decide whether to remove them or not
    else {
        int rrn = 0;
        while (rrn++ < template->next_rrn - 1) {
            long int byteoffset = rrn * FIXED_REG_SIZE + FIXED_HEADER;
            fseek(stream, byteoffset, SEEK_SET);

            // Compares each record with the filter to decide whether to remove it or not
            data record = fread_record(stream, true);
            int status = verify_record(record, filter);
            if (status == ERROR_CODE)
                continue;

            remove_fixed(stream, index, record, rrn, template);
            num_removed++;
        }

        return num_removed;
    }
}


void remove_variable(FILE *stream, index_array *index, data record, long int byteoffset, header *template) {
    /*
     * Removes a single record from a file with variable records encoding,
     * which means it uses the stack (Worst Fit) method for removal and insertion.
     */
    long int parent_offset = template->big_top;

    // Start removal process by verifying if ordered list has a starting point,
    // if it doesn't, simply insert the removed node offset at the top
    if (parent_offset == -1) {
        remove_from_index_array(index, record.id);
        remove_record(stream, byteoffset, &(template->big_top), false);

        template->big_top = byteoffset;
        free_record(record);
        return;
    }

    // If there is already a list, get its first deleted record values
    long int current_offset = parent_offset;
    fseek(stream, parent_offset, SEEK_SET);
    data current = fread_record(stream, false);

    // If the current record to be removed has greater size when compared to the preexisting top
    // removed element, set the top to the newly removed record, and reconnect list
    if (current.size <= record.size) {
        remove_from_index_array(index, record.id);
        remove_record(stream, byteoffset, &(template->big_top), false);
        template->big_top = byteoffset;
        free_record(record);
        free_record(current);
        return;
    }

    // Else, keep iterating through the list until the end is found or the next record has a smaller size,
    // then insert the newly removed record into the current position, and reconnect list
    free_record(current);
    while (current_offset != -1) {
        fseek(stream, current_offset, SEEK_SET);
        current = fread_record(stream, false);

        if (current.size <= record.size) {
            free_record(current);
            break;
        }
        parent_offset = current_offset;
        current_offset = current.big_next;
        free_record(current);
    }

    // Reconnecting ordered linked list, and removing record
    fseek(stream, parent_offset + sizeof(char) + sizeof(int), SEEK_SET);
    fwrite(&byteoffset, sizeof(long int), 1, stream);
    remove_from_index_array(index, record.id);
    remove_record(stream, byteoffset, &current_offset, false);
    free_record(record);
    return;

}


int remove_variable_filtered(FILE *stream, index_array *index, data filter, header *template) {
    /*
     * Removes all records with matching fields to given filter,
     * considering an Worst Fit method within a file with variable sized records
     */
    int num_removed = 0;

    // If filter has an id field, search records by id
    if (filter.id != EMPTY_FILTER) {

        // Go to byteoffset of the found record, or its parent if it has not been found (in which case the
        // comparison function will return an error code, and removal will be skipped).
        long int byteoffset = find_by_id(*index, filter.id).byteoffset;
        fseek(stream, byteoffset, SEEK_SET);
        data record = fread_record(stream, false);

        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            return ERROR_CODE;

        remove_variable(stream, index, record, byteoffset, template);
        return ++num_removed;
    }

    // If record doesn't have an id field, iterate over all records within file,
    // and compare to decide whether to remove them or not
    else {

        // While file pointer is not at end, keep reading the next record
        while (ftell(stream) < template->next_byteoffset - 1) {
            long int record_byteoffset = ftell(stream), after_byteoffset;

            data record = fread_record(stream, false);
            after_byteoffset = ftell(stream);

            // Compares each record with the filter to decide whether to remove it or not
            int status = verify_record(record, filter);
            if (status == ERROR_CODE)
                continue;

            remove_variable(stream, index, record, record_byteoffset, template);
            fseek(stream, after_byteoffset, SEEK_SET);
            num_removed++;
        }
    }

    return num_removed;
}


int remove_where(FILE *stream, index_array *index, data filter, header *template, bool is_fixed) {
    /*
     * Generic function to remove multiple records from a table, using reference index array,
     * as well as filter record to compare values to.
     * All records to be removed will therefore match at least all inputs given in the filter record.
     */

    // Decide which type of removal to perform, be it using Worst Fit linked list for variable sized records
    // or first fit for fixed size records (stack).
    int num_removed = is_fixed ? remove_fixed_filtered(stream, index, filter, template) :
                             remove_variable_filtered(stream, index, filter, template);

    // Count total removed records and return status accordingly.
    if (num_removed > 0) {
        template->num_removed += num_removed;
    }

    return template->num_removed > 0 ? SUCCESS_CODE : NOT_REMOVED;
}


void fill_with_garbage(FILE *stream, int max_space, int used_space) {
    // Fills the difference between max and used space remaining within a record with fixed size
    for (int i = 0; i < max_space - used_space; i++) {
        fputc(GARBAGE, stream);
    }
}


void insert_into(FILE *stream, index_array *index, data new_record, bool is_fixed, header *template) {
    /*
     * Create a index node in RAM 'cause we have to insert the primary key and RRN/byteoffset
     * (we'll choose which one bychecking file type next) info into the index file
     */
    index_node node_to_add = {.id = new_record.id, .rrn = EMPTY, .byteoffset = EMPTY};

    if (is_fixed) {
        // Header top corresponds to the removed records stack top
        int top_rrn = template->top;

        // Decide if stack is empty, inserting directly into end of file
        if (top_rrn == EMPTY) {
            // Find available position to write new record at end of data file and write it to disk
            int byteoffset = template->next_rrn * FIXED_REG_SIZE + FIXED_HEADER;
            fseek(stream, byteoffset, SEEK_SET);
            write_record(stream, new_record, is_fixed);

            // New record rrn is equal to header next_rrn field (before update it to match the rrn that is now occupied)
            node_to_add.rrn = template->next_rrn;

            // That RRN is now occupied (inserted now record in the end of data file), so increment it
            template->next_rrn++;
        }

        // Use dynamic approach to insert new record into already available space inside the table
        else {
            // Find available position to write new record where it can be reused
            int byteoffset = top_rrn * FIXED_REG_SIZE + FIXED_HEADER;

            // As all records have the same size, access stack top to retrieve next rrn of available space.
            node_to_add.rrn = top_rrn;

            // Seek to next available offset based on template top field
            fseek(stream, byteoffset + 1, SEEK_SET);

            // Update header field with previously empty record next pointer, as to not
            // lose track of the stack top after the insertion is made.
            fread(&template->top, sizeof(int), 1, stream);

            // Now that the space was reused, the data file has less removed records
            template->num_removed--;

            // Return to start of the empty record field byteoffset, and start writing record to be inserted.
            fseek(stream, -(sizeof(int) + sizeof(char)), SEEK_CUR);
            write_record(stream, new_record, is_fixed);
        }
    }

    // Record type is the variable sized one
    else {
        // Header top corresponds to the removed records list top
        long int top_byte_offset = template->big_top;

        // Decide if list is empty, inserting directly into end of file
        if (top_byte_offset == EMPTY) {
            // Find available byteoffset to write new record at end of data file and write it
            fseek(stream, template->next_byteoffset, SEEK_SET);
            write_record(stream, new_record, is_fixed);

            // New record byteoffset is equal to header next_byteoffset field (before update it to match the byteoffset
            // that is now occupied - the record was already written, and it began in there)
            node_to_add.byteoffset = template->next_byteoffset;

            // That byteoffset is now occupied, so get the next availableby catching data ptr position after writing the
            // whole new record.
            template->next_byteoffset = ftell(stream);
        }

        // Try to reuse logically removed spaces, if the new record size fits at least one of them
        else {
            // Seek to removed record size field (this is the biggest size, 'cause of worst fit's logic, so if the
            // record does not fit this space, it won't fit any other space).
            fseek(stream, top_byte_offset + sizeof(char), SEEK_SET);

            int max_reusable_space = 0;

            fread(&max_reusable_space, sizeof(int), 1, stream);

            // If the new record fits the biggest space to reuse on data file, fill it.
            if (evaluate_record_size(new_record, is_fixed) <= max_reusable_space) {
                // Update top of removed's list with next byteoffset (that contains a removed record) and its amount
                fread(&template->big_top, sizeof(long int), 1, stream);

                // Because a space was reused
                template->num_removed--;

                new_record.size = max_reusable_space;

                // Seek to beginning of record whose space will be reused on data file and write the new record's data
                // to disk
                fseek(stream, top_byte_offset, SEEK_SET);
                write_record(stream, new_record, is_fixed);

                // Set the byteoffset to be added into index file with the byteoffset of the record whose space was
                // reused, i.e., the byteoffset that is on top of the old list of logically removed records
                node_to_add.byteoffset = top_byte_offset;

                // Fill the space that the record still has with GARBAGE
                fill_with_garbage(stream, max_reusable_space, evaluate_record_size(new_record, is_fixed));
            }

            // The new record wasn't suitable even to the biggest available space to reuse; insert it into the end of
            // data file then.
            else {
                fseek(stream, 0, SEEK_END);
                write_record(stream, new_record, is_fixed);

                // Use the next byteoffset that is available to write
                node_to_add.byteoffset = template->next_byteoffset;

                // Update next byteoffset available to write new records
                template->next_byteoffset = ftell(stream);
            }
        }
    }

    // Update index data on RAM (index in array format)
    insert_into_index_array(index, node_to_add);
}

void update_field_fixed(int *destiny, int origin) {
    // Function to update fixed sized field, if
    // field is not an empty_filter on the desired parameter record
    // from which to update value from.
    if (origin != EMPTY_FILTER)
        *destiny = origin;
}


void update_field_variable(char **destiny, char *origin, int *size) {
    // Function to update variable sized field, if
    // field is not an empty_filter on the desired parameter record
    // from which to update value from.
    if (origin != NULL) {
        if (!*destiny) {
            *destiny = malloc(sizeof(char));
        }

        *destiny = realloc(*destiny, (strlen(origin) + 1) * sizeof(char));
        memcpy(*destiny, origin, strlen(origin) + 1);
        *size = strlen(*destiny);
    }
}


void update_record(data *record, data params) {
    /*
     * Function to update record data, based on a sequence of fields from
     * given parameter record.
     */
    record->removed = NOT_REMOVED;

    // First, update all fixed sized fields, if they do exist on the
    // params data variable
    update_field_fixed(&record->id, params.id);
    update_field_fixed(&record->year, params.year);
    update_field_fixed(&record->total, params.total);

    if (params.state[0] != EMPTY_FILTER) {
        record->state[0] = params.state[0];
        record->state[1] = params.state[1];
    }

    // Then, update all variable sized fields following the same requirement
    update_field_variable(&record->city, params.city, &record->city_size);
    update_field_variable(&record->brand, params.brand, &record->brand_size);
    update_field_variable(&record->model, params.model, &record->model_size);
}


void update_variable(FILE *stream, index_array *index, data record, data params, long int byteoffset, header *template) {
    /*
     * Updates variable sized records that match the values present in `params`.
     * Internal function called in update_variable_filtered.
     */
    fseek(stream, byteoffset, SEEK_SET);

    // Outdated record whose data are written on disk
    data record_to_update = fread_record(stream, false);
    int old_id = record_to_update.id;

    // Updating record data on RAM (putting updated data on record_to_update, based on `params`)
    update_record(&record_to_update, params);

    // If record fits its original space
    if (evaluate_record_size(record_to_update, false) <= record_to_update.size) {
        // Make an update on index array if `id` value is going to change
        if (params.id != EMPTY_FILTER) {
            remove_from_index_array(index, record.id);
            index_node new_node = {};
            new_node.id = params.id;
            new_node.byteoffset = byteoffset;

            insert_into_index_array(index, new_node);
        }

        // Write into data file the updated record
        fseek(stream, byteoffset, SEEK_SET);
        write_record(stream, record_to_update, false);

        // Put GARBAGE into not filled record space
        fill_with_garbage(stream, record_to_update.size, evaluate_record_size(record_to_update, false));
    }

    else {
        // If the updated record does not fit its original space, insert it into the end of data
        // file. In order to do this, it's necessary to get the updated record's size so it
        // can be inserted into data file.
        record_to_update.size = evaluate_record_size(record_to_update, false);
        data filter = {.id = old_id, .year = EMPTY_FILTER, .total = EMPTY_FILTER, .state = EMPTY_FILTER,
                .city = NULL, .model = NULL, .brand = NULL};

        // Delete the record of its original space. (call func. 6)
        remove_where(stream, index, filter, template, false);

        fseek(stream, 0, SEEK_SET);

        // Re-insert updated record into data file (call func. 7)
        insert_into(stream, index, record_to_update, false, template);

        // Write updated header into data file on disk
        fseek(stream, 0, SEEK_SET);
    }

    free_record(record_to_update);
}

int update_variable_filtered(FILE *stream, index_array *index, data filter, data params, header *template) {
    /*
     * Look for records that match the criteria contained in `filter`. If the `id`
     * field is filled, search for record in index file; otherwise (if `id` is not
     * search parameter), search sequentially on data file, comparing criteria.
    */
    int num_updated = 0;

    // If filter has an id field, find record with matching ids using the index table
    if (filter.id != EMPTY_FILTER) {

        // Get byteoffset of any existing records that match the filter id,
        // or an empty record if no records exist with given id
        long int byteoffset = find_by_id(*index, filter.id).byteoffset;
        fseek(stream, byteoffset, SEEK_SET);
        data record = fread_record(stream, false);

        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            return ERROR_CODE;

        // If record with id exists and with matching filtered values, update record with parameters
        update_variable(stream, index, record, params, byteoffset, template);
        free_record(record);

        return ++num_updated;
    }

    // If filter doesn't have an id field, iterate over all records and test their values
    // against the filter values, deciding if record should be updated one by one
    else {

        long int byteoffset = VARIABLE_HEADER;

        // While file is not at end (current byteoffset is less than next_byteoffset available)
        while (byteoffset < template->next_byteoffset - 1) {
            fseek(stream, byteoffset, SEEK_SET);
            data record = fread_record(stream, false);

            int status = verify_record(record, filter);
            if (status == ERROR_CODE) {
                byteoffset += record.size + 5;
                continue;
            }

            update_variable(stream, index, record, params, byteoffset, template);
            byteoffset += record.size + 5;
            num_updated++;

            free_record(record);
        }
    }

    return num_updated;
}


void update_fixed(FILE *stream, index_array *index, data record, data params, int rrn, header *template) {
    // Updates constant sized records that match the values present in `params`. Intern function called
    // in update_fixed_filtered.

    // Find record to update
    long int offset = rrn * FIXED_REG_SIZE + FIXED_HEADER;

    // Make an update on index array (index file brought to RAM) if `id` will have a new value
    if (params.id != EMPTY_FILTER) {
        remove_from_index_array(index, record.id);
        index_node new_node = {};
        new_node.id = params.id;
        new_node.rrn = rrn;

        insert_into_index_array(index, new_node);
    }

    // Attribute new values' fields (params) into a record format
    update_record(&record, params);

    // Make an update to data file on disk
    fseek(stream, offset, SEEK_SET);
    write_record(stream, record, true);

    free_record(record);
}


int update_fixed_filtered(FILE *stream, index_array *index, data filter, data params, header *template) {
    /*
     * Look for records that match the criteria contained in `filter`. If the `id`
     * field is filled, search for record in index file; otherwise (if `id` is not
     * search parameter), search sequentially on data file, comparing criteria.
    */
    int num_updated = 0;

    if (filter.id != EMPTY_FILTER) {
        // Look into the index array (index file sent to RAM so it can be manipulated
        // several times - the num_updated amount of times)
        int rrn = find_by_id(*index, filter.id).rrn;
        if (rrn == ERROR_CODE)
            return ERROR_CODE;

        // The record was found, so seek to it
        long int byteoffset = rrn * FIXED_REG_SIZE + FIXED_HEADER;
        fseek(stream, byteoffset, SEEK_SET);

        // Direct access into data file
        data record = fread_record(stream, true);
        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            return ERROR_CODE;

        // Update record because it matches params' values. Since we're dealing with
        // constant sized records, it's enough to just make the alteration - it always fits
        // the space; it's not necessary to test it.
        update_fixed(stream, index, record, params, rrn, template);

        // Increment it 'cause a new record was just updated! Then, return.
        return ++num_updated;
    }

    // The `id` is not a field present in filter. Look into the data file for the records to be updated.
    else {

        // Iterate through the records that exist.
        int rrn = 0;
        while (rrn++ < template->next_rrn - 1) {
            long int byteoffset = rrn * FIXED_REG_SIZE + FIXED_HEADER;
            fseek(stream, byteoffset, SEEK_SET);

            // Read record information and compare it with filter,
            // deciding whether to update it or not
            data record = fread_record(stream, true);
            int status = verify_record(record, filter);
            if (status == ERROR_CODE)
                continue;

            update_fixed(stream, index, record, params, rrn, template);
            num_updated++;
        }

        return num_updated;
    }
}

// Deals only with datafile insertion. However, the function return will be used by the index file
// (it contains either the RRN or the byteoffset of the recently inserted record, so it can be indexated).
long int data_insert_into(FILE *stream, data new_record, bool is_fixed, header *template) {
    // Keeps RRN or byteoffset of new record to be inserted into data file
    long int new_record_pos = -1;
    
    if (is_fixed) {
        // Header top corresponds to the removed records stack top
        int top_rrn = template->top;

        // Decide if stack is empty, inserting directly into end of file
        if (top_rrn == EMPTY) {
            // Find available position to write new record at end of data file and write it to disk
            int byteoffset = template->next_rrn * FIXED_REG_SIZE + FIXED_HEADER;
            fseek(stream, byteoffset, SEEK_SET);
            write_record(stream, new_record, is_fixed);

            // New record rrn is equal to header next_rrn field (before update it to match the rrn that is now occupied)
            new_record_pos = template->next_rrn;

            // That RRN is now occupied (inserted now record in the end of data file), so increment it
            template->next_rrn++;
        }

        // Use dynamic approach to insert new record into already available space inside the table
        else {
            // Find available position to write new record where it can be reused
            int byteoffset = top_rrn * FIXED_REG_SIZE + FIXED_HEADER;

            // As all records have the same size, access stack top to retrieve next rrn of available space.
            new_record_pos = top_rrn;

            // Seek to next available offset based on template top field
            fseek(stream, byteoffset + 1, SEEK_SET);

            // Update header field with previously empty record next pointer, as to not
            // lose track of the stack top after the insertion is made.
            fread(&template->top, sizeof(int), 1, stream);

            // Now that the space was reused, the data file has less removed records
            template->num_removed--;

            // Return to start of the empty record field byteoffset, and start writing record to be inserted.
            fseek(stream, -(sizeof(int) + sizeof(char)), SEEK_CUR);
            write_record(stream, new_record, is_fixed);
        }
    }

    // Record type is the variable sized one
    else {
        // Header top corresponds to the removed records list top
        long int top_byte_offset = template->big_top;

        // Decide if list is empty, inserting directly into end of file
        if (top_byte_offset == EMPTY) {
            // Find available byteoffset to write new record at end of data file and write it
            fseek(stream, template->next_byteoffset, SEEK_SET);
            write_record(stream, new_record, is_fixed);

            // New record byteoffset is equal to header next_byteoffset field (before update it to match the byteoffset
            // that is now occupied - the record was already written, and it began in there)
            new_record_pos = template->next_byteoffset;

            // That byteoffset is now occupied, so get the next available by catching data ptr position after writing the
            // whole new record.
            template->next_byteoffset = ftell(stream);
        }

        // Try to reuse logically removed spaces, if the new record size fits at least one of them
        else {
            // Seek to removed record size field (this is the biggest size, 'cause of worst fit's logic, so if the
            // record does not fit this space, it won't fit any other space).
            fseek(stream, top_byte_offset + sizeof(char), SEEK_SET);

            int max_reusable_space = 0;

            fread(&max_reusable_space, sizeof(int), 1, stream);

            // If the new record fits the biggest space to reuse on data file, fill it.
            if (evaluate_record_size(new_record, is_fixed) <= max_reusable_space) {
                // Update top of removed's list with next byteoffset (that contains a removed record) and its amount
                fread(&template->big_top, sizeof(long int), 1, stream);

                // Because a space was reused
                template->num_removed--;

                new_record.size = max_reusable_space;

                // Seek to beginning of record whose space will be reused on data file and write the new record's data
                // to disk
                fseek(stream, top_byte_offset, SEEK_SET);
                write_record(stream, new_record, is_fixed);

                // Set the byteoffset to be added into index file with the byteoffset of the record whose space was
                // reused, i.e., the byteoffset that is on top of the old list of logically removed records
                new_record_pos = top_byte_offset;

                // Fill the space that the record still has with GARBAGE
                fill_with_garbage(stream, max_reusable_space, evaluate_record_size(new_record, is_fixed));
            }

            // The new record wasn't suitable even to the biggest available space to reuse; insert it into the end of
            // data file then.
            else {
                fseek(stream, 0, SEEK_END);
                write_record(stream, new_record, is_fixed);

                // Use the next byteoffset that is available to write
                new_record_pos = template->next_byteoffset;

                // Update next byteoffset available to write new records
                template->next_byteoffset = ftell(stream);
            }
        }
    }

    return new_record_pos;
}