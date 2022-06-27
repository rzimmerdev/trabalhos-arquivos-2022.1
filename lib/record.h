#include <stdio.h>
#include <stdbool.h>

#ifndef RECORD_H
#define RECORD_H

#define BAD_STATUS "0"
#define OK_STATUS "1"

// Header constants
#define NEXT_RRN_b 174
#define NEXT_BYTEOFFSET_b 178
#define FIXED_HEADER 182
#define VARIABLE_HEADER 190

// Record constants
#define FIXED_MINIMUM 19
#define VARIABLE_MINIMUM 27
#define FIXED_REG_SIZE 97
#define IS_REMOVED '1'
#define NOT_REMOVED '0'

// Field constants
#define EMPTY -1
#define GARBAGE '$'
#define CITY_CODE '0'
#define BRAND_CODE '1'
#define MODEL_CODE '2'

// Status codes
#define SUCCESS_CODE 1
#define ERROR_CODE -1
#define NOT_FOUND 0


typedef struct Data_t {

    char removed;

    int size;
    long int big_next;

    int next;

    int id;
    int year;
    int total;
    char state[2];

    int city_size;
    char *city;

    int brand_size;
    char *brand;

    int model_size;
    char *model;

} data;


/*
 * Frees all variable sized fields inside record, if allocated.
 *
 * Args:
 *     data record: Record with fields to be freed
 */
void free_record(data record);


/*
 * Prints specific field values inside given header, taking into account whether they
 * are filled or not.
 *
 * Args:
 *     data record: Record from which to read fields from
 */
void printf_record(data record);


/*
 * Sets _size_ to NEXT_RRN value or NEXT_BYTEOFFSET given an input file with header information.
 *
 * Args:
 *     FILE *stream: File stream from which to read header
 *     void *size: int or long int type to write size to.
 *     bool is_fixed: File encoding to use, can be either FIXED (1) or VARIABLE (0).
 */
void get_file_size(FILE *stream, void *size, bool is_fixed);


/*
 * Write given record to specified file stream, using one of two possible encodings:
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
void write_record(FILE *dest, data record, bool is_fixed);


/*
 * Read one record from input file stream, given specific record type encoding.
 *
 * Args:
 *     FILE *stream: File stream to read record information from
 *     bool is_fixed: File encoding to use when reading the record (can be either FIXED (1) or VARIABLE (0))
 */
data fread_record(FILE *stream, bool is_fixed);


/*
 * Function to remove a record from within the file stream, based on its
 * offset and encoding type. Also takes in the next pointer,
 * which should correspond to whatever next empty record already existed on the
 * ~top~ field within the header. Then, write its offset or rrn in the ~next~ field.
 */
void remove_record(FILE *stream, long int record_offset, void *next, bool is_fixed);


/*
 * Reads a record from console based on desired encoding type
 * Saves all read fields and fills remaining fields with empty values
 * Returns final generated record with desired encoding, of data type
 */
data read_record_entry(bool is_fixed);


/*
 * Calculate record size field based on its encoding and data.
 */
int evaluate_record_size(data record, bool is_fixed);

#endif //RECORD_H
