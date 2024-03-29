#include <stdio.h>
#include <stdbool.h>
#include "record.h"
#include "index/index.h"

#ifndef T1_TABLE_H
#define T1_TABLE_H

#define EMPTY_FILTER -2

typedef struct Header_t {

    char status[1];
    int top;
    long int big_top;

    int next_rrn;
    long int next_byteoffset;
    int num_removed;

} header;


/* Writes empty header information into file to be used as a placeholder for future accesses.
*
* Args:
*     FILE *stream: File stream to write header information to
*     bool is_fixed: File encoding to use as reference (can be either FIXED (1) or VARIABLE (0))
*/
void write_header(FILE *stream, header placeholder, bool is_fixed, bool rewrite);


void update_status(FILE *stream, char STATUS[]);


char read_status(FILE *stream);


/*
 * Reads a single header into a newly created header variable, from within
 * the given stream file, and according to selected encoding type.
 */
header fread_header(FILE *stream, bool is_fixed);


/* Prints to console records read from given file, according to expected file encoding type.
*
* Args:
*     FILE *stream: File stream from which to read records from
*     bool is_fixed: File encoding to use when reading the input stream (can be either FIXED (1) or VARIABLE (0))
*
* Returns:
*     int: Returns SUCCESS_CODE if table could be accessed, and ERROR_CODE otherwise.
*/
int select_table(FILE *stream, bool is_fixed);


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
int select_where(FILE *stream, data filter, header header_template, bool is_fixed);


/*
 * Generic function to remove multiple records from a table, using reference index array,
 * as well as filter record to compare values to.
 * All records to be removed will therefore match at least all inputs given in the filter record.
 */
int remove_where(FILE *stream, index_array *index, data filter, header *template, bool is_fixed);


/*
* Inserts a new record field into the given table, as well as into the array of
* indexes.
* Uses the header top element to find next available space, or insert into end
* of file if no empty space within the removed fields is available.
* Uses a similar organization to what was used in the remove_where functionality, which means
* inserts according to Worse Fit in variable sized tables and First fit in
* fixed sized tables.
*
* Args:
*     FILE *stream: File stream to iterate through and write data in (data file)
*     index_array *index: The index file brought to RAM so it'll be possible to add the new records' id + RRN/byteoffset
*     data new_record: The record whose new data received by input stream will be added to data file
*     bool is_fixed: File encoding to use when reading the input stream (can be either FIXED (1) or VARIABLE (0))
*     header *header_template: The header info that is contained on beginning of data file (to update data file's header).
*/
void insert_into(FILE *stream, index_array *index, data new_record, bool is_fixed, header *template);


/*
* Used only for data files that contain constant sized records. It updates fields of a record present
* in the given table, as well as in the array of indexes, based on search filters. The search filters, as well as the
* new values to be put on the filtered fields, are parameters of the function.
*
* Args:
*     FILE *stream: File stream to iterate through and write data in (data file)
*     index_array *index: The index file brought to RAM so it'll be possible to change the new records' id + RRN/byteoffset
*     data filter: The fields that have been used to search the record to be updated and that have to change on data file
*     data params: The new values to the fields of the record that will change/be updated
*     header *header_template: The header info that is contained on beginning of data file (to update data file's header).
*
* Returns:
*     int: Returns num_updated if all updates could be made on the records filtered, and ERROR_CODE otherwise.
*/
int update_fixed_filtered(FILE *stream, index_array *index, data filter, data params, header *template);


/*
* Used only for data files that contain variable sized records. It updates fields of a record present
* in the given table, as well as in the array of indexes, based on search filters. The search filters, as well as the
* new values to be put on the filtered fields, are parameters of the function.
*
* Args:
*     FILE *stream: File stream to iterate through and write data in (data file)
*     index_array *index: The index file brought to RAM so it'll be possible to change the new records' id + RRN/byteoffset
*     data filter: The fields that have been used to search the record to be updated and that have to change on data file
*     data params: The new values to the fields of the record that will change/be updated
*     header *header_template: The header info that is contained on beginning of data file (to update data file's header).
*
* Returns:
*     int: Returns num_updated if all updates could be made on the records filtered, and ERROR_CODE otherwise.
*/
int update_variable_filtered(FILE *stream, index_array *index, data filter, data params, header *template);

/*
* Inserts a new record field into the given table. It does not deal with the index's info.
* Uses the header top element to find next available space, or insert into end
* of file if no empty space within the removed fields is available.
* Uses a similar organization to what was used in the remove_where functionality, which means
* inserts according to Worse Fit in variable sized tables and First fit in
* fixed sized tables.
*
* Args:
*     FILE *stream: File stream to iterate through and write data in (data file)
*     data new_record: The record whose new data received by input stream will be added to data file
*     bool is_fixed: File encoding to use when reading the input stream (can be either FIXED (1) or VARIABLE (0))
*     header *header_template: The header info that is contained on beginning of data file (to update data file's header).
*
* Returns:
*     long int: Returns new_record_pos. This variable contains either the RRN or the byteoffset of
*     the inserted record, so this information can be used to manipulate the index file anytime after. 
*/
long int data_insert_into(FILE *stream, data new_record, bool is_fixed, header *template);

#endif //T1_TABLE_H
