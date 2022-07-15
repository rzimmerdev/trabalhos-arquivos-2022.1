#include <stdbool.h>
#include <stdio.h>

#ifndef COMMANDS_H
#define COMMANDS_H

// Codes for function return values
#define SUCCESS_CODE 1
#define ERROR_CODE -1
#define NOT_FOUND 0


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
int create_table_command(char *csv_filename, char *out_filename, bool filetype);


/* Prints to console records read from given file, according to expected file encoding type.
*
* Args:
*     char *bin_filename: Binary file which contains table to select from
*     bool is_fixed: Expected encoding for selected file, can be either FIXED (1) or VARIABLE (0)
*
* Returns:
*     int: Returns SUCCESS_CODE if table could be accessed, and ERROR_CODE otherwise.
*/
int select_command(char *bin_filename, bool filetype);


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
int select_where_command(char *bin_filename, int total_parameters, bool is_fixed);


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
int select_rrn_command(char *bin_filename, int rrn);


/*
 * Creates an index table with index filename, based on records read
 * from input data file, with specified file encoding
 * Indexes are created as pairs of id's and rrn's or byteoffset's, depending
 * on the input record type.
 */
int create_index_command(char *data_filename, char *index_filename, bool is_fixed);


/*
 * Deletes multiple records from data file, using a sequence of filters
 * to select which records to be deleted. Index file is used to
 * perform search by index, increasing speed of deletion as index
 * values are stored in primary memory (RAM)
 */
int delete_records_command(char *data_filename, char *index_filename, int total_filters, bool is_fixed);


/*
 * Inserts multiple records into data file, as well as into an index file
 * New records with variable size are inserted according to Worst Fit strategy into
 * previously removed record spaces, or into the end of the file if no previously removed space is available.
 */
int insert_records_command(char *data_filename, char *index_filename, int total_insertions, bool is_fixed);


/*
 * Updates a sequence of records from given console input.
 * Updating method depends on the selected data encoding.
 * If the new record does not fit on the previously existing space (specifically for variable sized records),
 * record is deleted and reinserted with updated parameters to next available space according to Worst Fit method.
 */
int update_records_command(char *data_filename, char *index_filename, int total_updates, bool is_fixed);


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
int select_id_command(char *data_filename, char *index_filename, int id, bool is_fixed);


/*
 * Seeks stream to relative record offset within the given stream file.
 *
 * Example:
 *      >>> seek_node(stream, 2, false);
 *      Seeks to byteoffset 90 within the stream file. (45b node size for fixed index, and 57b for variable)
 *
 * Args:
 *      FILE *stream: Input stream containing B-Tree nodes
 *      int rrn: Relative register number, into which position to seek to
 *      bool is_fixed: Index file encoding type, can be either true for fixed sized data,
 *      or false for variable sized data.
 */
int insert_into_btree_command(char *data_filename, char *index_filename, int total_insertions, bool is_fixed);


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
int create_btree_index_command(char *data_filename, char *index_filename, bool is_fixed);

#endif //COMMANDS_H