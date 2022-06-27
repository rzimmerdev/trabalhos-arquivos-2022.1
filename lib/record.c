#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "record.h"
#include "utils.h"

void free_record(data record) {
    /* Frees all variable sized fields inside record, if allocated.
     *
     * Args:
     *     data record: Record with fields to be freed
     */
    if (record.city)
        free(record.city);
    if (record.model)
        free(record.model);
    if (record.brand)
        free(record.brand);
}


void printf_record(data record) {
    /* Prints specific field values inside given header, taking into account whether they
     * are filled or not.
     *
     * Args:
     *     data record: Record from which to read fields from
     */
    char empty[] = "NAO PREENCHIDO";
    printf("MARCA DO VEICULO: %s\n", record.brand ? record.brand : empty);
    printf("MODELO DO VEICULO: %s\n", record.model ? record.model : empty);
    if (record.year != -1) {
        printf("ANO DE FABRICACAO: %d\n", record.year);
    } else {
        printf("ANO DE FABRICACAO: %s\n", empty);
    }
    printf("NOME DA CIDADE: %s\n", record.city ? record.city : empty);
    printf("QUANTIDADE DE VEICULOS: %d\n", record.total);
    printf("\n");
}

void get_file_size(FILE *stream, void *size, bool is_fixed) {
    /* Sets _size_ to NEXT_RRN value or NEXT_BYTEOFFSET given an input file with header information.
     *
     * Args:
     *     FILE *stream: File stream from which to read header
     *     void *size: int or long int type to write size to.
     *     bool is_fixed: File encoding to use, can be either FIXED (1) or VARIABLE (0).
     */

    // Access either NEXT_RRN or NEXT_BYTEOFFSET binary value inside header.
    if (is_fixed)
        fseek(stream, NEXT_RRN_b, SEEK_SET);
    else
        fseek(stream, NEXT_BYTEOFFSET_b, SEEK_SET);

    fread(size, is_fixed ? sizeof(int) : sizeof(long int), 1, stream);
}


int write_variable_field(FILE *stream, char *value, char code, int size) {
    /*
     * Writes variable sized field to stream, as well as calculating resulting
     * write size based on total characters + fixed size.
     */
    if (value && strlen(value)) {
        fwrite(&size, 4, 1, stream);
        fwrite(&code, 1, 1, stream);
        fwrite(value, 1, size, stream);
        return size + sizeof(int) + sizeof(char);
    }
    return 0;
}


void write_record(FILE *stream, data record, bool is_fixed) {
    /* Write given record to specified file stream, using one of two possible encodings:
     *
     * is_fixed = FIXED    - Fixes maximum record size to 97 bytes, either trucating fields or
     *                       filling remaining spaces with _garbage_
     *
     * is_fixed = VARIABLE - Each record has a variable size, and therefore no size restraints are applied.
     *
     * Args:
     *     FILE *stream: File stream to write record information to
     *     data record: Record variable to access fields from and write them in order
     *     bool is_fixed: File encoding to use when writing record (can be either FIXED (1) or VARIABLE (0))
     */
    char code_city = CITY_CODE, code_brand = BRAND_CODE, code_model = MODEL_CODE;

    // Write the first fixed sized fields
    fwrite(&record.removed, 1, 1, stream);

    // Switch the type of next removed record field,
    // since the fixed sized record uses the rrn value, but the variable sized one uses the byteoffset
    if (is_fixed) {
        fwrite(&record.next, 4, 1, stream);
    } else {
        fwrite(&record.size, 4, 1, stream);
        fwrite(&record.big_next, 8, 1, stream);
    }
    fwrite(&record.id, 4, 1, stream);
    fwrite(&record.year, 4, 1, stream);
    fwrite(&record.total, 4, 1, stream);

    // If state field exists, write it, else write empty characters
    if (strlen(record.state))
        fwrite(record.state, 1, 2, stream);
    else
        fwrite("$$", 1, 2, stream);

    // Write variable sized fields to record, as well as saving used space to write garbage characters
    // at end in case the record is of fixed size.
    int city_space = write_variable_field(stream, record.city, code_city, record.city_size);
    int brand_space = write_variable_field(stream, record.brand, code_brand, record.brand_size);
    int model_space = write_variable_field(stream, record.model, code_model, record.model_size);

    // If record is variable sized, all writing is done and function should return
    if (!is_fixed)
        return;

    // If record has fixed size, after filling its fields, fill remaining space with
    // garbage character, as to match fixed size.
    for (int i = FIXED_MINIMUM + city_space + brand_space + model_space; i < FIXED_REG_SIZE; i++) {
        fputc(GARBAGE, stream);
    }
}

data fread_record(FILE *stream, bool is_fixed) {
    /* Read one record from input file stream, given specific record type encoding.
     *
     * Args:
     *     FILE *stream: File stream to read record information from
     *     bool is_fixed: File encoding to use when reading the record (can be either FIXED (1) or VARIABLE (0))
     */
    data record = {.city_size = 0, .brand_size = 0, .model_size = 0};

    fread(&record.removed, 1, 1, stream);

    // Next, write each field in record according to encoding type in desired order:
    if (is_fixed)
        fread(&record.next, 4, 1, stream);
    else {
        fread(&record.size, 4, 1, stream);
        fread(&record.big_next, 8, 1, stream);
    }

    // Reads fixed and variable sized fields into the record variable
    fread(&record.id, 4, 1, stream);
    fread(&record.year, 4, 1, stream);
    fread(&record.total, 4, 1, stream);

    // Reads the state field and decide whether it is empty or not, in which case
    // process the ~$~ garbage symbol and change it to an empty string.
    fread(record.state, 1, 2, stream);
    if (record.state[0] == GARBAGE) {
        record.state[0] = '\0';
        record.state[1] = '\0';
    }

    // Calculate minimum bytes already read based on the record encoding type
    int bytes_read = is_fixed ? FIXED_MINIMUM : VARIABLE_MINIMUM;

    // Iterate over possibly existing variable sized fields,
    // which are: city, brand, or/and model
    for (int i = 0; i < 3; i++) {
        // Decide whether to write a specific variable-sized field or just skip it in the
        // final file (Field should be skipped if it is variable-sized and is also empty).
        if (is_fixed) {
            char is_empty; fread(&is_empty, 1, 1, stream);

            ungetc(is_empty, stream);
            if (is_empty == GARBAGE)
                break;
        }
        else if (bytes_read >= record.size) {
            break;
        }

        // If variable sized field really exists, read its size and
        // respective field code, as some fields could possibly not exist or be empty
        int current_size;
        char current_code;
        fread(&current_size, 4, 1, stream);
        fread(&current_code, 1, 1, stream);

        // Read one of the three possible variable fields into its respective field inside the struct
        switch (current_code) {
            case CITY_CODE: {
                record.city_size = current_size;
                record.city = malloc(sizeof(char) * current_size + 1);
                fread(record.city, 1, current_size, stream); record.city[current_size] = '\0';
                break;
            }
            case BRAND_CODE: {
                record.brand_size = current_size;
                record.brand = malloc(sizeof(char) * current_size + 1);
                fread(record.brand, 1, current_size, stream); record.brand[current_size] = '\0';
                break;
            }
            case MODEL_CODE: {
                record.model_size = current_size;
                record.model = malloc(sizeof(char) * current_size + 1);
                fread(record.model, 1, current_size, stream); record.model[current_size] = '\0';
                break;
            }
        }

        bytes_read += 5 + current_size;
    }

    // If record is fixed_size, skip garbage characters at end based on number of bytes already read.
    if (is_fixed)
        fseek(stream, FIXED_REG_SIZE - bytes_read, SEEK_CUR);

    return record;
}


void remove_record(FILE *stream, long int record_offset, void *next, bool is_fixed) {
    /*
     * Function to remove a record from within the file stream, based on its
     * offset and encoding type. Also takes in the next pointer,
     * which should correspond to whatever next empty record already existed on the
     * ~top~ field within the header. Then, write its offset or rrn in the ~next~ field.
     */

    // Seek to given record offset start
    fseek(stream, record_offset, SEEK_SET);

    // Mark its ~removed~ field with a 'IS_REMOVED' character ('1')
    char removed = IS_REMOVED;
    fwrite(&removed, sizeof(char), 1, stream);

    // Then, based on record encoding write either the next removed record rrn or its byteoffset.
    if (is_fixed)
        fwrite((int *) next, sizeof(int), 1, stream);
    else {
        fseek(stream, sizeof(int), SEEK_CUR);
        fwrite((long int *) next, sizeof(long int), 1, stream);
    }
}


data read_record_entry(bool is_fixed) {
    /*
     * Reads a record from console based on desired encoding type
     * Saves all read fields and fills remaining fields with empty values
     * Returns final generated record with desired encoding, of data type
     */
    data entry_record = {};

    // Firstly, set fields used for removing records such as the
    // removed status, next_rrn or next_byteoffset to empty
    entry_record.removed = NOT_REMOVED;

    entry_record.next = EMPTY;
    entry_record.big_next = EMPTY;

    // Then, scan fixed sized fields, such as id, year and state
    scanf(" %d", &entry_record.id);

    char *year = scan_quote_string();
    if (strlen(year) > 0)
        entry_record.year = atoi(year);
    else
        entry_record.year = EMPTY;
    free(year);

    char *total = scan_quote_string();
    if (strlen(total) > 0)
        entry_record.total = atoi(total);
    else
        entry_record.total = EMPTY;
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

    // Then, scan variable sized records, such as city, brand and model fields
    entry_record.city = scan_quote_string();
    entry_record.city_size = strlen(entry_record.city);
    entry_record.brand = scan_quote_string();
    entry_record.brand_size = strlen(entry_record.brand);
    entry_record.model = scan_quote_string();
    entry_record.model_size = strlen(entry_record.model);

    // Finally, calculate the resulting record final size and save to the size field
    entry_record.size = evaluate_record_size(entry_record, is_fixed);

    return entry_record;
}

int evaluate_record_size(data record, bool is_fixed) {
    /*
     * Calculate record size field based on its encoding and data.
     */

    // If record encoding is of type fixed, return fixed size.
    if (is_fixed) {
        return FIXED_REG_SIZE;
    }

    // Else, start counting based on minimum variable size minus the status and size fields, which amount to size 5;
    int size = VARIABLE_MINIMUM - sizeof(char) - sizeof(int);

    // Then, increment size based on remaining variable sized fields
    // The size for each individual variable sized field is equivalent to
    // the size of its code + the size of the integer counting its size and the size of all its characters
    if (record.city && strlen(record.city)) {
        size += sizeof(char) + sizeof(int) + strlen(record.city);
    }

    if (record.brand && strlen(record.brand)) {
        size += sizeof(char) + sizeof(int) + strlen(record.brand);
    }

    if (record.model && strlen(record.model)) {
        size += sizeof(char) + sizeof(int) + strlen(record.model);
    }

    return size;
}