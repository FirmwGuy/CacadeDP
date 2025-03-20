/*
 *  Copyright (c) 2024 Victor M. Barrientos (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
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




#include "test.h"
#include "cdp_record.h"
//#include "cdp_agent.h"
#include <stdio.h>      // sprintf()




enum {
    CDP_NAME_ENUMERATION = CDP_WORD_ROOT + 100,
    CDP_NAME_TEMP,

    CDP_NAME_Z_COUNT
};



static void test_records_print(cdpRecord* record, char *sval) {
    if (!record) {
        strcpy(sval, "Void");
    } else if (cdp_record_is_dictionary(record)) {
        sprintf(sval, "{%"PRIX64"}", cdp_record_get_name(record));
    } else if (cdp_record_children(record)) {
        sprintf(sval, "[%"PRIX64"]", cdp_record_get_name(record));
    } else if (cdp_record_has_data(record)) {
        cdpValue val = cdp_record_value(record);
        sprintf(sval, "%u", val.uint32);
    }
}

static bool print_values(cdpEntry* entry, void* unused) {
    assert_not_null(entry->record);
    char this[16], prev[16], next[16];
    test_records_print(entry->record, this);
    test_records_print(entry->prev,   prev);
    test_records_print(entry->next,   next);
    munit_logf(MUNIT_LOG_DEBUG, "(%u):  %s  <%s, %s>\n", (unsigned)entry->position, this, prev, next);
    return true;
}


static void test_records_value(cdpRecord* rec, cdpValue trueval) {
    cdpData* data = rec->data;
    cdpValue vread = data->value[0];
    cdpValue value = cdp_record_value(rec);
    assert_size(data->capacity, ==, sizeof((cdpData){}.value));
    assert_size(data->size, ==, sizeof(int32_t));
    assert_uint(trueval.uint32, ==, value.uint32);
    assert_uint(trueval.uint32, ==, vread.uint32);
}




static void test_records_zero_item_ops(cdpRecord* record) {
    assert_false(cdp_record_children(record));
    assert_null(cdp_record_last(record));
    assert_null(cdp_record_find_by_name(record, CDP_NAME_ENUMERATION));
    assert_null(cdp_record_find_by_position(record, 0));
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    path->id[0] = 0;
    assert_null(cdp_record_find_by_path(record, path));
    assert_true(cdp_record_traverse(record, print_values, NULL, NULL));
}


static void test_records_one_item_ops(cdpRecord* record, cdpRecord* item) {
    assert_true(cdp_record_children(record));
    cdpRecord* found = cdp_record_last(record);
    assert_ptr_equal(found, item);
    found = cdp_record_find_by_name(record, cdp_record_get_name(item));
    assert_ptr_equal(found, item);
    found = cdp_record_find_by_position(record, 0);
    assert_ptr_equal(found, item);
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    path->id[0] = cdp_record_get_name(item);
    found = cdp_record_find_by_path(record, path);
    assert_ptr_equal(found, item);
    assert_true(cdp_record_traverse(record, print_values, NULL, NULL));
}


static void test_records_nested_one_item_ops(cdpRecord* cat, cdpID name, cdpRecord* item) {
    cdpRecord* record  = cdp_record_last(cat);
    cdpRecord* found = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
    assert_ptr_equal(found, item);

    record = cdp_record_find_by_name(cat, name);
    found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
    assert_ptr_equal(found, item);

    record = cdp_record_find_by_position(cat, 0);
    found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
    assert_ptr_equal(found, item);

    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    path->id[0] = name;
    record = cdp_record_find_by_path(cat, path);
    found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
    assert_ptr_equal(found, item);

    assert_true(cdp_record_traverse(record, print_values, NULL, NULL));
}




static void test_records_tech_list(unsigned storage) {
    cdpRecord* list = cdp_record_add_list(cdp_root(), CDP_NAME_TEMP, 0, CDP_NAME_TEMP, CDP_NAME_TEMP, storage, 20);

    /* One item operations */

    // Append, lookups and delete
    test_records_zero_item_ops(list);
    cdpValue  value = (cdpValue){.uint32 = 1};
    cdpRecord* item = cdp_record_append_value(list, CDP_NAME_ENUMERATION, CDP_NAME_ENUMERATION, CDP_NAME_ENUMERATION, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
    test_records_value(item, value);
    test_records_one_item_ops(list, item);
    cdp_record_delete(item);

    // Push and lookups
    test_records_zero_item_ops(list);
    value.uint32 = 1;
    item = cdp_record_prepend_value(list, CDP_NAME_ENUMERATION, CDP_NAME_ENUMERATION, CDP_NAME_ENUMERATION, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
    test_records_value(item, value);
    test_records_one_item_ops(list, item);

    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* found;
    uint32_t first = 1, last = 1;
    size_t index;

    for (unsigned n = 1; n < 10;  n++) {
        if (cdp_record_children(list) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_record_delete(cdp_record_first(list));
                found = cdp_record_first(list);
                first = cdp_record_value(found).uint32;
                break;
              case 2:
                cdp_record_delete(cdp_record_last(list));
                found = cdp_record_last(list);
                last  = cdp_record_value(found).uint32;
                break;
            }
        }

        value = (cdpValue) (n + 1);
        if (munit_rand_uint32() & 1) {
            index = cdp_record_children(list);

            item = cdp_record_append_value(list, CDP_NAME_Z_COUNT+n, CDP_NAME_Z_COUNT+n, CDP_NAME_Z_COUNT+n, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
            test_records_value(item, value);

            found = cdp_record_first(list);
            test_records_value(found, (cdpValue)first);
            found = cdp_record_last(list);
            test_records_value(found, value);

            last = value.uint32;
        } else {
            index = 0;

            item = cdp_record_prepend_value(list, CDP_NAME_Z_COUNT+n, CDP_NAME_Z_COUNT+n, CDP_NAME_Z_COUNT+n, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
            test_records_value(item, value);

            found = cdp_record_first(list);
            test_records_value(found, value);
            found = cdp_record_last(list);
            test_records_value(found, (cdpValue)last);

            first = value.uint32;
        }

        found = cdp_record_find_by_name(list, cdp_record_get_name(item));
        assert_ptr_equal(found, item);

        found = cdp_record_find_by_position(list, index);
        assert_ptr_equal(found, item);

        path->id[0] = cdp_record_get_name(item);
        found = cdp_record_find_by_path(list, path);
        assert_ptr_equal(found, item);

        assert_true(cdp_record_traverse(list, print_values, NULL, NULL));
    }

    /* Nested record */

    cdpRecord* child = cdp_record_append_list(list, CDP_NAME_TEMP, CDP_NAME_TEMP, CDP_NAME_TEMP, storage, 20);
    item = cdp_record_prepend_value(child, CDP_NAME_Z_COUNT+30, CDP_NAME_Z_COUNT+30, CDP_NAME_Z_COUNT+30, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
    test_records_value(item, value);
    assert_true(cdp_record_deep_traverse(list, print_values, NULL, NULL, NULL));

    cdp_record_delete(list);
}


static void test_records_tech_dictionary(unsigned storage) {
    cdpRecord* dict = cdp_record_add_dictionary(cdp_root(), CDP_NAME_TEMP, 0, CDP_NAME_TEMP, CDP_NAME_TEMP, storage, 20);

    /* One item operations */

    // Isert, lookups and delete
    test_records_zero_item_ops(dict);
    cdpValue  value = (cdpValue){.uint32 = 1};
    cdpRecord* item = cdp_record_add_value(dict, CDP_NAME_ENUMERATION, 0, CDP_NAME_ENUMERATION, CDP_NAME_ENUMERATION, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
    test_records_value(item, value);
    test_records_one_item_ops(dict, item);
    cdp_record_delete(item);

    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* found;
    uint32_t vmax = 1, vmin = 1000;
    cdpID name;

    for (unsigned n = 1; n < 10;  n++) {
        if (cdp_record_children(dict) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_record_delete(cdp_record_first(dict));
                found = cdp_record_first(dict);
                vmin = cdp_record_value(found).uint32;
                break;
              case 2:
                cdp_record_delete(cdp_record_last(dict));
                found = cdp_record_last(dict);
                vmax = cdp_record_value(found).uint32;
                break;
            }
        }

        do {
            value.uint32 = munit_rand_int_range(1, 1000);
            name = CDP_NAME_ENUMERATION + value.uint32;
            found = cdp_record_find_by_name(dict, name);
        } while (found);
        if (value.uint32 < vmin)   vmin = value.uint32;
        if (value.uint32 > vmax)   vmax = value.uint32;

        item = cdp_record_add_value(dict, name, 0, name, name, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
        test_records_value(item, value);

        found = cdp_record_find_by_name(dict, cdp_record_get_name(item));
        assert_ptr_equal(found, item);

        found = cdp_record_first(dict);
        test_records_value(found, (cdpValue)vmin);

        found = cdp_record_find_by_position(dict, 0);
        test_records_value(found, (cdpValue)vmin);

        found = cdp_record_last(dict);
        test_records_value(found, (cdpValue)vmax);

        found = cdp_record_find_by_position(dict, cdp_record_children(dict) - 1);
        test_records_value(found, (cdpValue)vmax);

        path->id[0] = cdp_record_get_name(item);
        found = cdp_record_find_by_path(dict, path);
        assert_ptr_equal(found, item);

        assert_true(cdp_record_traverse(dict, print_values, NULL, NULL));
    }

    /* Nested record */

    cdpRecord* child = cdp_record_add_dictionary(dict, CDP_NAME_TEMP+2000, 0, CDP_NAME_TEMP+2000, CDP_NAME_TEMP+2000, storage, 20);
    item = cdp_record_add_value(child, CDP_NAME_ENUMERATION, 0, CDP_NAME_ENUMERATION, CDP_NAME_ENUMERATION, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
    test_records_value(item, value);
    assert_true(cdp_record_deep_traverse(dict, print_values, NULL, NULL, NULL));

    cdp_record_delete(dict);
}


static cdpRecord* tech_catalog_create_structure(cdpID name, cdpValue value) {
    static cdpRecord record;
    CDP_0(&record);
    cdp_record_initialize_dictionary(&record, name, name, name, CDP_STORAGE_ARRAY, 2);
    cdpRecord* item = cdp_record_add_value(&record, CDP_NAME_ENUMERATION, 0, CDP_NAME_ENUMERATION, CDP_NAME_ENUMERATION, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
    test_records_value(item, value);
    return &record;
}

static int tech_catalog_compare(const cdpRecord* key, const cdpRecord* record, void* unused) {
    cdpRecord* itemK = cdp_record_find_by_name(key, CDP_NAME_ENUMERATION);
    cdpRecord* itemB = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
    assert(itemK && itemB);
    return cdp_record_value(itemK).int32 - cdp_record_value(itemB).int32;
}

static void test_records_tech_catalog(unsigned storage) {
    cdpRecord* cat;
    if (storage == CDP_STORAGE_ARRAY) {
        cat = cdp_record_add_catalog(cdp_root(), CDP_NAME_TEMP, 0, CDP_NAME_TEMP, CDP_NAME_TEMP, storage, 20, tech_catalog_compare);
    } else {
        cat = cdp_record_add_catalog(cdp_root(), CDP_NAME_TEMP, 0, CDP_NAME_TEMP, CDP_NAME_TEMP, storage, tech_catalog_compare);
    }

    /* One item operations */

    // Isert, lookups and delete
    test_records_zero_item_ops(cat);
    cdpValue value = (cdpValue){.int32 = 1};
    cdpRecord* record = cdp_record_add(cat, CDP_V(NULL), tech_catalog_create_structure(CDP_NAME_TEMP, value));
    cdpRecord* item = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
    test_records_nested_one_item_ops(cat, CDP_NAME_TEMP, item);
    cdp_record_delete(record);

    // Multi-item ops
    cdpPath* path = cdp_alloca(sizeof(cdpPath) + (1 * sizeof(cdpID)));
    path->length = 1;
    path->capacity = 1;
    cdpRecord* found;
    int32_t vmax = 1, vmin = 1000;
    cdpID name;

    for (unsigned n = 1; n < 10;  n++) {
        if (cdp_record_children(cat) > 2) {
            switch (munit_rand_int_range(0, 2)) {
              case 1:
                cdp_record_delete(cdp_record_first(cat));
                record = cdp_record_first(cat);
                found = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
                vmin = cdp_record_value(found).int32;
                break;
              case 2:
                cdp_record_delete(cdp_record_last(cat));
                record = cdp_record_last(cat);
                found = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
                vmax = cdp_record_value(found).int32;
                break;
            }
        }

        do {
            value.int32 = munit_rand_int_range(1, 1000);
            name = CDP_NAME_TEMP + value.int32;
            record = cdp_record_find_by_name(cat, name);
        } while (record);
        if (value.int32 < vmin)   vmin = value.int32;
        if (value.int32 > vmax)   vmax = value.int32;

        record = cdp_record_add(cat, CDP_V(NULL), tech_catalog_create_structure(name, value));
        item   = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
        test_records_value(item, value);

        record = cdp_record_find_by_name(cat, name);
        found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
        assert_ptr_equal(found, item);

        record = cdp_record_first(cat);
        found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
        test_records_value(found, CDP_V(vmin));

        record = cdp_record_find_by_position(cat, 0);
        found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
        test_records_value(found, CDP_V(vmin));

        record = cdp_record_last(cat);
        found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
        test_records_value(found, CDP_V(vmax));

        record = cdp_record_find_by_position(cat, cdp_record_children(cat) - 1);
        found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
        test_records_value(found, CDP_V(vmax));

        path->id[0] = name;
        record = cdp_record_find_by_path(cat, path);
        found  = cdp_record_find_by_name(record, CDP_NAME_ENUMERATION);
        assert_ptr_equal(found, item);

        assert_true(cdp_record_traverse(cat, print_values, NULL, NULL));
    }

    /* Nested record */
    assert_true(cdp_record_deep_traverse(cat, print_values, NULL, NULL, NULL));

    cdp_record_delete(cat);
}




static void test_records_tech_sequencing_list(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* bookL = cdp_record_add_list(cdp_root(), CDP_NAME_TEMP+1, 0, CDP_NAME_TEMP+1, CDP_NAME_TEMP+1, CDP_STORAGE_LINKED_LIST);
    cdpRecord* bookA = cdp_record_add_list(cdp_root(), CDP_NAME_TEMP+2, 0, CDP_NAME_TEMP+2, CDP_NAME_TEMP+2, CDP_STORAGE_ARRAY, maxItems);

    cdpRecord* foundL, *foundA;

    for (unsigned n = 0; n < maxItems;  n++) {
        cdpValue value = (cdpValue){.uint32 = 1 + (munit_rand_uint32() % (maxItems>>1))};
        cdpID name = CDP_NAME_ENUMERATION + value.uint32;

        if ((foundL = cdp_record_find_by_name(bookL, name))) cdp_record_delete(foundL);
        if ((foundA = cdp_record_find_by_name(bookA, name))) cdp_record_delete(foundA);
        assert((!foundL && !foundA) || (foundL && foundA));

        if (cdp_record_children(bookL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_record_delete(cdp_record_first(bookL));
                cdp_record_delete(cdp_record_first(bookA));
                break;
              case 2:
                cdp_record_delete(cdp_record_last(bookL));
                cdp_record_delete(cdp_record_last(bookA));
                break;
            }
        }

        cdp_record_add_value(bookL, name, 0, name, name, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
        cdp_record_add_value(bookA, name, 0, name, name, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));

        cdpRecord* recordL = cdp_record_first(bookL);
        cdpRecord* recordA = cdp_record_first(bookA);

        do {
            assert(recordL && recordA);

            value = cdp_record_value(recordL);
            test_records_value(recordA, value);

            recordL = cdp_record_next(bookL, recordL);
            recordA = cdp_record_next(bookA, recordA);
        } while (recordL);
    }

    cdp_record_delete(bookA);
    cdp_record_delete(bookL);
}


static void test_records_tech_sequencing_dictionary(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* dictL = cdp_record_add_dictionary(cdp_root(), CDP_NAME_TEMP+1, 0, CDP_NAME_TEMP+1, CDP_NAME_TEMP+1, CDP_STORAGE_LINKED_LIST);
    cdpRecord* dictA = cdp_record_add_dictionary(cdp_root(), CDP_NAME_TEMP+2, 0, CDP_NAME_TEMP+2, CDP_NAME_TEMP+2, CDP_STORAGE_ARRAY, maxItems);
    cdpRecord* dictT = cdp_record_add_dictionary(cdp_root(), CDP_NAME_TEMP+3, 0, CDP_NAME_TEMP+3, CDP_NAME_TEMP+3, CDP_STORAGE_RED_BLACK_T);

    cdpRecord* foundL, *foundA, *foundT;

    for (unsigned n = 0; n < maxItems;  n++) {
        cdpValue value = (cdpValue){.uint32 = 1 + (munit_rand_uint32() % (maxItems>>1))};
        cdpID name = CDP_NAME_ENUMERATION + value.uint32;

        if ((foundL = cdp_record_find_by_name(dictL, name))) cdp_record_delete(foundL);
        if ((foundA = cdp_record_find_by_name(dictA, name))) cdp_record_delete(foundA);
        if ((foundT = cdp_record_find_by_name(dictT, name))) cdp_record_delete(foundT);
        assert((!foundL && !foundA && !foundT) || (foundL && foundA && foundT));

        if (cdp_record_children(dictL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_record_delete(cdp_record_first(dictL));
                cdp_record_delete(cdp_record_first(dictA));
                cdp_record_delete(cdp_record_first(dictT));
                break;
              case 2:
                cdp_record_delete(cdp_record_last(dictL));
                cdp_record_delete(cdp_record_last(dictA));
                cdp_record_delete(cdp_record_last(dictT));
                break;
            }
        }

        cdp_record_add_value(dictL, name, 0, name, name, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
        cdp_record_add_value(dictA, name, 0, name, name, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));
        cdp_record_add_value(dictT, name, 0, name, name, (cdpID)0, CDP_ID(0), value, sizeof(uint32_t), sizeof(uint32_t));

        cdpRecord* recordL = cdp_record_first(dictL);
        cdpRecord* recordA = cdp_record_first(dictA);
        cdpRecord* recordT = cdp_record_first(dictT);

        do {
            assert(recordL && recordA && recordT);

            value = cdp_record_value(recordL);
            test_records_value(recordA, value);
            test_records_value(recordT, value);

            recordL = cdp_record_next(dictL, recordL);
            recordA = cdp_record_next(dictA, recordA);
            recordT = cdp_record_next(dictT, recordT);
        } while (recordL);
    }

    cdp_record_delete(dictT);
    cdp_record_delete(dictA);
    cdp_record_delete(dictL);
}


static void test_records_tech_sequencing_catalog(void) {
    size_t maxItems = munit_rand_int_range(2, 100);

    cdpRecord* catL = cdp_record_add_catalog(cdp_root(), CDP_NAME_TEMP+1, 0, CDP_NAME_TEMP+1, CDP_NAME_TEMP+1, CDP_STORAGE_LINKED_LIST, tech_catalog_compare);
    cdpRecord* catA = cdp_record_add_catalog(cdp_root(), CDP_NAME_TEMP+2, 0, CDP_NAME_TEMP+2, CDP_NAME_TEMP+2, CDP_STORAGE_ARRAY, maxItems, tech_catalog_compare);
    cdpRecord* catT = cdp_record_add_catalog(cdp_root(), CDP_NAME_TEMP+3, 0, CDP_NAME_TEMP+3, CDP_NAME_TEMP+3, CDP_STORAGE_RED_BLACK_T, tech_catalog_compare);

    cdpRecord* foundL, *foundA, *foundT;
    cdpRecord  key = *tech_catalog_create_structure(CDP_NAME_TEMP, (cdpValue)0);
    cdpRecord* item = cdp_record_find_by_name(&key, CDP_NAME_ENUMERATION);

    for (unsigned n = 0; n < maxItems;  n++) {
        cdpValue value = {.int32 = 1 + (munit_rand_uint32() % (maxItems>>1))};
        cdpID name = CDP_NAME_ENUMERATION + value.int32;
        cdp_record_update_value(item, sizeof(int32_t), value);

        if ((foundL = cdp_record_find_by_key(catL, &key, tech_catalog_compare, NULL)))    cdp_record_delete(foundL);
        if ((foundA = cdp_record_find_by_key(catA, &key, tech_catalog_compare, NULL)))    cdp_record_delete(foundA);
        if ((foundT = cdp_record_find_by_key(catT, &key, tech_catalog_compare, NULL)))    cdp_record_delete(foundT);
        assert((!foundL && !foundA && !foundT) || (foundL && foundA && foundT));

        if (cdp_record_children(catL)) {
            switch (munit_rand_int_range(0, 4)) {
              case 1:
                cdp_record_delete(cdp_record_first(catL));
                cdp_record_delete(cdp_record_first(catA));
                cdp_record_delete(cdp_record_first(catT));
                break;
              case 2:
                cdp_record_delete(cdp_record_last(catL));
                cdp_record_delete(cdp_record_last(catA));
                cdp_record_delete(cdp_record_last(catT));
                break;
            }
        }

        cdp_record_add(catL, CDP_V(NULL), tech_catalog_create_structure(name, value));
        cdp_record_add(catA, CDP_V(NULL), tech_catalog_create_structure(name, value));
        cdp_record_add(catT, CDP_V(NULL), tech_catalog_create_structure(name, value));

        cdpRecord* bookL = cdp_record_first(catL);
        cdpRecord* bookA = cdp_record_first(catA);
        cdpRecord* bookT = cdp_record_first(catT);

        do {
            cdpRecord*recordL = cdp_record_find_by_name(bookL, CDP_NAME_ENUMERATION);
            cdpRecord*recordA = cdp_record_find_by_name(bookA, CDP_NAME_ENUMERATION);
            cdpRecord*recordT = cdp_record_find_by_name(bookT, CDP_NAME_ENUMERATION);
            assert(recordL && recordA && recordT);

            value = cdp_record_value(recordL);
            test_records_value(recordA, value);
            test_records_value(recordT, value);

            bookL = cdp_record_next(catL, bookL);
            bookA = cdp_record_next(catA, bookA);
            bookT = cdp_record_next(catT, bookT);
        } while (bookL);
    }

    cdp_record_finalize(&key);

    cdp_record_delete(catT);
    cdp_record_delete(catA);
    cdp_record_delete(catL);
}


MunitResult test_records(const MunitParameter params[], void* user_data_or_fixture) {
    cdp_record_system_initiate();

    test_records_tech_list(CDP_STORAGE_LINKED_LIST);
    test_records_tech_list(CDP_STORAGE_ARRAY);
    test_records_tech_list(CDP_STORAGE_PACKED_QUEUE);
    test_records_tech_sequencing_list();

    test_records_tech_dictionary(CDP_STORAGE_LINKED_LIST);
    test_records_tech_dictionary(CDP_STORAGE_ARRAY);
    test_records_tech_dictionary(CDP_STORAGE_RED_BLACK_T);
    test_records_tech_sequencing_dictionary();

    test_records_tech_catalog(CDP_STORAGE_LINKED_LIST);
    test_records_tech_catalog(CDP_STORAGE_ARRAY);
    test_records_tech_catalog(CDP_STORAGE_RED_BLACK_T);
    test_records_tech_sequencing_catalog();

    cdp_record_system_shutdown();
    return MUNIT_OK;
}

