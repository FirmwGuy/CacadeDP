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
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */


#include "cdp_record.h"

#include <stdarg.h>




#define CDP_MAX_FAST_STACK_DEPTH  16

unsigned MAX_DEPTH = CDP_MAX_FAST_STACK_DEPTH;     // FixMe: (used by path/traverse) better policy than a global for this.

#define cdpFunc     void*




static inline int record_compare_by_name(const cdpRecord* restrict key, const cdpRecord* restrict rec, void* unused) {
    cdpID keyName = cdp_record_get_name(key);
    cdpID recName = cdp_record_get_name(rec);
    return (keyName < recName)? -1: (keyName > recName);
}




/*
    Include child storage techs
*/
#include "cdp_storage_linked_list.h"
#include "cdp_storage_dynamic_array.h"
#include "cdp_storage_packed_queue.h"
#include "cdp_storage_red_black_tree.h"
#include "cdp_storage_octree.h"




/***********************************************
 *                                             *
 * CascadeDP Layer 1 Record API starts here... *
 *                                             *
 ***********************************************/


cdpRecord CDP_ROOT;   // The root record.


/*
    Initiates the record system.
*/
void cdp_record_system_initiate(void) {
    cdp_record_initialize_dictionary(&CDP_ROOT, cdp_text_to_word(CDP_WORD_ROOT), CDP_STORAGE_RED_BLACK_T, 0);   // The root dictionary is the same as "/" in text paths.
}


/*
    Shutdowns the record system.
*/
void cdp_record_system_shutdown(void) {
    cdp_record_finalize(&CDP_ROOT);
}




static inline void* record_create_storage(unsigned storage, unsigned capacity) {
    cdpStore* store;

    switch (storage) {
      case CDP_STORAGE_LINKED_LIST: {
        store = list_new();
        break;
      }
      case CDP_STORAGE_ARRAY: {
        assert(capacity > 0);
        store = array_new(capacity);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(capacity > 0);
        store = packed_q_new(capacity);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        store = rb_tree_new();
        break;
      }
      case CDP_STORAGE_OCTREE: {
        store = octree_new();
        break;
      }
    }
    store->autoid = 1;

    return store;
}


void cdp_record_relink_storage(cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    cdpStore* store = record->children;
    assert(store);
    store->owner = record;           // Re-link record with its own children storage.
}


static inline void store_check_auto_id(cdpStore* parStore, cdpRecord* record) {
    if (cdp_record_id_is_pending(record)) {
        cdp_record_set_name(record, cdp_id_to_numeric(parStore->autoid++));
    }
    // FixMe: if otherwise.
}




/*
    Creates a new children store for records.
*/
cdpStore* cdp_store_new(  cdpID domain, cdpID tag,
                          unsigned storage, unsigned sorting, size_t basez
                          cdpCompare compare  ) {
    assert(record && cdp_id_valid(name) && (type < CDP_TYPE_COUNT) && (storage < CDP_STORAGE_COUNT) && (sorting < CDP_SORT_COUNT));

    switch (storage) {
      case CDP_STORAGE_ARRAY: {
        assert(basez);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        sorting = CDP_SORT_BY_INSERTION;
        assert(basez);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        assert(sorting != CDP_SORT_BY_INSERTION);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        sorting = CDP_SORT_BY_FUNCTION;
        break;
      }
    }

    //CDP_0(record);

    record->metarecord.name    = name;
    record->metarecord.type    = type;
    record->metarecord.storage = storage;
    record->metarecord.sorting = sorting;

    va_list args;
    va_start(args, sorting);
    if (type == CDP_TYPE_NORMAL) {
        record->basez = basez;

        if (sorting == CDP_SORT_BY_FUNCTION
         || sorting == CDP_SORT_BY_HASH) {

        }
    } else {
        void* address = va_arg(args, void*);
        assert(address);

        if (type == CDP_TYPE_LINK)
            record->link = address;
        else
            record->agent = address;
    }
    va_end(args);
}


/*
    Initiates a record struct with the requested parameters.
*/
void cdp_record_initialize( cdpRecord* record, cdpID name, unsigned type,
                            unsigned storage, size_t basez, unsigned sorting,
                            ... ) {
    assert(record && cdp_id_valid(name) && (type < CDP_TYPE_COUNT) && (storage < CDP_STORAGE_COUNT) && (sorting < CDP_SORT_COUNT));

    switch (storage) {
      case CDP_STORAGE_ARRAY: {
        assert(basez);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        sorting = CDP_SORT_BY_INSERTION;
        assert(basez);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        assert(sorting != CDP_SORT_BY_INSERTION);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        sorting = CDP_SORT_BY_FUNCTION;
        break;
      }
    }

    //CDP_0(record);

    record->metarecord.name    = name;
    record->metarecord.type    = type;
    record->metarecord.storage = storage;
    record->metarecord.sorting = sorting;

    va_list args;
    va_start(args, sorting);
    if (type == CDP_TYPE_NORMAL) {
        record->basez = basez;

        if (sorting == CDP_SORT_BY_FUNCTION
         || sorting == CDP_SORT_BY_HASH) {

        }
    } else {
        void* address = va_arg(args, void*);
        assert(address);

        if (type == CDP_TYPE_LINK)
            record->link = address;
        else
            record->agent = address;
    }
    va_end(args);
}




/* Creates a deep copy of record and all its data.
*/
void cdp_record_initialize_clone(cdpRecord* clone, cdpID nameID, cdpRecord* record) {
    assert(clone && cdp_record_is_normal(record));

    assert(!cdp_record_has_data(record) && !cdp_record_with_store(record));
    // Clone data: Pending!

    CDP_0(clone);

    clone->metarecord = record->metarecord;
}




/*
    Adds/allocates data into a record.
*/
void* cdp_record_set_data(  cdpRecord* record, cdpID domain, cdpID tag,
                            cdpValue attribute, unsigned datatype, bool writable,
                            cdpValue value, ... ) {
    assert(cdp_tag_valid(domain) && cdp_tag_valid(tag) && (datatype < CDP_DATATYPE_COUNT));
    if (!cdp_record_has_data(record)) {
        assert(cdp_record_has_data(record))
        return NULL;
    }

    cdpData* data;
    void* dataloc;
    va_list args;
    va_start(args, writable);

    switch (datatype) {
      case CDP_DATATYPE_VALUE: {
        size_t size     = va_arg(args, size_t);
        size_t capacity = va_arg(args, size_t);
        assert(capacity);

        size_t dmax = cdp_max(sizeof((cdpData){}.value), capacity);
        data = cdp_malloc0(sizeof(cdpData) - sizeof((cdpData){}.value) + dmax);

        if (size > sizeof(cdpValue)) {
            memcpy(data->value, value->pointer, capacity);
        } else if (size) {
            data->value = value;
        } else {
            size = capacity;
        }
        data->capacity = dmax;
        data->size     = size;
        break;
      }
      case CDP_DATATYPE_DATA: {
        size_t size       = va_arg(args, size_t);
        size_t capacity   = va_arg(args, size_t);
        cdpDel destructor = va_arg(args, cdpDel);
        assert(capacity);

        data = cdp_malloc0(sizeof(cdpData));

        if (value.pointer) {
            assert(size);
            data->data = cdp_malloc(sizeof(cdpData) - sizeof(record->data->_data) + dmax);
            memcpy(data->data, value.pointer, capacity);
        } else {
            record->data = cdp_malloc0(sizeof(cdpData) - sizeof(record->data->_data) + dmax);
        }
        data->capacity   = capacity;
        data->size       = size;
        data->data       = value.pointer;
        data->destructor = destructor;
        break;
      }
      case CDP_DATATYPE_HANDLE: {
        break;
      }
    }

    data->domain    = domain;
    data->tag       = tag;
    data->attribute = attribute;
    data->datatype  = datatype;
    data->writable  = writable;

    va_end(args);

    return data;
}


/*
    Adds/inserts a *copy* of the specified record into another record.
*/
cdpRecord* cdp_record_add(cdpRecord* parent, cdpRecord* record, bool prepend) {
    assert(cdp_record_is_normal(parent) && !cdp_record_is_void(record));    // 'Void' records are never used.
    if CDP_RARELY(prepend && !cdp_record_is_insertable(parent)) {
        assert(!prepend);
        return NULL;
    }

    cdpStore* store;
    if (parent->metarecord.withstore) {
        store = parent->children;
    } else {
        store = record_create_storage(parent->metarecord.storage, parent->basez);

        // Link parent record with its child storage.
        store->owner = parent;
        parent->children = store;
        parent->metarecord.withstore = 1;
    }
    store_check_auto_id(store, record);
    cdpRecord* child;

    // Add new record to parent store.
    switch (parent->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        child = list_add(parent->children, parent, prepend, record);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        child = array_add(parent->children, parent, prepend, record);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(!cdp_record_is_dictionary(parent));
        child = packed_q_add(parent->children, parent, prepend, record);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        child = rb_tree_add(parent->children, parent, record);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        child = octree_add(parent->children, parent, record);
        break;
      }
    }

    cdp_record_transfer(record, child);

    CDP_0(record);      // This avoids deleting children during move operations.

    // Update child.
    child->store = store;

    // Update parent.
    store->chdCount++;

    return child;
}


/*
    Adds/inserts a *copy* of the specified record into another record.
*/
cdpRecord* cdp_record_sorted_insert(cdpRecord* parent, cdpRecord* record, cdpCompare compare, void* context) {
    assert(cdp_record_is_normal(parent) && cdp_record_is_insertable(parent) && !cdp_record_is_void(record) && compare); // 'Void' records are never used.

    cdpStore* store;
    if (parent->metarecord.withstore) {
        store = parent->children;
    } else {
        store = record_create_storage(parent->metarecord.storage, parent->basez);

        // Link parent record with its child storage.
        store->owner = parent;
        parent->children = store;
        parent->metarecord.withstore = 1;
    }
    store_check_auto_id(store, record);
    cdpRecord* child;

    // Add new record to parent.
    switch (parent->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        child = list_sorted_insert(parent->children, record, compare, context);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        child = array_sorted_insert(parent->children, record, compare, context);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(parent->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        return NULL;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        child = rb_tree_sorted_insert(parent->children, record, compare, context);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        child = octree_sorted_insert(parent->children, record, compare, context);
        break;
      }
    }

    cdp_record_transfer(record, child);

    CDP_0(record);

    // Update child.
    child->store = store;

    // Update parent.
    store->chdCount++;

    return child;
}




/*
   Reads data from a record.
*/
void* cdp_record_read(const cdpRecord* record, size_t* capacity, size_t* size, void* data) {
    assert(!cdp_record_is_void(record));

    if (record->metarecord.type == CDP_TYPE_LINK) {
        return record->link;
    }
    if (record->metarecord.type == CDP_TYPE_AGENT) {
        return record->agent;
    }

    REC_DATATYPE_SELECT(record) {
      NONE: {
        assert(cdp_record_has_data(record));  // This shouldn't happen.
        break;
      }

      NEAR: {
        if (data) {
            assert(capacity && *capacity);
            memcpy(data, &record->_near, cdp_min(*capacity, sizeof(cdpValue)));
        }
        CDP_PTR_SEC_SET(capacity, sizeof(record->_near));
        CDP_PTR_SEC_SET(size, sizeof(cdpValue));
        return CDP_P(&record->_near);
      }

      DATA: {
        if (data) {
            assert(capacity && *capacity);
            memcpy(data, record->data->_data, cdp_min(*capacity, record->data->capacity));
        }
        CDP_PTR_SEC_SET(capacity, record->data->capacity);
        CDP_PTR_SEC_SET(size, record->data->size);
        return record->data->_data;
      }

      FAR: {
        if (data) {
            assert(capacity && *capacity);
            memcpy(data, record->data->_far, cdp_min(*capacity, record->data->capacity));
        }
        CDP_PTR_SEC_SET(capacity, record->data->capacity);
        CDP_PTR_SEC_SET(size, record->data->size);
        return record->data->_far;
      }
    } SELECTION_END;

    CDP_PTR_SEC_SET(size, 0);
    CDP_PTR_SEC_SET(capacity, 0);
    return NULL;
}


cdpValue cdp_record_read_value(const cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    if (record->metarecord.type == CDP_TYPE_LINK) {
        return (cdpValue) record->link;
    }
    if (record->metarecord.type == CDP_TYPE_AGENT) {
        return (cdpValue) record->agent;
    }

    REC_DATATYPE_SELECT(record) {
      NONE: {
        assert(cdp_record_has_data(record));  // This shouldn't happen.
        break;
      }
      NEAR: {
        return record->_near;
      }
      DATA: {
        assert(record->data->size >= sizeof(cdpValue));
        return record->data->_data[0];
      }
      FAR: {
        assert(record->data->size >= sizeof(cdpValue));
        return *(cdpValue*)record->data->_far;
      }
    } SELECTION_END;

    return (cdpValue){0};
}



/*
   Updates the data of a record.
*/
void* cdp_record_update(cdpRecord* record, size_t capacity, size_t size, cdpValue data, bool swap) {
    assert(cdp_record_is_normal(record) && capacity && size);

    // ToDo: re-grow buffer and capacities if needed.

    REC_DATATYPE_SELECT(record) {
      NONE: {
        assert(cdp_record_has_data(record));  // This shouldn't happen.
        break;
      }

      NEAR: {
        assert(capacity == sizeof(cdpValue));
        record->_near = data;
        return &record->_near;
      }

      DATA: {
        if (data.pointer) {
            assert(record->data->capacity == capacity);
            memcpy(record->data->_data, data.pointer, capacity);
        } else {
            memset(record->data->_data, 0, record->data->capacity);
        }
        record->data->size = size;
        return record->data->_data;
      }

      FAR: {
        if (swap) {
            assert(capacity && data.pointer);
            record->data->capacity = capacity;
            record->data->_far = data.pointer;
        } else if (data.pointer) {
            assert(record->data->capacity == capacity);
            memcpy(record->data->_far, data.pointer, capacity);
        } else {
            memset(record->data->_far, 0, record->data->capacity);
        }
        record->data->size = size;
        return record->data->_far;
      }
    } SELECTION_END;

    return NULL;
}




void cdp_record_data_delete(cdpRecord* record) {
    assert(cdp_record_is_normal(record));

    REC_DATATYPE_SELECT(record) {
      NONE: {
        return;
      }
      NEAR: {
        record->_near = (cdpValue){0};
        break;
      }
      DATA: {
        CDP_FREE(record->data);
        break;
      }
      FAR: {
        record->data->destructor(record->data->_far);
        CDP_FREE(record->data);
        break;
      }
    } SELECTION_END;

    record->metadata.recdata = CDP_RECDATA_NONE;
}


void cdp_record_data_reset(cdpRecord* record) {
    assert(cdp_record_is_normal(record));

    REC_DATATYPE_SELECT(record) {
      NONE: {
        assert(cdp_record_has_data(record));
        break;
      }
      NEAR: {
        record->_near = (cdpValue){0};
        return;
      }
      DATA: {
        memset(record->data->_data, 0, record->data->capacity);
        return;
      }
      FAR: {
        memset(record->data->_far, 0, record->data->capacity);
        return;
      }
    } SELECTION_END;
}




/*
    Constructs the full path (sequence of ids) for a given record, returning the depth.
    The cdpPath structure may be reallocated.
*/
bool cdp_record_path(const cdpRecord* record, cdpPath** path) {
    assert(record && path);

    cdpPath* tempPath;
    if (*path) {
        tempPath = *path;
        assert(tempPath->capacity);
    } else {
        tempPath = cdp_dyn_malloc(cdpPath, cdpID, MAX_DEPTH);
        tempPath->capacity = MAX_DEPTH;
        *path = tempPath;
    }
    tempPath->length = 0;

    // Traverse up the hierarchy to construct the path in reverse order
    for (const cdpRecord* current = record;  current;  current = cdp_record_parent(current)) {  // FixMe: assuming single parenthood for now.
        if (tempPath->length >= tempPath->capacity) {
            unsigned newCapacity = tempPath->capacity * 2;
            cdpPath* newPath = cdp_dyn_malloc(cdpPath, cdpID, newCapacity);     // FixMe: use realloc.

            memcpy(&newPath->id[tempPath->capacity], tempPath->id, tempPath->capacity);

            newPath->length   = tempPath->capacity;
            newPath->capacity = newCapacity;
            CDP_PTR_OVERW(tempPath, newPath);
            *path = tempPath;
        }

        // Prepend the current record's id to the path
        tempPath->id[tempPath->capacity - tempPath->length - 1] = current->metarecord.name;
        tempPath->length++;
    }

    return true;
}


/*
    Gets the first child record.
*/
cdpRecord* cdp_record_first(const cdpRecord* record) {
    if (!cdp_record_children(record))
        return NULL;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_first(record->children);
      }
      case CDP_STORAGE_ARRAY: {
        return array_first(record->children);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_first(record->children);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_first(record->children);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_first(record->children);
      }
    }

    return NULL;
}


/*
    Gets the last child record.
*/
cdpRecord* cdp_record_last(const cdpRecord* record) {
    if (!cdp_record_children(record))
        return NULL;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_last(record->children);
      }
      case CDP_STORAGE_ARRAY: {
        return array_last(record->children);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_last(record->children);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_last(record->children);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_last(record->children);
      }
   }

    return NULL;
}


/*
    Retrieves a child record by its id.
*/
cdpRecord* cdp_record_find_by_name(const cdpRecord* record, cdpID name) {
    assert(cdp_id_valid(name));

    if (!cdp_record_children(record))
        return NULL;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_name(record->children, name);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_name(record->children, name, record);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_find_by_name(record->children, name);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_name(record->children, name, record);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_name(record->children, name, record);
      }
    }

    return NULL;
}



/*
    Finds a child record based on specified key.
*/
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context) {
    assert(!cdp_record_is_void(key) && compare);

    if (!cdp_record_children(record))
        return NULL;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_key(record->children, key, compare, context);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_key(record->children, key, compare, context);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(record->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_key(record->children, key, compare, context);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_key(record->children, key, compare, context);
      }
    }

    return NULL;
}


/*
    Gets the record at index position from book.
*/
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t position) {
    if (position >= cdp_record_children(record))
        return NULL;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_position(record->children, position);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_position(record->children, position);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_find_by_position(record->children, position);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_position(record->children, position, record);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_position(record->children, position, record);
      }
    }

    return NULL;
}


/*
    Gets the record by its path from start record.
*/
cdpRecord* cdp_record_find_by_path(const cdpRecord* start, const cdpPath* path) {
    assert(!cdp_record_is_void(start) && path && path->length);
    if (!cdp_record_children(start))
        return NULL;
    const cdpRecord* record = start;

    for (unsigned depth = 0;  depth < path->length;  depth++) {
        record = cdp_record_find_by_name(record, path->id[depth]);
        if (!record) return NULL;
    }

    return (cdpRecord*)record;
}





/*
    Retrieves the previous sibling of record.
*/
cdpRecord* cdp_record_prev(const cdpRecord* parent, cdpRecord* record) {
    assert(!cdp_record_is_void(record));
    if (!parent)
        parent = cdp_record_parent(record);
    assert(cdp_record_children(parent));

    switch (parent->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_prev(record);
      }
      case CDP_STORAGE_ARRAY: {
        return array_prev(parent->children, record);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_prev(parent->children, record);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_prev(record);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_prev(record);
      }
    }

    return NULL;
}


/*
    Retrieves the next sibling of record (sorted or unsorted).
*/
cdpRecord* cdp_record_next(const cdpRecord* parent, cdpRecord* record) {
    assert(!cdp_record_is_void(record));
    if (!parent)
        parent = cdp_record_parent(record);
    assert(cdp_record_children(parent));


    switch (parent->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_next(record);
      }
      case CDP_STORAGE_ARRAY: {
        return array_next(parent->children, record);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_next(parent->children, record);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_next(record);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_next(record);
      }
    }

    return NULL;
}




/*
    Retrieves the first/next child record by its id.
*/
cdpRecord* cdp_record_find_next_by_name(const cdpRecord* record, cdpID id, uintptr_t* childIdx) {
    assert(cdp_id_valid(id));

    if (!cdp_record_children(record))
        return NULL;

    if (cdp_record_is_dictionary(record) || !childIdx) {
        CDP_PTR_SEC_SET(childIdx, 0);
        return cdp_record_find_by_name(record, id);
    }

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_next_by_name(record->children, id, (cdpListNode**)childIdx);
      }
      case CDP_STORAGE_ARRAY: {
        return array_next_by_name(record->children, id, childIdx);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_next_by_name(record->children, id, (cdpPackedQNode**)childIdx);
      }
      case CDP_STORAGE_RED_BLACK_T: {    // Unused.
        break;
      }
      case CDP_STORAGE_OCTREE: {
        return octree_next_by_name(record->children, id, (cdpListNode**)childIdx);
      }
    }

    return NULL;
}


/*
    Gets the next record with the (same) id as specified for each branch.
*/
cdpRecord* cdp_record_find_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev) {
    assert(cdp_record_children(start) && path && path->length);
    if (!cdp_record_children(start))  return NULL;
    const cdpRecord* record = start;

    for (unsigned depth = 0;  depth < path->length;  depth++) {
        // FixMe: depth must be stored in a stack as well!
        // ...(pending)
        record = cdp_record_find_next_by_name(record, path->id[depth], prev);
        if (!record) return NULL;
    }

    return (cdpRecord*)record;
}


/*
    Traverses the children of a record, applying a function to each.
*/
bool cdp_record_traverse(cdpRecord* record, cdpTraverse func, void* context, cdpBookEntry* entry) {
    assert(!cdp_record_is_void(record) && func);

    size_t children = cdp_record_children(record);
    if (!children)
        return true;

    if (!entry)
        entry = cdp_alloca(sizeof(cdpBookEntry));
    CDP_0(entry);

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_traverse(record->children, record, func, context, entry);
      }
      case CDP_STORAGE_ARRAY: {
        return array_traverse(record->children, record, func, context, entry);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_traverse(record->children, record, func, context, entry);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_traverse(record->children, record, cdp_bitson(children) + 2, func, context, entry);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_traverse(record->children, record, cdp_bitson(children) + 2, func, context, entry);
      }
    }

    return true;
}


/*
    Traverses each child branch and sub-branch of a record, applying a function to each.
*/
bool cdp_record_deep_traverse(cdpRecord* record, cdpTraverse func, cdpTraverse endFunc, void* context, cdpBookEntry* entry) {
    assert(!cdp_record_is_void(record) && (func || endFunc));

    if (!cdp_record_children(record))
        return true;

    bool ok = true;
    cdpRecord* child;
    unsigned depth = 0;
    if (!entry)
        entry = cdp_alloca(sizeof(cdpBookEntry));
    CDP_0(entry);
    entry->parent = record;
    entry->record = cdp_record_first(record);

    // Non-recursive version of branch descent:
    size_t stackSize = MAX_DEPTH * sizeof(cdpBookEntry);
    cdpBookEntry* stack = (MAX_DEPTH > CDP_MAX_FAST_STACK_DEPTH)?  cdp_malloc(stackSize):  cdp_alloca(stackSize);
    for (;;) {
        // Ascend to parent if no more siblings in branch.
        if (!entry->record) {
            if CDP_RARELY(!depth)  break;    // endFunc is never called on root book.
            depth--;

            if (endFunc) {
                ok = endFunc(&stack[depth], context);
                if (!ok)  break;
            }

            // Next record.
            entry->record   = stack[depth].next;
            entry->parent   = stack[depth].parent;
            entry->prev     = stack[depth].record;
            entry->next     = NULL;
            entry->position = stack[depth].position + 1;
            entry->depth    = depth;
            continue;
        }

      NEXT_SIBLING:   // Get sibling
        switch (entry->parent->metarecord.storage) {
          case CDP_STORAGE_LINKED_LIST: {
            entry->next = list_next(entry->record);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            entry->next = array_next(entry->record->store, entry->record);
            break;
          }
          case CDP_STORAGE_PACKED_QUEUE: {
            entry->next = packed_q_next(entry->record->store, entry->record);
            break;
          }
          case CDP_STORAGE_RED_BLACK_T: {
            entry->next = rb_tree_next(entry->record);
            break;
          }
          case CDP_STORAGE_OCTREE: {
            entry->next = octree_next(entry->record);
            break;
          }
        }

        if (func) {
            ok = func(entry, context);
            if (!ok)  break;
        }

        // Descent to children if it's a book.
        if (cdp_record_children(entry->record)
        && ((child = cdp_record_first(entry->record)))) {
            assert(depth < MAX_DEPTH);

            stack[depth++]  = *entry;

            entry->parent   = entry->record;
            entry->record   = child;
            entry->prev     = NULL;
            entry->position = 0;
            entry->depth    = depth;

            goto NEXT_SIBLING;
        }

        // Next record.
        entry->prev   = entry->record;
        entry->record = entry->next;
        entry->position += 1;
    }

    if (MAX_DEPTH > CDP_MAX_FAST_STACK_DEPTH)
        cdp_free(stack);

    return ok;
}




/*
    Converts an unsorted record into a dictionary.
*/
void cdp_record_to_dictionary(cdpRecord* record) {
    if (cdp_record_is_dictionary(record))
        return;
    else
        record->metarecord.dictionary = 1;

    if (cdp_record_children(record) <= 1)
        return;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_sort(record->children, record_compare_by_name, NULL);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_sort(record->children, record_compare_by_name, NULL);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(record->metarecord.storage != CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {    // Unneeded.
        break;
      }
      case CDP_STORAGE_OCTREE: {
        assert(record->metarecord.storage != CDP_STORAGE_OCTREE);    // Unsupported.
        break;
      }
    }
}


/*
    Sorts unsorted records according to user defined function.
*/
void cdp_record_sort(cdpRecord* record, cdpCompare compare, void* context) {
    assert(!cdp_record_is_void(record) && !cdp_record_is_dictionary(record) && compare);

    if (cdp_record_children(record) <= 1)
        return;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_sort(record->children, compare, context);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_sort(record->children, compare, context);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(record->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        // ToDo: re-sort RB-tree.
        assert(record->metarecord.storage != CDP_STORAGE_RED_BLACK_T);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        // ToDo: re-sort RB-tree.
        assert(record->metarecord.storage != CDP_STORAGE_OCTREE);
        break;
      }
    }
}


/*
    De-initiates a record.
*/
void cdp_record_finalize(cdpRecord* record) {
    assert(!cdp_record_is_void(record) && !cdp_record_is_shadowed(record));

    // Delete storage (and children)
    if (cdp_record_with_store(record)) {

        // ToDo: clean shadow.

        switch (record->metarecord.storage) {
          case CDP_STORAGE_LINKED_LIST: {
            cdpList* list = record->children;
            list_del_all_children(list);
            list_del(list);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            cdpArray* array = record->children;
            array_del_all_children(array);
            array_del(array);
            break;
          }
          case CDP_STORAGE_PACKED_QUEUE: {
            cdpPackedQ* pkdq = record->children;
            packed_q_del_all_children(pkdq);
            packed_q_del(pkdq);
            break;
          }
          case CDP_STORAGE_RED_BLACK_T: {
            cdpRbTree* tree = record->children;
            rb_tree_del_all_children(tree);
            rb_tree_del(tree);
            break;
          }
          case CDP_STORAGE_OCTREE: {
            cdpOctree* octree = record->children;
            octree_del_all_children(octree);
            octree_del(octree);
            break;
          }
        }
    }

    // Delete value
    if (record->metadata.recdata != CDP_RECDATA_NEAR) {
        if (record->metadata.recdata == CDP_RECDATA_FAR) {
            record->data->destructor(record->data->_far);
        cdp_free(record->data);
    }   }

    // ToDo: deal with link/agent here.

    // ToDo: unlink from 'self' list.
}


/*
    Removes last child from record.
*/
bool cdp_record_child_take(cdpRecord* record, cdpRecord* target) {
    assert(!cdp_record_is_void(record) && target);

    cdpStore* store = CDP_CHD_STORE(record->children);
    if (!store->chdCount)
        return false;

    // Remove this record from its parent (re-organizing siblings).
    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_take(record->children, target);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_take(record->children, target);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_take(record->children, target);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_take(record->children, target);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_take(record->children, target);
        break;
      }
    }

    store->chdCount--;

    return true;
}


/*
    Removes first child from record.
*/
bool cdp_record_child_pop(cdpRecord* record, cdpRecord* target) {
    assert(!cdp_record_is_void(record) && target);

    cdpStore* store = CDP_CHD_STORE(record->children);
    if (!store->chdCount)
        return false;

    // Remove this record from its parent (re-organizing siblings).
    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_pop(record->children, target);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_pop(record->children, target);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_pop(record->children, target);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_pop(record->children, target);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_pop(record->children, target);
        break;
      }
    }

    store->chdCount--;

    return true;
}


/*
    Deletes a record and all its children re-organizing (sibling) storage
*/
void cdp_record_remove(cdpRecord* record, cdpRecord* target) {
    assert(record && !cdp_record_is_shadowed(record) && record != &CDP_ROOT);

    cdpStore* store = cdp_record_par_store(record);
    cdpRecord*  parent = store->owner;

    if (target)
        cdp_record_transfer(record, target);  // Save record.
    else
        cdp_record_finalize(record);          // Delete record (along children, if any).

    // Remove this record from its parent (re-organizing siblings).
    switch (parent->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_remove_record(parent->children, record);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_remove_record(parent->children, record);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_remove_record(parent->children, record);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_remove_record(parent->children, record);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_remove_record(parent->children, record);
        break;
      }
    }

    store->chdCount--;
}


/*
    Deletes all children of a record.
*/
void cdp_record_branch_reset(cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    cdpStore* store = CDP_CHD_STORE(record->children);
    if (!store->chdCount)
        return;

    switch (record->metarecord.storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_del_all_children(record->children);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_del_all_children(record->children);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_del_all_children(record->children);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_del_all_children(record->children);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_del_all_children(record->children);
        break;
      }
    }

    store->chdCount = 0;
}




/*
    Encoding of names into 6-bit values.
*/
#define ACRON_MAX_CHARS     9

cdpID cdp_text_to_acronysm(const char *s) {
    assert(s && *s);

    while (*s == ' ') {
        s++;            // Trim leading spaces.
    }
    if (!*s)
        return 0;

    size_t len = strlen(s);
    while (len > 0  &&  s[len - 1] == ' ') {
        len--;          // Trim trailing spaces.
    }

    if (len > ACRON_MAX_CHARS)
        return 0;       // Limit to max allowed characters.

    cdpID coded = 0;
    for (size_t n = 0; n < len; n++) {
        char c = s[n];

        if (c < 0x20  ||  c > 0x5F)
            return 0;   // Uncodable characters.

        coded |= (cdpID)(c - 0x20) << (6 * ((ACRON_MAX_CHARS - 1) - n));    // Shift and encode each character.
    }

    return cdp_id_to_acronysm(coded);
}


size_t cdp_acronysm_to_text(cdpID acro, char s[10]) {
    assert(cdp_id_name_valid(acro));
    cdpID coded = cdp_id(acro);

    unsigned length;
    for (length = 0; length < ACRON_MAX_CHARS; length++) {
        char c = (char)((coded >> (6 * ((ACRON_MAX_CHARS - 1) - length))) & 0x3F);  // Extract 6 bits for each character (starting from the highest bits).

        s[length] = c + 0x20;   // Restore the original ASCII character.
    }
    s[length] = '\0';

    while (length > 0  &&  s[length - 1] == ' ') {
        s[--length] = '\0';     // Replace trailing spaces with null characters.
    }

    return length;
}




/*
    Encoding of names into 5-bit values.
*/
#define WORD_MAX_CHARS      11

cdpID cdp_text_to_word(const char *s) {
    assert(s && *s);

    while (*s == ' ') {
        s++;            // Trim leading spaces.
    }
    if (!*s)
        return 0;

    size_t len = strlen(s);
    while (len > 0  &&  s[len - 1] == ' ') {
        len--;          // Trim trailing spaces.
    }
    if (len > WORD_MAX_CHARS)
        return 0;       // Limit to max allowed characters.

    cdpID coded = 0;
    for (size_t n = 0; n < len; n++) {
        char c = s[n];

        uint8_t encoded_char;
        if (c >= 0x61  &&  c <= 0x7A) {
            encoded_char = c - 0x61 + 1;        // Map 'a'-'z' to 1-26.
        } else switch (c) {
          case ' ': encoded_char = 0;   break;  // Treat space as 0.
          case ':': encoded_char = 27;  break;
          case '_': encoded_char = 28;  break;
          case '-':`encoded_char = 29;  break;
          case '.':`encoded_char = 30;  break;
          case '/':`encoded_char = 31;  break;

          default:
            return 0;   // Uncodable characters.
        }

         coded |= (cdpID)encoded_char << (5 * ((WORD_MAX_CHARS - 1) - n));  // Shift and encode each character.
    }

    return cdp_id_to_word(coded);
}


size_t cdp_word_to_text(cdpID coded, char s[12]) {
    assert(cdp_id_name_valid(coded));

    const char* translation_table = ":_-./";    // Reverse translation table for values 27-31.
    unsigned length;
    for (length = 0; length < WORD_MAX_CHARS; length++) {
        uint8_t encoded_char = (coded >> (5 * ((WORD_MAX_CHARS - 1) - length))) & 0x1F; // Extract each 5-bit segment, starting from the most significant bits.

        if (encoded_char >= 1  &&  encoded_char <= 26) {
            s[length] = (char)(encoded_char - 1 + 0x61);            // 'a' - 'z'.
        } else if (encoded_char == 0) {
            s[length] = ' ';                                        // Space.
        } else if (encoded_char >= 27  &&  encoded_char <= 31) {
            s[length] = translation_table[encoded_char - 27];       // Map 27-31 using table.
        }
    }
    s[length] = '\0';

    while (length > 0  &&  s[length - 1] == ' ') {
        s[--length] = '\0';     // Replace trailing spaces with null characters.
    }

    return length;
}

