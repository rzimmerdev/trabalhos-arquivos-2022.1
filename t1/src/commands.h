#include <stdbool.h>
#include <stdio.h>

#ifndef COMMANDS_H
#define COMMANDS_H

int create_table_command(char *csv_filename, char *out_filename, bool filetype);

int select_command(char *bin_filename, bool filetype);

int select_where_command(char *bin_filename, int total_parameters, bool is_fixed);

int select_id_command(char *bin_filename, int rrn);

#endif //COMMANDS_H