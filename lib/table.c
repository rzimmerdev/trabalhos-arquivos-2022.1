#include <string.h>
#include <stdlib.h>

#include "record.h"
#include "table.h"
#include "index.h"
// TODO: Add more comments


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
    // if
    if (is_fixed) {
        fwrite(&placeholder.next_rrn, sizeof(int), 1, stream);
    } else {
        fwrite(&placeholder.next_byteoffset, sizeof(long int), 1, stream);
    }

    fwrite(&placeholder.num_removed, sizeof(int), 1, stream);
}


void update_status(FILE *stream, char STATUS[]) {
    fseek(stream, 0, SEEK_SET);
    fwrite(STATUS, sizeof(char), 1, stream);
}


char read_status(FILE *stream) {
    fseek(stream, 0, SEEK_SET);
    char status;
    fread(&status, sizeof(char), 1, stream);
    return status;
}


header fread_header(FILE *stream, bool is_fixed) {
    // TODO: docstring
    // TODO: Refactor comments
    header placeholder = {};

    fseek(stream, 0, SEEK_SET); // Be sure that header is being written at file beginning

    // Write BAD_STATUS to header, as it is currently being worked on and could therefore be corrupted
    // if program closes unexpectedly.
    fread(placeholder.status, sizeof(char), 1, stream);
    if (is_fixed) {
        fread(&placeholder.top, sizeof(int), 1, stream);
    } else {
        fread(&placeholder.big_top, sizeof(long int), 1, stream);
    }

    fseek(stream, 169, SEEK_CUR);

    // Set next_rrn or next_byteoffset in header to 0, as placeholder header should only be written
    // if
    if (is_fixed) {
        fread(&placeholder.next_rrn, sizeof(int), 1, stream);
    } else {
        fread(&placeholder.next_byteoffset, sizeof(long int), 1, stream);
    }

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


int select_where(FILE *stream, data template, header header_template, bool is_fixed) {
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

        if ((template.year != EMPTY && (record.year == -1 || template.year != record.year)) ||
            (template.id != EMPTY && (record.id == -1 || template.id != record.id)) ||
            (template.total != EMPTY && (record.total == -1 || template.total != record.total))) {

            // Free record and continue iterating otherwise (in which case given filter doesn't match current record).
            free_record(record);
            continue;
        }

        if ((template.city && !record.city) || (template.city && record.city && strcmp(template.city, record.city)) ||
            (template.brand && !record.brand) || (template.brand && record.brand && strcmp(template.brand, record.brand)) ||
            (template.model && !record.model) || (template.model && record.model && strcmp(template.model, record.model))) {

            // Free record and continue iterating otherwise. (in which case given filter doesn't match current record).
            free_record(record);
            continue;
        }

        printf_record(record);
        free_record(record);
        total_found++;
    }

    return total_found > 0 ? SUCCESS_CODE : NOT_FOUND;
}


int verify_record(data record, data filter) {
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


void remove_fixed(FILE *stream, index_array *index, data record, int rrn, header *template) {
    long int offset = rrn * FIXED_REG_SIZE + FIXED_HEADER;

    remove_from_index_array(index, record.id);
    remove_record(stream, offset, &(template->top), true);

    template->top = rrn;
    free_record(record);
}


int remove_fixed_filtered(FILE *stream, index_array *index, data filter, header *template) {
    int num_removed = 0;

    if (filter.id != EMPTY_FILTER) {

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
    else {
        int rrn = 0;
        while (rrn++ < template->next_rrn - 1) {
            long int byteoffset = rrn * FIXED_REG_SIZE + FIXED_HEADER;
            fseek(stream, byteoffset, SEEK_SET);

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
    long int parent_offset = template->big_top;

    if (parent_offset == -1) {
        remove_from_index_array(index, record.id);
        remove_record(stream, byteoffset, &(template->big_top), false);

        template->big_top = byteoffset;
        free_record(record);
        return;
    }

    long int current_offset = parent_offset;
    fseek(stream, parent_offset, SEEK_SET);
    data current = fread_record(stream, false);

    if (current.size < record.size) {
        remove_from_index_array(index, record.id);
        remove_record(stream, byteoffset, &(template->big_top), false);

        template->big_top = byteoffset;
        free_record(record);
        return;
    }

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

    fseek(stream, parent_offset + sizeof(char) + sizeof(int), SEEK_SET);
    fwrite(&byteoffset, sizeof(long int), 1, stream);
    remove_from_index_array(index, record.id);
    remove_record(stream, byteoffset, &current_offset, false);
    free_record(record);
}


int remove_variable_filtered(FILE *stream, index_array *index, data filter, header *template) {
    int num_removed = 0;

    if (filter.id != EMPTY_FILTER) {
        long int byteoffset = find_by_id(*index, filter.id).byteoffset;
        fseek(stream, byteoffset, SEEK_SET);
        data record = fread_record(stream, false);

        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            return ERROR_CODE;

        remove_variable(stream, index, record, byteoffset, template);
        return ++num_removed;
    }
    else {

        long int byteoffset = VARIABLE_HEADER;
        while (byteoffset < template->next_byteoffset - 1) {
            fseek(stream, byteoffset, SEEK_SET);
            data record = fread_record(stream, false);

            int status = verify_record(record, filter);
            if (status == ERROR_CODE) {
                byteoffset += record.size + 5;
                continue;
            }

            remove_variable(stream, index, record, byteoffset, template);
            byteoffset += record.size + 5;
            num_removed++;
        }
    }

    return num_removed;
}


int remove_where(FILE *stream, index_array *index, data filter, bool is_fixed) {
    fseek(stream, 0, SEEK_SET);
    header header_template = fread_header(stream, is_fixed);

    int num_removed = is_fixed ? remove_fixed_filtered(stream, index, filter, &header_template) :
                             remove_variable_filtered(stream, index, filter, &header_template);

    if (num_removed > 0) {
        header_template.num_removed += num_removed;
    }

    header_template.status[0] = OK_STATUS[0];
    write_header(stream, header_template, is_fixed, true);
    return 1;
}

int insert_into(FILE *stream, index_array *index, data new_record, bool is_fixed, header *template) {
    index_node node_to_add = {.id = new_record.id, .rrn = EMPTY, .byteoffset = EMPTY};

    if (is_fixed) {
        // Stack's top - it contains the last logically removed record
        int top_rrn = template->top;

        if (top_rrn == EMPTY) {
            int byteoffset = template->next_rrn * FIXED_REG_SIZE + FIXED_HEADER;
            fseek(stream, byteoffset, SEEK_SET);
            write_record(stream, new_record, is_fixed);

            // Inserted record's RRN
            node_to_add.rrn = template->next_rrn;

            template->next_rrn++;
        }

        // Dynamic approach of space reuse 
        else { 
            int byteoffset = top_rrn * FIXED_REG_SIZE + FIXED_HEADER;

            node_to_add.rrn = top_rrn;

            // Put ptr on 'next logically removed' field
            fseek(stream, byteoffset + 1, SEEK_SET);

            // Update the top of the stack that indicate the removed records and its amount
            fread(&template->top, sizeof(int), 1, stream);
            template->num_removed--;

            // Put data file's ptr on the beginning of the record whose space will be reused 
            fseek(stream, byteoffset, SEEK_SET);
            write_record(stream, new_record, is_fixed);
        }
    }

    else {
        long int top_byte_offset = template->big_top;

        if (top_byte_offset == EMPTY) {
            fseek(stream, template->next_byteoffset, SEEK_SET);

            write_record(stream, new_record, is_fixed);

            // Set with the byteoffset of the last available before update, i.e., 
            // the byteoffset in which begins the record that was just written
            node_to_add.byteoffset = template->next_byteoffset;

            template->next_byteoffset = ftell(stream);
        }

        // Try to reuse logically removed's spaces, if the new record fits in there
        else { 
            // Put ptr on 'removed record's size' field
            fseek(stream, top_byte_offset + sizeof(char), SEEK_SET);

            int max_reusable_space = 0;

            fread(&max_reusable_space, sizeof(int), 1, stream);

            // If new record fits 'worst fit' space, reuse it
            if (evaluate_record_size(new_record, is_fixed) <= max_reusable_space) {
                // Update top of removed's list and its amt
                fread(&template->big_top, sizeof(long int), 1, stream);
                template->num_removed--;

                new_record.size = max_reusable_space;

                // Put data file ptr on beginning of the record whose space will be reused
                fseek(stream, top_byte_offset, SEEK_SET);
                write_record(stream, new_record, is_fixed);

                // Set with byteoffset of the record whose space was reused, i.e., the
                // byteoffset is in the old logically removed's list top

                node_to_add.byteoffset = top_byte_offset;

                // Fill record's remaining space with garbage
                for (int i = 0; i < max_reusable_space - evaluate_record_size(new_record, is_fixed); i++) {
                    fputc(GARBAGE, stream);
                }
            }

            else {
                // It has not fit on the biggest space available to reuse; put record on the end.
                fseek(stream, 0, SEEK_END);
                write_record(stream, new_record, is_fixed);

                node_to_add.byteoffset = template->next_byteoffset;

                template->next_byteoffset = ftell(stream);
            }
        }
    }

    insert_into_index_array(index, node_to_add);
}

void update_record(data *record, data params) {
    record->removed = NOT_REMOVED;

    if (params.id != EMPTY_FILTER) {
        record->id = params.id;
    }

    if (params.year != EMPTY_FILTER) {
        record->year = params.year;
    }

    if (params.total != EMPTY_FILTER) {
        record->total = params.total;
    }

    if (params.state[0] != EMPTY_FILTER) {
        record->state[0] = params.state[0];
        record->state[1] = params.state[1];
    }

    if (params.city != NULL) {
        // Only if the field has changed!
        if (record->city) {
            free(record->city);
        }

        record->city = (char *) malloc((strlen(params.city) + 1) * sizeof(char));
        memcpy(record->city, params.city, strlen(params.city) + 1);
        record->city_size = strlen(record->city);
    }

    if (params.brand != NULL) {
        if (record->brand) {
            free(record->brand);
        }

        record->brand = (char *) malloc((strlen(params.brand) + 1) * sizeof(char));
        memcpy(record->brand, params.brand, strlen(params.brand) + 1);
        record->brand_size = strlen(record->brand);
    }

    if (params.model != NULL) {
        if (record->model) {
            free(record->model);
        }

        record->model = (char *) malloc((strlen(params.model) + 1) * sizeof(char));
        memcpy(record->model, params.model, strlen(params.model) + 1);
        record->model_size = strlen(record->model);
    }
}

void update_variable(FILE *stream, index_array *index, data record, data params, long int byteoffset, header *template) {
    fseek(stream, byteoffset, SEEK_SET);

    // Outdated file, whose data are written in disk
    data record_to_update = fread_record(stream, false);
    int old_id = record_to_update.id;

    // Updating in RAM the data for the record (putting updated data in it now)
    update_record(&record_to_update, params);

    // If fits record's space, 
    if (evaluate_record_size(record_to_update, false) <= record_to_update.size) {
        // Make an update on index array
        if (params.id != EMPTY_FILTER) {
            remove_from_index_array(index, record.id);
            index_node new_node = {};
            new_node.id = params.id;
            new_node.byteoffset = byteoffset;

            insert_into_index_array(index, new_node);
        }

        fseek(stream, byteoffset, SEEK_SET);
        write_record(stream, record_to_update, false);

        // Fill record's remaining space (unused)
        for (int i = 0; i < record_to_update.size - evaluate_record_size(record_to_update, false); i++) {
            fputc(GARBAGE, stream);
        }
    }

    else {
        data filter = {.id = old_id};
        remove_where(stream, index, filter, false);
        insert_into(stream, index, record_to_update, false, template);
    }

    free_record(record_to_update);
}

int update_variable_filtered(FILE *stream, index_array *index, data filter, data params, header *template) {
    int num_updated = 0;

    if (filter.id != EMPTY_FILTER) {
        long int byteoffset = find_by_id(*index, filter.id).byteoffset;
        fseek(stream, byteoffset, SEEK_SET);
        data record = fread_record(stream, false);

        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            return ERROR_CODE;

        update_variable(stream, index, record, params, byteoffset, template);

        free_record(record);

        return ++num_updated;
    }

    else {

        long int byteoffset = VARIABLE_HEADER;
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
    long int offset = rrn * FIXED_REG_SIZE + FIXED_HEADER;

    // Make an update on index array
    if (params.id != EMPTY_FILTER) {
        remove_from_index_array(index, record.id);
        index_node new_node = {};
        new_node.id = params.id;
        new_node.rrn = rrn;

        insert_into_index_array(index, new_node);
    }

    // Make an update on data file
    update_record(&record, params);
    
    fseek(stream, offset, SEEK_SET);
    write_record(stream, record, true);

    free_record(record);
}

int update_fixed_filtered(FILE *stream, index_array *index, data filter, data params, header *template) {
    int num_updated = 0;

    /*
     * Search for records that match filter's criteria. If the field 'id' is filled, search
     * the record on index file;
     * if you don't have 'id' as search's parameter, look out sequentially on data file, 
     * comparing the wanted criteria.
    */
    if (filter.id != EMPTY_FILTER) {

        int rrn = find_by_id(*index, filter.id).rrn;
        if (rrn == ERROR_CODE)
            return ERROR_CODE;

        long int byteoffset = rrn * FIXED_REG_SIZE + FIXED_HEADER;
        fseek(stream, byteoffset, SEEK_SET);

        // Direct access on data file
        data record = fread_record(stream, true);
        int result = verify_record(record, filter);
        if (result == ERROR_CODE)
            return ERROR_CODE;

        /*
         * Change records that match the values contained on params.
         * If we have padronized size records (called here as 'fixed'), then you can
         * simply make the alteration, because it's always suitable for the spot we want to put it.
        */
        update_fixed(stream, index, record, params, rrn, template);

        return ++num_updated;
    }
    else {
        int rrn = 0;
        while (rrn++ < template->next_rrn - 1) {
            long int byteoffset = rrn * FIXED_REG_SIZE + FIXED_HEADER;
            fseek(stream, byteoffset, SEEK_SET);

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