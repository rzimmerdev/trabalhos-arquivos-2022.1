#include <stdio.h>
#include <stdlib.h>

#include "record_types.h"
#include "../lib/record.h"
#include "../lib/dataframe.h"


void csv_to_record_fixed(FILE *csv, FILE *dest, record *header_template, record *data_template) {

    record *header = read_record(csv, header_template, ',');

}

void csv_to_record_variable() {


}