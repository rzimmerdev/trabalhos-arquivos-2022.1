#include <stdbool.h>

#ifndef COMMANDS_H
#define COMMANDS_H

int create_table_command(char *csv_filename, char *out_filename, bool filetype);

int select_command(char *bin_filename, bool filetype);

void select_where_command();

int select_id_command(char *bin_filename, int rrn);

#endif //COMMANDS_H