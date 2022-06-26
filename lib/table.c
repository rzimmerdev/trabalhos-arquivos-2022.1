#include <string.h>

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

    if ((template.year != EMPTY && (record.year == -1 || template.year != record.year)) ||
        (template.id != EMPTY && (record.id == -1 || template.id != record.id)) ||
        (template.total != EMPTY && (record.total == -1 || template.total != record.total))) {

            // Free record and continue iterating otherwise (in which case given filter doesn't match current record).
        return false;
    }

    if ((template.city && !record.city) || (template.city && record.city && strcmp(template.city, record.city)) ||
        (template.brand && !record.brand) || (template.brand && record.brand && strcmp(template.brand, record.brand)) ||
        (template.model && !record.model) || (template.model && record.model && strcmp(template.model, record.model)) ||
        (strlen(template.state) == 2 && !record.state) || (strlen(template.state) == 2 && strcmp(template.state, record.state))) {

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

    if (filter.id != -1) {

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

    if (filter.id != -1) {
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


int remove_where(FILE *stream, char *index_filename, data filter, bool is_fixed) {
    fseek(stream, 0, SEEK_SET);
    header header_template = fread_header(stream, is_fixed);

    index_array index = index_to_array(index_filename, is_fixed);

    int num_removed = is_fixed ? remove_fixed_filtered(stream, &index, filter, &header_template) :
                             remove_variable_filtered(stream, &index, filter, &header_template);


    array_to_index(index, is_fixed);
    free_index_array(&index);

    if (num_removed > 0) {
        header_template.num_removed += num_removed;
    }

    header_template.status[0] = OK_STATUS[0];
    write_header(stream, header_template, is_fixed, true);
    return 1;
}