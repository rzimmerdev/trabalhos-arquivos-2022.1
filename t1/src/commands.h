#include <stdbool.h>

#ifndef COMMANDS_H
#define COMMANDS_H

int create_table_command(char *csv_filename, char *out_filename, bool filetype);

void select_command();

void select_where_command();

void select_id_command();

#endif //COMMANDS_H