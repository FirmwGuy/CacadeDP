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


typedef struct {
    cdpChdStore store;         // Parent info.
    //
    size_t      capacity;         // Total capacity of the array to manage allocations
    //
    cdpRecord*  record;           // Children Record
} cdpArray;




/*
    Dynamic array implementation
*/

static inline cdpArray* array_new(int capacity) {
    CDP_NEW(cdpArray, array);
    array->capacity = capacity;
    array->record = cdp_malloc0(capacity * sizeof(cdpRecord));
    return array;
}


static inline void array_del(cdpArray* array) {
    cdp_free(array->record);
    cdp_free(array);
}


static inline cdpRecord* array_search(cdpArray* array, const void* key, cdpCompare compare, void* context, size_t* index) {
    size_t imax = cdp_ptr_has_val(index)? *index - 1: array->store.chdCount - 1;
    size_t imin = 0, i;
    cdpRecord* record;
    do {
        i = (imax + imin) >> 1;  // (max + min) / 2
        record = &array->record[i];
        int res = compare(key, record, array->store.context);
        if (0 > res) {
            if (!i) break;
            imax = i - 1;
        } else if (0 < res) {
            imin = ++i;
        } else {
            CDP_PTR_SEC_SET(index, i);
            return record;
        }
    } while (imax >= imin);
    CDP_PTR_SEC_SET(index, i);
    return NULL;
}


static inline void array_update_children_parent_ptr(cdpRecord* record, cdpRecord* last) {
    for (;  record <= last;  record++) {
        if (cdp_record_is_book(record)) {
            cdpChdStore* store = record->recData.book.children;
            store->book = record;    // Updates (grand) child link to the (child) parent.
        }
    }
}


static inline cdpRecord* array_sorted_insert(cdpArray* array, cdpRecord* record, cdpCompare compare, void* context) {
    size_t index = 0;
    cdpRecord* prev = array_search(array, record, compare, context, &index);
    if (prev) {
        // FixMe: delete children.
        assert(prev);
    }
    child = &array->record[index];
    if (index < array->store.chdCount) {
        memmove(child + 1, child, array->store.chdCount * sizeof(cdpRecord));
        array_update_children_parent_ptr(child + 1, &array->record[array->store.chdCount]);
        CDP_0(child);
    }
    return child;
}

static inline cdpRecord* array_add(cdpArray* array, cdpRecord* parent, bool prepend, const cdpRecord* record) {
    // Increase array space if necessary
    if (array->capacity == array->store.chdCount) {
        assert(array->capacity);
        array->capacity *= 2;
        CDP_REALLOC(array->record, array->capacity * sizeof(cdpRecord));
        memset(&array->record[array->store.chdCount], 0, array->store.chdCount * sizeof(cdpRecord));
        array_update_children_parent_ptr(array->record, &array->record[array->store.chdCount - 1]);
    }

    // Insert
    cdpRecord* child;
    if (array->store.chdCount) {
        if (cdp_record_is_dictionary(parent)) {
            child = array_sorted_insert(array, record, record_compare_by_name, NULL);
        } else if (cdp_record_is_catalog(parent)) {
            child = array_sorted_insert(array, record, array->store->sorter.compare, array->store->sorter.context);
        } else if (prepend) {
            // Prepend
            child = array->record;
            memmove(child + 1, child, array->store.chdCount * sizeof(cdpRecord));
            array_update_children_parent_ptr(child + 1, &array->record[array->store.chdCount]);
            CDP_0(child);
        } else {
            // Append
            child = &array->record[array->store.chdCount];
        }
    } else {
        child = array->record;
    }
    *child = *record;

    return child;
}


static inline cdpRecord* array_first(cdpArray* array) {
    return array->record;
}


static inline cdpRecord* array_last(cdpArray* array) {
    return &array->record[array->store.chdCount - 1];
}


static inline cdpRecord* array_find_by_name(cdpArray* array, cdpID id, const cdpRecord* book) {
    if (cdp_record_is_dictionary(book) && !array->store.compare) {    // FixMe: catalog
        cdpRecord key = {.metadata.id = id};
        return array_search(array, &key, record_compare_by_name, NULL, NULL);
    } else {
        cdpRecord* record = array->record;
        for (size_t i = 0; i < array->store.chdCount; i++, record++) {
            if (record->metadata.id == id)
                return record;
        }
    }
    return NULL;
}


static inline cdpRecord* array_find_by_position(cdpArray* array, size_t position) {
    assert(position < array->store.chdCount);
    return &array->record[position];
}


static inline cdpRecord* array_prev(cdpArray* array, cdpRecord* record) {
    return (record > array->record)? record - 1: NULL;
}


static inline cdpRecord* array_next(cdpArray* array, cdpRecord* record) {
    cdpRecord* last = &array->record[array->store.chdCount - 1];
    return (record < last)? record + 1: NULL;
}


static inline cdpRecord* array_next_by_name(cdpArray* array, cdpID id, uintptr_t* prev) {
    cdpRecord* record = array->record;
    for (size_t i = prev? (*prev + 1): 0;  i < array->store.chdCount;  i++, record++){
        if (record->metadata.id == id)
            return record;
    }
    return NULL;
}

static inline bool array_traverse(cdpArray* array, cdpRecord* book, cdpRecordTraverse func, void* context) {
    assert(array && array->capacity >= array->store.chdCount);
    cdpBookEntry entry = {.parent = book, .next = array->record};
    cdpRecord* last = &array->record[array->store.chdCount - 1];
    do {
        if (entry.record) {
            if (!func(&entry, 0, context))
                return false;
            entry.position++;
            entry.prev = entry.record;
        }
        entry.record = entry.next;
        entry.next++;
    } while (entry.next <= last);
    entry.next = NULL;
    return func(&entry, 0, context);
}


static inline void array_sort(cdpArray* array, cdpCompare compare, void* context) {
  #ifdef _GNU_SOURCE
    qsort_r
  #else
    qsort_s
  #endif
        (array->record, array->store.chdCount, sizeof(cdpRecord), (cdpFunc) compare, context);

    array_update_children_parent_ptr(array->record, &array->record[array->store.chdCount - 1]);
}


static inline void array_remove_record(cdpArray* array, cdpRecord* record) {
    assert(array && array->capacity >= array->store.chdCount);
    cdpRecord* last = &array->record[array->store.chdCount - 1];
    if (record < last)
        memmove(record, record + 1, (size_t) cdp_ptr_dif(last, record));
    CDP_0(last);
}


static inline void array_del_all_children(cdpArray* array, unsigned maxDepth) {
    cdpRecord* child = array->record;
    for (size_t n = 0; n < array->store.chdCount; n++, child++) {
        cdp_record_finalize(child, maxDepth - 1);
        CDP_0(child);   // ToDo: this may be skipped.
    }
}
