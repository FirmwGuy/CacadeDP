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

#include <stdio.h>      // sprintf()

#include "cdp_record.h"



static void test_records_print(cdpRecord* record, char *sval) {
    if (!record) {
        strcpy(sval, "None");
    } else if (cdp_record_is_book(record)) {
        sprintf(sval, "[%d]", record->metadata.id);
    } else if (cdp_record_is_dictionary(record)) {
        sprintf(sval, "{%d}", record->metadata.id);
    } else if (cdp_record_is_register(record)) {
        unsigned uval;
        cdp_register_read(record, 0, &uval, NULL);
        sprintf(sval, "%u", uval);
    }
}

static bool print_values(cdpBookEntry* entry, unsigned depth, void* unused) {
    assert_not_null(entry->record);
    char this[16], prev[16], next[16];
    test_records_print(entry->record, this);
    test_records_print(entry->prev,   prev);
    test_records_print(entry->next,   next);
    munit_logf(MUNIT_LOG_DEBUG, "(%u):  %s  <%s, %s>\n", (unsigned)entry->position, this, prev, next);
    return true;
}


static void test_records_register_val(cdpRecord* reg, unsigned value) {
    unsigned vread = 0;
    size_t size = 0;
    cdp_register_read(reg, 0, &vread, &size);
    assert_size(size, ==, sizeof(value));
    assert_uint(value, ==, vread);
}


static void test_records_zero_item_ops(cdpRecord* book) {
    assert_true(cdp_record_is_book(book));
    assert_null(cdp_book_last(book));
    assert_null(cdp_book_find_by_name(book, CDP_NAME_VALUE));
    assert_null(cdp_book_find_by_position(book, 0));
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    path->id[0] = 0;
    assert_null(cdp_book_find_by_path(book, path));
    assert_true(cdp_book_traverse(book, print_values, NULL));
}


static void test_records_one_item_ops(cdpRecord* book, cdpRecord* reg) {
    cdpRecord* found = cdp_book_last(book);
    assert_ptr_equal(found, reg);
    found = cdp_book_find_by_name(book, reg->metadata.id);
    assert_ptr_equal(found, reg);
    found = cdp_book_find_by_position(book, 0);
    assert_ptr_equal(found, reg);
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    path->id[0] = reg->metadata.id;
    found = cdp_book_find_by_path(book, path);
    assert_ptr_equal(found, reg);
    assert_true(cdp_book_traverse(book, print_values, NULL));
}


static void test_records_tech_book(unsigned storage) {
    cdpRecord* book  = cdp_book_add_book(cdp_root(), CDP_NAME_TEMP, CDP_TYPE_BOOK, storage, 20);

    /* One item operations */

    // Append, lookups and delete
    test_records_zero_item_ops(book);
    unsigned value = 1;
    cdpRecord* reg = cdp_book_add_uint32(book, CDP_NAME_VALUE, value);
    test_records_register_val(reg, value);
    test_records_one_item_ops(book, reg);
    cdp_register_remove(reg);

    // Push and lookups
    test_records_zero_item_ops(book);
    value = 1;
    reg = cdp_book_prepend_uint32(book, CDP_NAME_VALUE, value);
    test_records_register_val(reg, value);
    test_records_one_item_ops(book, reg);

    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* found;
    unsigned first = 1, last = 1;
    size_t index;

    for (unsigned n = 1; n < 10;  n++) {
        if (cdp_book_children(book) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_register_remove(cdp_book_first(book));
                found = cdp_book_first(book);
                cdp_register_read(found, 0, &first, NULL);
                break;
              case 2:
                cdp_register_remove(cdp_book_last(book));
                found = cdp_book_last(book);
                cdp_register_read(found, 0, &last, NULL);
                break;
            }
        }

        value = n + 1;
        if (munit_rand_uint32() & 1) {
            index = cdp_book_children(book);

            reg = cdp_bool_add_uint32(book, CDP_NAME_VALUE+n, value);
            test_records_register_val(reg, value);

            found = cdp_book_first(book);
            test_records_register_val(found, first);
            found = cdp_book_last(book);
            test_records_register_val(found, value);

            last = value;
        } else {
            index = 0;

            reg = cdp_record_prepend_register(book, CDP_NAME_VALUE+n, CDP_NAME_VALUE+n, false, &value, sizeof(value));
            test_records_register_val(reg, value);

            found = cdp_book_first(book);
            test_records_register_val(found, value);
            found = cdp_book_last(book);
            test_records_register_val(found, last);

            first = value;
        }

        found = cdp_book_find_by_name(book, reg->metadata.id);
        assert_ptr_equal(found, reg);

        found = cdp_book_find_by_position(book, index);
        assert_ptr_equal(found, reg);

        path->id[0] = reg->metadata.id;
        found = cdp_book_find_by_path(book, path);
        assert_ptr_equal(found, reg);

        assert_true(cdp_book_traverse(book, print_values, NULL));
    }

    /* Nested books */

    cdpRecord* chdBook = cdp_record_add_book(book, NAME_TEST_BOOK, NAME_TEST_BOOK, storage, 20);
    reg = cdp_record_prepend_register(chdBook, CDP_NAME_VALUE+30, CDP_NAME_VALUE+30, false, &value, sizeof(value));
    test_records_register_val(reg, value);
    assert_true(cdp_book_deep_traverse(book, 3, print_values, NULL, NULL));

    cdp_record_remove(book, 16);
}


static void test_records_tech_dictionary(unsigned storage) {
    cdpRecord* dict = cdp_root_add_dictionary(NAME_TEST_DICT, storage+1, storage, NULL, NULL, 20);

    /* One item operations */

    // Isert, lookups and delete
    test_records_zero_item_ops(dict);
    unsigned value = 1;
    cdpRecord* reg = cdp_record_add_register(dict, CDP_NAME_VALUE, CDP_NAME_VALUE, false, &value, sizeof(value));
    test_records_register_val(reg, value);
    test_records_one_item_ops(dict, reg);
    cdp_register_remove(reg);

    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* found;
    unsigned vmax = 1, vmin = 1000, name;

    for (unsigned n = 1; n < 10;  n++) {
        if (cdp_book_children(dict) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_register_remove(cdp_record_top(dict, false));
                found = cdp_record_top(dict, false);
                cdp_register_read(found, 0, &vmin, NULL);
                break;
              case 2:
                cdp_register_remove(cdp_record_top(dict, true));
                found = cdp_record_top(dict, true);
                cdp_register_read(found, 0, &vmax, NULL);
                break;
            }
        }

        do {
            value = munit_rand_int_range(1, 1000);
            name = CDP_NAME_VALUE + value;
            found = cdp_book_find_by_name(dict, name);
        } while (found);
        if (value < vmin)   vmin = value;
        if (value > vmax)   vmax = value;

        reg = cdp_record_add_register(dict, name, name, false, &value, sizeof(value));
        test_records_register_val(reg, value);

        found = cdp_book_find_by_name(dict, reg->metadata.id);
        assert_ptr_equal(found, reg);

        found = cdp_record_top(dict, false);
        test_records_register_val(found, vmin);

        found = cdp_book_find_by_position(dict, 0);
        test_records_register_val(found, vmin);

        found = cdp_record_top(dict, true);
        test_records_register_val(found, vmax);

        found = cdp_book_find_by_position(dict, cdp_book_children(dict) - 1);
        test_records_register_val(found, vmax);

        path->id[0] = reg->metadata.id;
        found = cdp_book_find_by_path(dict, path);
        assert_ptr_equal(found, reg);

        assert_true(cdp_book_traverse(dict, print_values, NULL));
    }

    /* Nested books */

    cdpRecord* chdDict = cdp_record_add_dictionary(dict, NAME_TEST_DICT+2000, NAME_TEST_DICT, storage, NULL, NULL, 20);
    reg = cdp_record_add_register(chdDict, CDP_NAME_VALUE, CDP_NAME_VALUE, false, &value, sizeof(value));
    test_records_register_val(reg, value);
    assert_true(cdp_book_deep_traverse(dict, 3, print_values, NULL, NULL));

    cdp_record_remove(dict, 16);
}


static void test_records_tech_sequencing_book(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* bookL = cdp_root_add_book(NAME_TEST_BOOK+1, NAME_TEST_BOOK+1, CDP_STO_CHD_LINKED_LIST);
    cdpRecord* bookA = cdp_root_add_book(NAME_TEST_BOOK+2, NAME_TEST_BOOK+2, CDP_STO_CHD_ARRAY, maxItems);

    cdpRecord* foundL, *foundA;

    for (unsigned n = 0; n < maxItems;  n++) {
        unsigned value = 1 + (munit_rand_uint32() % (maxItems>>1));
        unsigned name = CDP_NAME_VALUE + value;

        if ((foundL = cdp_book_find_by_name(bookL, name))) cdp_register_remove(foundL);
        if ((foundA = cdp_book_find_by_name(bookA, name))) cdp_register_remove(foundA);
        assert((!foundL && !foundA) || (foundL && foundA));

        if (cdp_book_children(bookL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_register_remove(cdp_record_top(bookL, false));
                cdp_register_remove(cdp_record_top(bookA, false));
                break;
              case 2:
                cdp_register_remove(cdp_record_top(bookL, true));
                cdp_register_remove(cdp_record_top(bookA, true));
                break;
            }
        }

        cdp_record_add_register(bookL, name, name, false, &value, sizeof(value));
        cdp_record_add_register(bookA, name, name, false, &value, sizeof(value));

        cdpRecord* recordL = cdp_record_top(bookL, false);
        cdpRecord* recordA = cdp_record_top(bookA, false);

        do {
            assert(recordL && recordA);

            cdp_register_read(recordL, 0, &value, NULL);
            test_records_register_val(recordA, value);

            recordL = cdp_book_find_by_next(bookL, recordL);
            recordA = cdp_book_find_by_next(bookA, recordA);
        } while (recordL);
    }

    cdp_record_remove(bookA, 16);
    cdp_record_remove(bookL, 16);
}


static void test_records_tech_sequencing_dictionary(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* dictL = cdp_root_add_dictionary(NAME_TEST_DICT+1, NAME_TEST_DICT+1, CDP_STO_CHD_LINKED_LIST, NULL, NULL);
    cdpRecord* dictA = cdp_root_add_dictionary(NAME_TEST_DICT+2, NAME_TEST_DICT+2, CDP_STO_CHD_ARRAY, NULL, NULL, maxItems);
    cdpRecord* dictT = cdp_root_add_dictionary(NAME_TEST_DICT+3, NAME_TEST_DICT+3, CDP_STO_CHD_RED_BLACK_T, NULL, NULL);

    cdpRecord* foundL, *foundA, *foundT;

    for (unsigned n = 0; n < maxItems;  n++) {
        unsigned value = 1 + (munit_rand_uint32() % (maxItems>>1));
        unsigned name = CDP_NAME_VALUE + value;

        if ((foundL = cdp_book_find_by_name(dictL, name))) cdp_register_remove(foundL);
        if ((foundA = cdp_book_find_by_name(dictA, name))) cdp_register_remove(foundA);
        if ((foundT = cdp_book_find_by_name(dictT, name))) cdp_register_remove(foundT);
        assert((!foundL && !foundA && !foundT) || (foundL && foundA && foundT));

        if (cdp_book_children(dictL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_register_remove(cdp_record_top(dictL, false));
                cdp_register_remove(cdp_record_top(dictA, false));
                cdp_register_remove(cdp_record_top(dictT, false));
                break;
              case 2:
                cdp_register_remove(cdp_record_top(dictL, true));
                cdp_register_remove(cdp_record_top(dictA, true));
                cdp_register_remove(cdp_record_top(dictT, true));
                break;
            }
        }

        cdp_record_add_register(dictL, name, name, false, &value, sizeof(value));
        cdp_record_add_register(dictA, name, name, false, &value, sizeof(value));
        cdp_record_add_register(dictT, name, name, false, &value, sizeof(value));

        cdpRecord* recordL = cdp_record_top(dictL, false);
        cdpRecord* recordA = cdp_record_top(dictA, false);
        cdpRecord* recordT = cdp_record_top(dictT, false);

        do {
            assert(recordL && recordA && recordT);

            cdp_register_read(recordL, 0, &value, NULL);
            test_records_register_val(recordA, value);
            test_records_register_val(recordT, value);

            recordL = cdp_book_find_by_next(dictL, recordL);
            recordA = cdp_book_find_by_next(dictA, recordA);
            recordT = cdp_book_find_by_next(dictT, recordT);
        } while (recordL);
    }

    cdp_record_remove(dictT, 16);
    cdp_record_remove(dictA, 16);
    cdp_record_remove(dictL, 16);
}


MunitResult test_records(const MunitParameter params[], void* user_data_or_fixture) {
    cdp_record_system_initiate();

    test_records_tech_book(CDP_STO_CHD_LINKED_LIST);
    test_records_tech_book(CDP_STO_CHD_ARRAY);
    test_records_tech_book(CDP_STO_CHD_PACKED_QUEUE);

    test_records_tech_dictionary(CDP_STO_CHD_LINKED_LIST);
    test_records_tech_dictionary(CDP_STO_CHD_ARRAY);
    test_records_tech_dictionary(CDP_STO_CHD_RED_BLACK_T);

    test_records_tech_sequencing_book();
    test_records_tech_sequencing_dictionary();

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
