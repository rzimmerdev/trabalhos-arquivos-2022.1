/*
 * Nomes (G1): Danielle Modesti e Rafael Zimmer
 * nUSPs: 12543544 e 12542612
 * Disciplina: Organizacao de Arquivos - semestre (2022.1)
 * Trabalho 3 — frota de veículos no Brasil.
 *
 * Este trabalho tem como objetivo indexar arquivos de dados usando um indice
 * arvore-B e utilizar esse indice para a recuperacao de dados. 
 * 
 * As funcionalidades dos trabalhos 1 e 2 fornecem uma base a partir da qual as do
 * trabalho 3 sao desenvolvidas :)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/utils.h"
#include "commands.h"

// Available op codes to be used for eleven different commands that control data and/or indexate
// it.
typedef enum Command_t {
    CREATE_TABLE = 1,
    SELECT = 2,
    SELECT_WHERE = 3,
    SELECT_ID = 4,
    CREATE_INDEX = 5,
    REMOVE_RECORDS = 6,
    INSERT_RECORDS = 7,
    UPDATE_RECORDS = 8,
    CREATE_BTREE_INDEX = 9,
    SELECT_WHERE_BTREE = 10,
    INSERT_INTO_BTREE = 11
} command;

// Possible filetype encodings to be used when reading or writing binary files:
// records on file have constant size (called FIXED) or it can change and depends 
// on each specific record (called VARIABLE)
typedef enum Filetype_t {
    FIXED = 1,
    VARIABLE = 0
} filetypes;


int main() {
    // Choosing functionality 
    int option, filetype;
    scanf("%d ", &option);

    // Choosing file type (is its size constant/is fixed or can it change/is variable?)
    char *file_type_str = scan_word();
    if (file_type_str[4] == '1') {
        filetype = FIXED;
    }

    else {
        filetype = VARIABLE;
    }

    free(file_type_str);

    switch ((command)option) {
        case CREATE_TABLE: {
            char *csv_filename = scan_word();
            char *out_filename = scan_word();

            // Make data file (the `out` one)
            int status = create_table_command(csv_filename, out_filename, filetype);
            free(csv_filename);

            if (status != ERROR_CODE)
                binarioNaTela(out_filename);
            else
                printf("Falha no processamento do arquivo.");
            free(out_filename);

            break;
        }
        case SELECT: {
            // Specify in which data file you want to recover all records' data (but only show the NOT_REMOVED ones)
            char *bin_filename = scan_word();

            int status = select_command(bin_filename, filetype);
            free(bin_filename);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.");
            break;
        }
        case SELECT_WHERE: {
            // Specify in which data file you want to recover the records' data. Also, get the criteria to show them
            // (display only the ones that match search's parameters)
            char *bin_filename = scan_word();
            int total_parameters; scanf("%d ", &total_parameters);

            int status = select_where_command(bin_filename, total_parameters, filetype);
            free(bin_filename);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            break;
        }
        case SELECT_ID: {
            // Specify in which data file you want to recover the record's info. Also, get its exact spot on binary file
            // by its RRN. Only type 1 files (FIXED ones) can use this command.
            char *bin_filename = scan_word();
            int rrn; scanf("%d", &rrn);

            int status = select_rrn_command(bin_filename, rrn);
            free(bin_filename);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            break;
        }
        case CREATE_INDEX: {
            /* Specify which data file you want to indexate (create a bin. file that will keep record of
             * all records' primary keys (IDs') and RRNs/byteoffsets (used to find them on data file)), so it'll be
             * possible to find them faster.
             * index with id + RRN -> only if working with constant sized's records (FIXED)
             * index with id + byteoffset -> only if working with variable sized's records (VARIABLE)
            */
            char *data_filename = scan_word();
            char *index_filename = scan_word();

            int status = create_index_command(data_filename, index_filename, filetype);

            if (status == SUCCESS_CODE)
                binarioNaTela(index_filename);

            else if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");
            free(data_filename);
            free(index_filename);

            break;
        }
        case REMOVE_RECORDS: {
            /*
             * Specify in which data file you want to delete records. It will also be necessary to get its index file so
             * it'll be possible to update it (removing in there the primary key and the RRN/byteoffset of the deleted 
             * records).
             */
            char *data_filename = scan_word();
            char *index_filename = scan_word();

            int total_filters; scanf("%d ", &total_filters);
            int status = delete_records_command(data_filename, index_filename, total_filters, filetype);

            if (status == SUCCESS_CODE) {
                binarioNaTela(data_filename);
                binarioNaTela(index_filename);
            }
            else if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");
            free(data_filename);
            free(index_filename);

            break;
        }
        case INSERT_RECORDS: {
            /*
             * Specify in which data file you want to insert records. It will also be necessary to get its index file so
             * it'll be possible to update it (adding in there the primary key and the RRN/byteoffset of the new 
             * inserted records).
             */
            char *data_filename = scan_word();
            char *index_filename = scan_word();

            int total_insertions; scanf("%d ", &total_insertions);

            int status = insert_records_command(data_filename, index_filename, total_insertions, filetype);

            if (status != ERROR_CODE) {
                binarioNaTela(data_filename);
                binarioNaTela(index_filename);
            }
            else
                printf("Falha no processamento do arquivo.");
            free(data_filename);
            free(index_filename);

            break;
        }
        case UPDATE_RECORDS: {
            /*
             * Specify in which data file you want to update records. It will also be necessary to get its index file so
             * it'll be possible to update it (sometimes removing & re-adding in there the primary key and the 
             * RRN/byteoffset of the updated records).
             */
            char *data_filename = scan_word();
            char *index_filename = scan_word();

            int total_updates; scanf("%d ", &total_updates);
            int status = update_records_command(data_filename, index_filename, total_updates, filetype);

            if (status != ERROR_CODE) {
                binarioNaTela(data_filename);
                binarioNaTela(index_filename);
            }
            else
                printf("Falha no processamento do arquivo.");
            free(data_filename);
            free(index_filename);

            break;
        }
        case CREATE_BTREE_INDEX: {
            char *data_filename = scan_word();
            char *index_filename = scan_word();

            int status = create_btree_index_command(data_filename, index_filename, filetype);

            if (status != ERROR_CODE) {
                binarioNaTela(index_filename);
            }
            else
                printf("Falha no processamento do arquivo.");
            free(data_filename);
            free(index_filename);

            break;
        }
        case SELECT_WHERE_BTREE: {
            /*
             * Specify in which data file you want to recover the record's data. In order 
             * to do so, get the id field as search criteria. The search will be performed
             * on the index ('cause the record is already indexated), so it'll be possible
             * to access the record's info directly on data file. In the end, the matching
             * record is displayed.
            */
            char *data_filename = scan_word();
            char *index_filename = scan_word();

            free(scan_word());

            int id;
            scanf("%d ", &id);

            int status = select_id_command(data_filename, index_filename, id, filetype);

            if (status == ERROR_CODE)
                printf("Falha no processamento do arquivo.");
            else if (status == NOT_FOUND)
                printf("Registro inexistente.\n");

            free(data_filename);
            free(index_filename);
            break;
        }
        case INSERT_INTO_BTREE: {
            /*
             * Specify in which data file you want to insert records. It will also be necessary to get its index/btree 
             * file so it'll be possible to update it (adding in there the primary key and the RRN/byteoffset of the new 
             * inserted records).
             */
            char *data_filename = scan_word();
            char *index_filename = scan_word();

            int total_insertions; scanf("%d ", &total_insertions);

            int status = insert_into_btree_command(data_filename, index_filename, total_insertions, filetype);

            if (status != ERROR_CODE) {
                binarioNaTela(data_filename);
                binarioNaTela(index_filename);
            }
            else
                printf("Falha no processamento do arquivo.");
            free(data_filename);
            free(index_filename);
            break;
        }
    }

    return EXIT_SUCCESS;
}