#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#include "../lib/utils.h"
#include "../lib/record.h"
#include "../lib/table.h"
#include "../lib/index.h"
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
    data template = {.id = EMPTY, .year = EMPTY, .total = EMPTY};
    for (int i = 0; i < total_parameters; i++) {
        char *column = scan_word();
        int fixed;
        char *variable;

        int column_code = lookup(column);
        free(column);

        if (column_code == 0 || column_code == 1 || column_code == 2)
            scanf("%d ", &fixed);
        else {
            variable = scan_word_quoted(); getchar();
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


// TODO: Add error codes
int create_index_command(char *data_filename, char *index_filename, bool is_fixed) {

    FILE *original_file_ptr = fopen(data_filename, "rb");

    if (original_file_ptr == NULL)
        return ERROR_CODE;

    FILE *index_file_ptr = fopen("index.bin", "wb");

    create_index(original_file_ptr, index_file_ptr, is_fixed);
    fclose(original_file_ptr);
    fclose(index_file_ptr);
    binarioNaTela("index.bin");
    return 1;
}


int verify_stream(char *data_filename, char *index_filename) {
    FILE *data_file_ptr = fopen(data_filename, "rb+");

    if (data_file_ptr == NULL)
        return ERROR_CODE;

    FILE *index_file_ptr = fopen(index_filename, "rb+");

    if (index_file_ptr == NULL)
        return ERROR_CODE;

    fclose(data_file_ptr);
    fclose(index_file_ptr);
    return SUCCESS_CODE;
}


int delete_records_command(char *data_filename, char *index_filename, int total_filters, bool is_fixed) {

    if (verify_stream(data_filename, index_filename) == ERROR_CODE)
        return ERROR_CODE;
    FILE *data_file_ptr = fopen(data_filename, "rb+");

    for (; total_filters > 0; total_filters--) {
        fseek(data_file_ptr, 0, SEEK_SET);
        int total_parameters; scanf("%d ", &total_parameters);
        data filter = scanf_filter(total_parameters);

        remove_where(data_file_ptr, index_filename, filter, is_fixed);
        free_record(filter);
    }

    fclose(data_file_ptr);

    return SUCCESS_CODE;
}

// Para registro de tipo 2
int evaluate_record_size(data record, bool is_fixed) {
    if (is_fixed) {
        return FIXED_REG_SIZE;
    }
    
    int size = 22; // Tamanho considerando apenas informacoes de campos de tamanho fixo

    if (strlen(record.city)) {
        size += 5 + strlen(record.city); // Somar 5 para tamanho + codigo do campo variavel
    }

    if (strlen(record.brand)) {
        size += 5 + strlen(record.brand);
    }

    if (strlen(record.model)) {
        size += 5 + strlen(record.model);
    }

    return size;
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
    // Teste dos ponteiros de arquivo e sua validez
    if (verify_stream(data_filename, index_filename) == ERROR_CODE) {

        return ERROR_CODE;
    }

    FILE *data_file_ptr = fopen(data_filename, "rb+");

    // Teste da consistencia do arquivo de dados (ao abrir, estava com BAD_STATUS?)
    if (getc(data_file_ptr) == '0') {
        fclose(data_file_ptr);

        return ERROR_CODE;
    }

    header file_header = fread_header(data_file_ptr, is_fixed);

    // Para escrever no arquivo de dados, configurar status como BAD_STATUS
    update_status(data_file_ptr, BAD_STATUS);

    // TODO: manipular indice; configurar status para 0 para usar rb+
    // FILE *index_file_ptr = fopen(index_filename, "rb+");

    for (int i = 0; i < total_insertions; i++) {
        data curr_insertion = read_record_entry(is_fixed);

        if (is_fixed) {
            int top_rrn = file_header.top;

            if (top_rrn == EMPTY) {
                fseek(data_file_ptr, 0, SEEK_END);
                write_record(data_file_ptr, curr_insertion, is_fixed);

                file_header.next_rrn++;
            }

            else { // Usar abordagem dinamica de reuso de espacos
                // Calculate RRN based on the generic formulae below:
                int byte_offset = top_rrn * 97 + FIXED_HEADER;

                // Posicionar ptr no campo de prox. registro logicamente removido
                fseek(data_file_ptr, byte_offset + 1, SEEK_SET);

                // Atualizar topo da pilha de removidos e sua qtd
                fread(&file_header.top, sizeof(int), 1, data_file_ptr);
                file_header.num_removed--;

                // Posicionar ptr do arquivo de dados no inicio do registro cujo espaco sera reutilizado                
                fseek(data_file_ptr, byte_offset, SEEK_SET);
                write_record(data_file_ptr, curr_insertion, is_fixed);
            }
        }

        else {
            long int top_byte_offset = file_header.big_top;

            if (top_byte_offset == EMPTY) {
                fseek(data_file_ptr, 0, SEEK_END);
                write_record(data_file_ptr, curr_insertion, is_fixed);

                file_header.next_byteoffset = ftell(data_file_ptr);
            }

            else { // Tentar reutilizar espacos de registro log. removido, se couber o novo reg.
                // Posicionar ptr no campo de tamanho do reg. removido.
                fseek(data_file_ptr, top_byte_offset + 1, SEEK_SET);

                int max_reusable_space = 0;

                fread(&max_reusable_space, sizeof(int), 1, data_file_ptr);

                // Se couber no espaco de worst fit, reutilize-o
                if (evaluate_record_size(curr_insertion, is_fixed) <= max_reusable_space) {
                    // Atualizar topo da lista de removidos e sua qtd
                    fread(&file_header.big_top, sizeof(long int), 1, data_file_ptr);
                    file_header.num_removed--;

                    curr_insertion.size = max_reusable_space;

                    // Posicionar ptr do arquivo de dados no inicio do registro cujo espaco sera reutilizado                
                    fseek(data_file_ptr, top_byte_offset, SEEK_SET);
                    write_record(data_file_ptr, curr_insertion, is_fixed);

                    // Preencher espaco que sobrou do registro
                    for (int i = 0; i < max_reusable_space - evaluate_record_size(curr_insertion, is_fixed); i++) {
                        fputc(GARBAGE, data_file_ptr);
                    }
                }

                else {
                    // Nao coube no maior espaco disponivel para reutilizar; inserir no fim.
                    fseek(data_file_ptr, 0, SEEK_END);
                    write_record(data_file_ptr, curr_insertion, is_fixed);

                    file_header.next_byteoffset = ftell(data_file_ptr);
                }
            }
        }

        free_record(curr_insertion);
    }

    // Escrever cabecalho no arquivo em disco
    write_header(data_file_ptr, file_header, is_fixed, 0);

    // Para finalizar a escrita no arquivo de dados, configurar status como OK_STATUS
    update_status(data_file_ptr, OK_STATUS);

    fclose(data_file_ptr);
    // fclose(index_file_ptr);
    
    return SUCCESS_CODE;
}

int update_records_command(char *data_filename, char *index_filename, int total_updates, bool is_fixed) {
    if (verify_stream(data_filename, index_filename) == ERROR_CODE)
        return ERROR_CODE;
    FILE *data_file_ptr = fopen(data_filename, "rb+");
    FILE *index_file_ptr = fopen(index_filename, "rb+");

    fclose(data_file_ptr);
    fclose(index_file_ptr);

    return SUCCESS_CODE;
}
