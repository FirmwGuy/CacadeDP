#include "cdp_record.h"
#include <stdio.h>


enum {
    NAME_TEST_BOOK,
    NAME_TEST_DICT,
    NAME_UNSIGNED
};


bool test_register_val(cdpRecord* reg, unsigned value) {
    unsigned* vread;
    size_t size = 0;
    cdp_record_register_read(reg, 0, (void**)&vread, &size);
    assert(value == *vread && size == sizeof(value));
    return true;
}


bool test_zero_item_ops(cdpRecord* book) {
    assert(cdp_record_is_book_or_dic(book));    
    assert(!cdp_record_top(book, true));
    assert(!cdp_record_by_name(book, 0));
    assert(!cdp_record_by_index(book, 0));
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpNameID)));
    path->length = 1;
    path->capacity = 1;
    path->nameID[0] = 0;
    assert(!cdp_record_by_path(book, path));
    return true;
}


bool test_one_item_ops(cdpRecord* book, cdpRecord* reg) {
    cdpRecord* found = cdp_record_top(book, true);
    assert(found == reg);
    found = cdp_record_by_name(book, reg->metadata.nameID);
    assert(found == reg);
    found = cdp_record_by_index(book, 0);
    assert(found == reg);
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpNameID)));
    path->length = 1;
    path->capacity = 1;
    path->nameID[0] = reg->metadata.nameID;
    found = cdp_record_by_path(book, path);
    assert(found == reg);
    return true;
}


bool print_values(cdpBookEntry* entry, unsigned depth, void* unused) {
    unsigned* this, *prev, *next;
    size_t size = 0;
    cdp_record_register_read(entry->record, 0, (void**)&this, &size);
    cdp_record_register_read(entry->prev,   0, (void**)&prev, &size);
    cdp_record_register_read(entry->next,   0, (void**)&next, &size);
    printf("%u: %d (%d, %d)\n", (unsigned)entry->index, *this, *prev, *next);
    return true;
}


int test_one_item(cdpRecord* book) {    
    // Append, lookups and delete
    test_zero_item_ops(book);
    unsigned value = 1;
    cdpRecord* reg = cdp_record_add_register(book, NAME_UNSIGNED, NAME_UNSIGNED, false, &value, sizeof(value));
    test_register_val(reg, value);
    test_one_item_ops(book, reg);
    cdp_record_delete_register(reg);
    
    // Push, lookups and delete
    test_zero_item_ops(book);
    value = 2;
    reg = cdp_record_push_register(book, NAME_UNSIGNED, NAME_UNSIGNED, false, &value, sizeof(value));
    test_register_val(reg, value);
    test_one_item_ops(book, reg);
    
    // Test sequence
    cdp_record_traverse(book, print_values, NULL);
    cdp_record_delete_register(reg);    
    return 0;
}


int test_collections(int argC, const char** argV) {
    cdp_record_system_initiate();
    
    cdpRecord* book = cdp_record_root_add_book(NAME_TEST_BOOK, CDP_STO_CHD_LINKED_LIST, CDP_STO_CHD_LINKED_LIST);
    test_one_item(book);
    cdp_record_delete(book, 2);     // FixMe: test with maxDepth = 1.
    
    book = cdp_record_root_add_book(NAME_TEST_BOOK, CDP_STO_CHD_ARRAY, CDP_STO_CHD_ARRAY, 8);
    test_one_item(book);
    cdp_record_delete(book, 2);     // FixMe: test with maxDepth = 1.

    book = cdp_record_root_add_book(NAME_TEST_BOOK, CDP_STO_CHD_RED_BLACK_T, CDP_STO_CHD_RED_BLACK_T);
    test_one_item(book);
    cdp_record_delete(book, 2);     // FixMe: test with maxDepth = 1.
    
    cdp_record_system_shutdown();
    return 0;
}



int main(int argC, const char** argV) {
    return test_collections(argC, argV);
}
