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
    } else if (cdp_record_is_catalog(record)) {
        sprintf(sval, "<%d>", record->metadata.id);
    } else if (cdp_record_is_register(record)) {
        uint32_t uval;
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


static void test_records_register_val(cdpRecord* reg, uint32_t value) {
    uint32_t vread = 0;
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
    assert_true(cdp_book_traverse(book, print_values, NULL, NULL));
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
    assert_true(cdp_book_traverse(book, print_values, NULL, NULL));
}


static void test_records_nested_one_item_ops(cdpRecord* cat, cdpID id, cdpRecord* reg) {
    cdpRecord* book  = cdp_book_last(cat);
    cdpRecord* found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
    assert_ptr_equal(found, reg);
    book  = cdp_book_find_by_name(cat, id);
    found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
    assert_ptr_equal(found, reg);
    book  = cdp_book_find_by_position(cat, 0);
    found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
    assert_ptr_equal(found, reg);
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    path->id[0] = id;
    book  = cdp_book_find_by_path(cat, path);
    found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
    assert_ptr_equal(found, reg);
    assert_true(cdp_book_traverse(book, print_values, NULL, NULL));
}




static void test_records_tech_book(unsigned storage) {
    cdpRecord* book = cdp_book_add_book(cdp_root(), CDP_NAME_TEMP, CDP_TYPE_BOOK, storage, 20);

    /* One item operations */

    // Append, lookups and delete
    test_records_zero_item_ops(book);
    uint32_t value = 1;
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
    uint32_t first = 1, last = 1;
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

            reg = cdp_book_add_uint32(book, CDP_NAME_VALUE-n, value);
            test_records_register_val(reg, value);

            found = cdp_book_first(book);
            test_records_register_val(found, first);
            found = cdp_book_last(book);
            test_records_register_val(found, value);

            last = value;
        } else {
            index = 0;

            reg = cdp_book_prepend_uint32(book, CDP_NAME_VALUE-n, value);
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

        assert_true(cdp_book_traverse(book, print_values, NULL, NULL));
    }

    /* Nested books */

    cdpRecord* chdBook = cdp_book_add_book(book, CDP_NAME_TEMP, CDP_TYPE_BOOK, storage, 20);
    reg = cdp_book_prepend_uint32(chdBook, CDP_NAME_VALUE-30, value);
    test_records_register_val(reg, value);
    assert_true(cdp_book_deep_traverse(book, 3, print_values, NULL, NULL, NULL));

    cdp_book_remove(book);
}


static void test_records_tech_dictionary(unsigned storage) {
    cdpRecord* dict = cdp_book_add_dictionary(cdp_root(), CDP_NAME_TEMP, storage, 20);

    /* One item operations */

    // Isert, lookups and delete
    test_records_zero_item_ops(dict);
    uint32_t value = 1;
    cdpRecord* reg = cdp_book_add_uint32(dict, CDP_NAME_VALUE, value);
    test_records_register_val(reg, value);
    test_records_one_item_ops(dict, reg);
    cdp_register_remove(reg);

    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* found;
    uint32_t vmax = 1, vmin = 1000;
    cdpID name;

    for (unsigned n = 1; n < 10;  n++) {
        if (cdp_book_children(dict) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_register_remove(cdp_book_first(dict));
                found = cdp_book_first(dict);
                cdp_register_read(found, 0, &vmin, NULL);
                break;
              case 2:
                cdp_register_remove(cdp_book_last(dict));
                found = cdp_book_last(dict);
                cdp_register_read(found, 0, &vmax, NULL);
                break;
            }
        }

        do {
            value = munit_rand_int_range(1, 1000);
            name = CDP_NAME_VALUE - value;
            found = cdp_book_find_by_name(dict, name);
        } while (found);
        if (value < vmin)   vmin = value;
        if (value > vmax)   vmax = value;

        reg = cdp_book_add_uint32(dict, name, value);
        test_records_register_val(reg, value);

        found = cdp_book_find_by_name(dict, reg->metadata.id);
        assert_ptr_equal(found, reg);

        found = cdp_book_first(dict);
        test_records_register_val(found, vmin);

        found = cdp_book_find_by_position(dict, 0);
        test_records_register_val(found, vmin);

        found = cdp_book_last(dict);
        test_records_register_val(found, vmax);

        found = cdp_book_find_by_position(dict, cdp_book_children(dict) - 1);
        test_records_register_val(found, vmax);

        path->id[0] = reg->metadata.id;
        found = cdp_book_find_by_path(dict, path);
        assert_ptr_equal(found, reg);

        assert_true(cdp_book_traverse(dict, print_values, NULL, NULL));
    }

    /* Nested books */

    cdpRecord* chdDict = cdp_book_add_dictionary(dict, CDP_NAME_TEMP-2000, storage, 20);
    reg = cdp_book_add_uint32(chdDict, CDP_NAME_VALUE, value);
    test_records_register_val(reg, value);
    assert_true(cdp_book_deep_traverse(dict, 3, print_values, NULL, NULL, NULL));

    cdp_book_remove(dict);
}


static cdpRecord* tech_catalog_create_structure(cdpID id, int32_t value) {
    static cdpRecord book;
    CDP_0(&book);
    cdp_record_initialize(&book, CDP_TYPE_BOOK, 0, id, CDP_TYPE_DICTIONARY, CDP_STO_CHD_ARRAY, 2);
    cdpRecord* reg = cdp_book_add_int32(&book, CDP_NAME_VALUE, value);
    test_records_register_val(reg, value);
    return &book;
}

static int tech_catalog_compare(const cdpRecord* key, const cdpRecord* book, void* unused) {
    cdpRecord* regK = cdp_book_find_by_name(key, CDP_NAME_VALUE);
    cdpRecord* regB = cdp_book_find_by_name(book, CDP_NAME_VALUE);
    assert(regK && regB);
    return cdp_register_read_int32(regK) - cdp_register_read_int32(regB);
}

static void test_records_tech_catalog(unsigned storage) {
    cdpRecord* cat = cdp_book_add_catalog(cdp_root(), CDP_NAME_TEMP, storage, tech_catalog_compare, NULL, 20);

    /* One item operations */

    // Isert, lookups and delete
    test_records_zero_item_ops(cat);
    int32_t value = 1;
    cdpRecord* book = cdp_catalog_add(cat, tech_catalog_create_structure(CDP_NAME_TEMP, value));
    cdpRecord* reg = cdp_book_find_by_name(book, CDP_NAME_VALUE);
    test_records_nested_one_item_ops(cat, CDP_NAME_TEMP, reg);
    cdp_book_remove(book);

    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* found;
    int32_t vmax = 1, vmin = 1000;
    cdpID name;

    for (unsigned n = 1; n < 10;  n++) {
        if (cdp_book_children(cat) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_book_remove(cdp_book_first(cat));
                book  = cdp_book_first(cat);
                found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
                vmin  = cdp_register_read_int32(found);
                break;
              case 2:
                cdp_book_remove(cdp_book_last(cat));
                book  = cdp_book_last(cat);
                found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
                vmax  = cdp_register_read_int32(found);
                break;
            }
        }

        do {
            value = munit_rand_int_range(1, 1000);
            name = CDP_NAME_TEMP - value;
            book = cdp_book_find_by_name(cat, name);
        } while (book);
        if (value < vmin)   vmin = value;
        if (value > vmax)   vmax = value;

        book = cdp_catalog_add(cat, tech_catalog_create_structure(name, value));
        reg  = cdp_book_find_by_name(book, CDP_NAME_VALUE);
        test_records_register_val(reg, value);

        book  = cdp_book_find_by_name(cat, name);
        found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
        assert_ptr_equal(found, reg);

        book  = cdp_book_first(cat);
        found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
        test_records_register_val(found, vmin);

        book  = cdp_book_find_by_position(cat, 0);
        found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
        test_records_register_val(found, vmin);

        book  = cdp_book_last(cat);
        found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
        test_records_register_val(found, vmax);

        book  = cdp_book_find_by_position(cat, cdp_book_children(cat) - 1);
        found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
        test_records_register_val(found, vmax);

        path->id[0] = name;
        book  = cdp_book_find_by_path(cat, path);
        found = cdp_book_find_by_name(book, CDP_NAME_VALUE);
        assert_ptr_equal(found, reg);

        assert_true(cdp_book_traverse(cat, print_values, NULL, NULL));
    }

    /* Nested books */
    assert_true(cdp_book_deep_traverse(cat, 8, print_values, NULL, NULL, NULL));

    cdp_book_remove(cat);
}




static void test_records_tech_sequencing_book(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* bookL = cdp_book_add_book(cdp_root(), CDP_NAME_TEMP-1, CDP_TYPE_BOOK, CDP_STO_CHD_LINKED_LIST);
    cdpRecord* bookA = cdp_book_add_book(cdp_root(), CDP_NAME_TEMP-2, CDP_TYPE_BOOK, CDP_STO_CHD_ARRAY, maxItems);

    cdpRecord* foundL, *foundA;

    for (unsigned n = 0; n < maxItems;  n++) {
        uint32_t value = 1 + (munit_rand_uint32() % (maxItems>>1));
        cdpID name = CDP_NAME_VALUE - value;

        if ((foundL = cdp_book_find_by_name(bookL, name))) cdp_register_remove(foundL);
        if ((foundA = cdp_book_find_by_name(bookA, name))) cdp_register_remove(foundA);
        assert((!foundL && !foundA) || (foundL && foundA));

        if (cdp_book_children(bookL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_register_remove(cdp_book_first(bookL));
                cdp_register_remove(cdp_book_first(bookA));
                break;
              case 2:
                cdp_register_remove(cdp_book_last(bookL));
                cdp_register_remove(cdp_book_last(bookA));
                break;
            }
        }

        cdp_book_add_uint32(bookL, name, value);
        cdp_book_add_uint32(bookA, name, value);

        cdpRecord* recordL = cdp_book_first(bookL);
        cdpRecord* recordA = cdp_book_first(bookA);

        do {
            assert(recordL && recordA);

            cdp_register_read(recordL, 0, &value, NULL);
            test_records_register_val(recordA, value);

            recordL = cdp_book_next(bookL, recordL);
            recordA = cdp_book_next(bookA, recordA);
        } while (recordL);
    }

    cdp_book_remove(bookA);
    cdp_book_remove(bookL);
}


static void test_records_tech_sequencing_dictionary(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* dictL = cdp_book_add_dictionary(cdp_root(), CDP_NAME_TEMP-1, CDP_STO_CHD_LINKED_LIST);
    cdpRecord* dictA = cdp_book_add_dictionary(cdp_root(), CDP_NAME_TEMP-2, CDP_STO_CHD_ARRAY, maxItems);
    cdpRecord* dictT = cdp_book_add_dictionary(cdp_root(), CDP_NAME_TEMP-3, CDP_STO_CHD_RED_BLACK_T);

    cdpRecord* foundL, *foundA, *foundT;

    for (unsigned n = 0; n < maxItems;  n++) {
        uint32_t value = 1 + (munit_rand_uint32() % (maxItems>>1));
        cdpID name = CDP_NAME_VALUE - value;

        if ((foundL = cdp_book_find_by_name(dictL, name))) cdp_register_remove(foundL);
        if ((foundA = cdp_book_find_by_name(dictA, name))) cdp_register_remove(foundA);
        if ((foundT = cdp_book_find_by_name(dictT, name))) cdp_register_remove(foundT);
        assert((!foundL && !foundA && !foundT) || (foundL && foundA && foundT));

        if (cdp_book_children(dictL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_register_remove(cdp_book_first(dictL));
                cdp_register_remove(cdp_book_first(dictA));
                cdp_register_remove(cdp_book_first(dictT));
                break;
              case 2:
                cdp_register_remove(cdp_book_last(dictL));
                cdp_register_remove(cdp_book_last(dictA));
                cdp_register_remove(cdp_book_last(dictT));
                break;
            }
        }

        cdp_book_add_uint32(dictL, name, value);
        cdp_book_add_uint32(dictA, name, value);
        cdp_book_add_uint32(dictT, name, value);

        cdpRecord* recordL = cdp_book_first(dictL);
        cdpRecord* recordA = cdp_book_first(dictA);
        cdpRecord* recordT = cdp_book_first(dictT);

        do {
            assert(recordL && recordA && recordT);

            cdp_register_read(recordL, 0, &value, NULL);
            test_records_register_val(recordA, value);
            test_records_register_val(recordT, value);

            recordL = cdp_book_next(dictL, recordL);
            recordA = cdp_book_next(dictA, recordA);
            recordT = cdp_book_next(dictT, recordT);
        } while (recordL);
    }

    cdp_book_remove(dictT);
    cdp_book_remove(dictA);
    cdp_book_remove(dictL);
}


static void test_records_tech_sequencing_catalog(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* catL = cdp_book_add_catalog(cdp_root(), CDP_NAME_TEMP-1, CDP_STO_CHD_LINKED_LIST, tech_catalog_compare, NULL);
    cdpRecord* catA = cdp_book_add_catalog(cdp_root(), CDP_NAME_TEMP-2, CDP_STO_CHD_ARRAY,       tech_catalog_compare, NULL, maxItems);
    cdpRecord* catT = cdp_book_add_catalog(cdp_root(), CDP_NAME_TEMP-3, CDP_STO_CHD_RED_BLACK_T, tech_catalog_compare, NULL);

    cdpRecord* foundL, *foundA, *foundT;
    cdpRecord  key = *tech_catalog_create_structure(CDP_NAME_TEMP, 0);
    cdpRecord* reg = cdp_book_find_by_name(&key, CDP_NAME_VALUE);

    for (unsigned n = 0; n < maxItems;  n++) {
        int32_t value = 1 + (munit_rand_uint32() % (maxItems>>1));
        cdpID name = CDP_NAME_VALUE - value;
        cdp_register_update_int32(reg, value);

        if ((foundL = cdp_book_find_by_key(catL, &key))) cdp_book_remove(foundL);
        if ((foundA = cdp_book_find_by_key(catA, &key))) cdp_book_remove(foundA);
        if ((foundT = cdp_book_find_by_key(catT, &key))) cdp_book_remove(foundT);
        assert((!foundL && !foundA && !foundT) || (foundL && foundA && foundT));

        if (cdp_book_children(catL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_book_remove(cdp_book_first(catL));
                cdp_book_remove(cdp_book_first(catA));
                cdp_book_remove(cdp_book_first(catT));
                break;
              case 2:
                cdp_book_remove(cdp_book_last(catL));
                cdp_book_remove(cdp_book_last(catA));
                cdp_book_remove(cdp_book_last(catT));
                break;
            }
        }

        cdp_catalog_add(catL, tech_catalog_create_structure(name, value));
        cdp_catalog_add(catA, tech_catalog_create_structure(name, value));
        cdp_catalog_add(catT, tech_catalog_create_structure(name, value));

        cdpRecord* bookL = cdp_book_first(catL);
        cdpRecord* bookA = cdp_book_first(catA);
        cdpRecord* bookT = cdp_book_first(catT);

        do {
            cdpRecord*recordL = cdp_book_find_by_name(bookL, CDP_NAME_VALUE);
            cdpRecord*recordA = cdp_book_find_by_name(bookA, CDP_NAME_VALUE);
            cdpRecord*recordT = cdp_book_find_by_name(bookT, CDP_NAME_VALUE);
            assert(recordL && recordA && recordT);

            cdp_register_read(recordL, 0, &value, NULL);
            test_records_register_val(recordA, value);
            test_records_register_val(recordT, value);

            bookL = cdp_book_next(catL, bookL);
            bookA = cdp_book_next(catA, bookA);
            bookT = cdp_book_next(catT, bookT);
        } while (bookL);
    }

    cdp_record_finalize(&key, 4);

    cdp_book_remove(catT);
    cdp_book_remove(catA);
    cdp_book_remove(catL);
}


MunitResult test_records(const MunitParameter params[], void* user_data_or_fixture) {
    cdp_record_system_initiate();

    test_records_tech_book(CDP_STO_CHD_LINKED_LIST);
    test_records_tech_book(CDP_STO_CHD_ARRAY);
    test_records_tech_book(CDP_STO_CHD_PACKED_QUEUE);

    test_records_tech_dictionary(CDP_STO_CHD_LINKED_LIST);
    test_records_tech_dictionary(CDP_STO_CHD_ARRAY);
    test_records_tech_dictionary(CDP_STO_CHD_RED_BLACK_T);

    test_records_tech_catalog(CDP_STO_CHD_LINKED_LIST);
    test_records_tech_catalog(CDP_STO_CHD_ARRAY);
    test_records_tech_catalog(CDP_STO_CHD_RED_BLACK_T);

    test_records_tech_sequencing_book();
    test_records_tech_sequencing_dictionary();
    test_records_tech_sequencing_catalog();

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
