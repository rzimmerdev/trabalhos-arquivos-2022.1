#include <stdbool.h>

#ifndef COMMANDS_H
#define COMMANDS_H

void create_table_command(char *csv_filename, bool filetype);

void select_command();

void select_where_command();

void select_id_command();

#endif //COMMANDS_H