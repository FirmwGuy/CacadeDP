/*
 *  Copyright (c) 2024 Victor M. Barrientos <firmw.guy@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */


/*  This test program uses Munit, which is MIT licensed. Please see munit.h file
 *  for a complete license information.
 */
#define MUNIT_ENABLE_ASSERT_ALIASES
#include "munit.h"


#include "cdp_record.h"


enum {
    NAME_NONE = 0,
    //
    NAME_TEST_BOOK,
    NAME_TEST_DICT,
    NAME_UNSIGNED
};


bool print_values(cdpBookEntry* entry, unsigned depth, void* unused) {
    unsigned this, prev = 0, next = 0;
    assert_not_null(entry->record); cdp_record_register_read(entry->record, 0, &this, NULL);
    if (entry->prev)                cdp_record_register_read(entry->prev,   0, &prev, NULL);
    if (entry->next)                cdp_record_register_read(entry->next,   0, &next, NULL);
    munit_logf(MUNIT_LOG_DEBUG, "%u: %d (%d, %d)\n", (unsigned)entry->index, this, prev, next);
    return true;
}


void test_records_register_val(cdpRecord* reg, unsigned value) {
    unsigned vread = 0;
    size_t size = 0;
    cdp_record_register_read(reg, 0, &vread, &size);
    assert_size(size, ==, sizeof(value));
    assert_uint(value, ==, vread);
}


void test_records_zero_item_ops(cdpRecord* book) {
    assert_true(cdp_record_is_book_or_dic(book));    
    assert_null(cdp_record_top(book, true));
    assert_null(cdp_record_by_name(book, NAME_UNSIGNED));
    assert_null(cdp_record_by_index(book, 0));
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpNameID)));
    path->length = 1;
    path->capacity = 1;
    path->nameID[0] = 0;
    assert_null(cdp_record_by_path(book, path));
    assert_true(cdp_record_traverse(book, print_values, NULL));
}


void test_records_one_item_ops(cdpRecord* book, cdpRecord* reg) {
    cdpRecord* found = cdp_record_top(book, true);
    assert_ptr_equal(found, reg);
    found = cdp_record_by_name(book, reg->metadata.nameID);
    assert_ptr_equal(found, reg);
    found = cdp_record_by_index(book, 0);
    assert_ptr_equal(found, reg);
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpNameID)));
    path->length = 1;
    path->capacity = 1;
    path->nameID[0] = reg->metadata.nameID;
    found = cdp_record_by_path(book, path);
    assert_ptr_equal(found, reg);
    assert_true(cdp_record_traverse(book, print_values, NULL));
}


void test_records_tech(unsigned storage) {    
    cdpRecord* book = cdp_record_root_add_book(NAME_TEST_BOOK, storage+1, storage);

    /* One item operations */
    
    // Append, lookups and delete
    test_records_zero_item_ops(book);
    unsigned value = 1;
    cdpRecord* reg = cdp_record_add_register(book, NAME_UNSIGNED, NAME_UNSIGNED, false, &value, sizeof(value));
    test_records_register_val(reg, value);
    test_records_one_item_ops(book, reg);
    cdp_record_delete_register(reg);
    
    // Push and lookups
    test_records_zero_item_ops(book);
    value = 1;
    reg = cdp_record_push_register(book, NAME_UNSIGNED, NAME_UNSIGNED, false, &value, sizeof(value));
    test_records_register_val(reg, value);
    test_records_one_item_ops(book, reg);
    
    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpNameID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* last = reg, *first = reg, *found;
    size_t index;

    for (unsigned n = 1; n < 10;  n++) {        
        if (cdp_record_book_or_dic_children(book) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_record_delete_register(first);
                first = cdp_record_top(book, false);
              case 2:
                cdp_record_delete_register(last);
                last = cdp_record_top(book, true);
            }
        }
        
        value = n + 1;
        if (munit_rand_uint32() & 1) {
            index = cdp_record_book_or_dic_children(book);

            reg = cdp_record_add_register(book, NAME_UNSIGNED+n, NAME_UNSIGNED+n, false, &value, sizeof(value));
            test_records_register_val(reg, value);

            found = cdp_record_top(book, false);
            assert_ptr_equal(found, first);
            found = cdp_record_top(book, true);
            assert_ptr_equal(found, reg);
            
            last = reg;
        } else {
            index = 0;
            
            reg = cdp_record_push_register(book, NAME_UNSIGNED+n, NAME_UNSIGNED+n, false, &value, sizeof(value));
            test_records_register_val(reg, value);

            found = cdp_record_top(book, false);
            assert_ptr_equal(found, reg);
            found = cdp_record_top(book, true);
            assert_ptr_equal(found, last);
            
            first = reg;
        }
        
        found = cdp_record_by_name(book, reg->metadata.nameID);
        assert_ptr_equal(found, reg);
        
        found = cdp_record_by_index(book, index);
        assert_ptr_equal(found, reg);
        
        path->nameID[0] = reg->metadata.nameID;
        found = cdp_record_by_path(book, path);
        assert_ptr_equal(found, reg);
        
        assert_true(cdp_record_traverse(book, print_values, NULL));
    }
    
    cdp_record_delete(book, 2);     // FixMe: test with maxDepth = 1.
}


MunitResult test_records(const MunitParameter params[], void* user_data_or_fixture) {
    cdp_record_system_initiate();
    
    test_records_tech(CDP_STO_CHD_LINKED_LIST);
    test_records_tech(CDP_STO_CHD_ARRAY);
    test_records_tech(CDP_STO_CHD_RED_BLACK_T);
        
    cdp_record_system_shutdown();
    return MUNIT_OK;
}




// Munit setup follows...

MunitTest testing[] = {
  { "/records",               // name
    test_records,             // test
    NULL,                     // setup
    NULL,                     // tear_down
    MUNIT_TEST_OPTION_NONE,   // options
    NULL                      // parameters
  },

  {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}  // EOL
};

const MunitSuite munitSuite = {
    "/CascadeDP",             // name
    testing,                  // tests
    NULL,                     // suites
    1,                        // iterations
    MUNIT_SUITE_OPTION_NONE   // options
};

int main(int argC, char* argV[MUNIT_ARRAY_PARAM(argC + 1)]) {
    return munit_suite_main(&munitSuite, NULL, argC, argV);
}
