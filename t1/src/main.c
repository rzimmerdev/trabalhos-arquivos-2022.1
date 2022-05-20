//----------------------------------------//
// Project developed by Rafael Zimmer     //
// nUsp: 12542612                         //
//                                        //
// Exercise for writing and reading from  //
// files in C, for File Organization      //
// course at Usp.                         //
//----------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/dataframe.h"
#include "../lib/record.h"


int main() {

    int total_fields = 7;
    // Tamanho de cada campo no registro:
    int field_sizes[7] = {1, 1, -1, 1, -1, -1, -1};
    int type_sizes[7] = {4, 4, 1, 4, 1, 1, 1};

    record *template = create_record(total_fields, field_sizes, type_sizes);

    return 0;
}