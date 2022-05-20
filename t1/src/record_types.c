#include <stdio.h>
#include <stdlib.h>

#include "../lib/record.h"
#include "../lib/dataframe.h"


table *read_csv(FILE *fp) {

    int csv_template[7] = {1, 1, -1, 1, -1, -1, -1};
    int type_template[7] = {4, 4, 1, 4, 1, 1, 1};

    record *template = create_record(7, csv_template, type_template);

    return read_mixed_file(fp, template, ',');
}


void csv_to_record_fixed(FILE *fp, record *header_template, record *data_template) {

    table *dataframe = read_csv(fp);


}

void csv_to_record_variable() {


}