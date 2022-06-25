#include <stdio.h>
#include <stdbool.h>

#ifndef INDEX_H
#define INDEX_H

void create_index(FILE *origin_stream, FILE *index_stream, bool is_fixed);

#endif //INDEX_H
