#include "cdp_record.h"
#include <stdio.h>


static bool print_values(cdpBookEntry* entry, unsigned depth, void* unused) {
    unsigned* this, *prev, *next;
    size_t size = 0;
    cdp_record_register_read(entry->record, 0, (void**)&this, &size);
    cdp_record_register_read(entry->prev,   0, (void**)&prev, &size);
    cdp_record_register_read(entry->next,   0, (void**)&next, &size);
    printf("%u: %d (%d, %d)\n", (unsigned)entry->index, *this, *prev, *next);
    return true;
}

int test_collections(int argC, const char** argV) {
    cdp_record_system_initiate();
    
    cdpRecord* arrayBook = cdp_record_root_add_book(1, 1, CDP_STO_CHD_ARRAY, 8);
    
    /* Test register operations */
    
    // Test 1 item insertion
    unsigned value = 1;
    cdpRecord* reg = cdp_record_add_register(arrayBook, 2, 2, false, &value, sizeof(value));
    unsigned* vread;
    size_t size = 0;
    cdp_record_register_read(reg, 0, (void**)&vread, &size);
    assert(value == *vread && size == sizeof(value));
    
    // Test 1 item pop
    
    
    
    // Test sequence insertion/traversal
    for (unsigned n=0; n<10; n++) {
        cdp_record_add_register(arrayBook, 2, 2, false, &n, sizeof(n));
    }
    cdp_record_traverse(arrayBook, print_values, NULL);
    
    //for (cdpRecord* reg = cdp_record_top(arrayBook, true))
    
    cdp_record_system_shutdown();
    return 0;
}



int main(int argC, const char** argV) {
    return test_collections(argC, argV);
}
