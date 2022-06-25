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
int select_rrn_command(char *bin_filename, int rrn);


int create_index_command(char *data_filename, char *index_filename, bool is_fixed);


int delete_records_command(char *data_filename, char *index_filename, int total_filters, bool is_fixed);

#endif //COMMANDS_H