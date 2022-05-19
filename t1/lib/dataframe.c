#import "registry.h"


typedef struct Table_t {

    registry *header;

    char type;
    int total_registries;
    registry *registries;

} table;


table *read_csv() {


}