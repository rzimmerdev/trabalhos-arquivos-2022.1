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
#include "../lib/registry.h"


int main() {

    int total_fields = 7;
    int field_sizes = {1, 1, -1, 1, -1, -1, -1};
    int type_sizes[total_fields] = {8, 8, 1, 8, 1, 1, 1};

    registry *template = create_registry(total_fields, field_sizes, type_sizes);

    return 0;
}