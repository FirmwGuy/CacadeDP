#include "cdp_record.h"


int test_collections(int argC, const char** argV) {
    cdp_record_system_initiate();
    
    cdpRecord* arrayBook = cdp_record_root_add_book(1, 1, CDP_STO_CHD_ARRAY, 8);
    
    for (unsigned n=0; n<100; n++) {
        cdp_record_add_register(arrayBook, 2, 2, false, &n, sizeof(n));
    }
    
    cdp_record_system_shutdown();
    return 0;
}



int main(int argC, const char** argV) {
    return test_collections(argC, argV);
}
