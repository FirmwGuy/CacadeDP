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
    cdpID keyName = cdp_record_get_id(key);
    cdpID recName = cdp_record_get_id(rec);
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
    Initiates the record system
*/
void cdp_record_system_initiate(void) {
    cdp_record_initialize_dictionary(&CDP_ROOT, cdp_id_to_word(CDP_WORD_ROOT), CDP_STORAGE_RED_BLACK_T, 0);   // The root dictionary is the same as "/" in text paths.
}


/*
    Shutdowns the record system
*/
void cdp_record_system_shutdown(void) {
    cdp_record_finalize(&CDP_ROOT);
}




/*
    Create a new data store for records
*/

#define VALUE_CAP_MIN       (sizeof((cdpData){}.value))
#define DATA_HEAD_SIZE      (sizeof(cdpData) - VALUE_CAP_MIN)

cdpData* cdp_data_new(  cdpID domain, cdpID tag,
                        cdpID attribute, unsigned datatype, bool writable,
                        void** dataloc, cdpValue value, ...  ) {
    assert(cdp_id_text_valid(domain) && cdp_id_text_valid(tag) && (datatype < CDP_DATATYPE_COUNT));

    cdpData* data;
    void*    address;
    va_list  args;
    va_start(args, value);

    switch (datatype) {
      case CDP_DATATYPE_VALUE: {
        size_t size     = va_arg(args, size_t);
        size_t capacity = va_arg(args, size_t);
        assert(capacity  &&  (capacity >= size));

        size_t dmax  = cdp_max(VALUE_CAP_MIN, capacity);
        size_t alocz = DATA_HEAD_SIZE + dmax;

        if (size) {
            if (capacity > VALUE_CAP_MIN) {
                data = cdp_malloc(alocz);
                memset(data, 0, DATA_HEAD_SIZE);
                memcpy(data->value, value.pointer, size);
            } else {
                data = cdp_malloc0(alocz);
                data->value = value;
            }
        } else {
            data = cdp_malloc0(alocz);
            size = capacity;
        }
        data->capacity = dmax;
        data->size     = size;

        address = data->value;
        break;
      }

      case CDP_DATATYPE_DATA: {
        size_t size       = va_arg(args, size_t);
        size_t capacity   = va_arg(args, size_t);
        cdpDel destructor = va_arg(args, cdpDel);
        assert(capacity  &&  (capacity >= size));

        data = cdp_malloc0(sizeof(cdpData));

        if (destructor) {
            data->data       = value.pointer;
            data->destructor = destructor;
        } else {
            if (size) {
                data->data = cdp_malloc(capacity);
                memcpy(data->data, value.pointer, size);
            } else {
                data->data = cdp_malloc0(capacity);
            }
            data->destructor = cdp_free;
        }
        data->capacity = capacity;
        data->size     = size;

        address = data->data;
        break;
      }

      case CDP_DATATYPE_HANDLE: {
        cdpRecord* handle  = va_arg(args, cdpRecord*);
        cdpRecord* library = va_arg(args, cdpRecord*);
        assert(handle && library);

        data = cdp_malloc0(sizeof(cdpData));

        data->handle  = handle;
        data->library = library;

        address = cdp_record_data(handle);
        break;
      }

      case CDP_DATATYPE_STREAM: {
        cdpRecord* stream  = va_arg(args, cdpRecord*);
        cdpRecord* library = va_arg(args, cdpRecord*);
        assert(handle && library);

        data = cdp_malloc0(sizeof(cdpData));

        data->stream  = stream;
        data->library = library;

        address = cdp_record_data(stream);
        break;
      }
    }

    va_end(args);

    data->domain    = domain;
    data->tag       = tag;
    data->attribute = attribute;
    data->datatype  = datatype;
    data->writable  = writable;

    CDP_PTR_SEC_SET(dataloc, address);

    return data;
}


void cdp_data_del(cdpData* data) {
    assert(data);

    switch (data->datatype) {
      case CDP_DATATYPE_DATA: {
        if (data->destructor)
            data->destructor(data->data);
        break;
      }
      case CDP_DATATYPE_HANDLE:
      case CDP_DATATYPE_STREAM: {
        // ToDo: unref handle?
        break;
      }
    }

    cdp_free(data);
}




/*
    Creates a new children store for records
*/
cdpStore* cdp_store_new(  cdpID domain, cdpID tag,
                          unsigned storage, unsigned indexing, size_t capacity
                          cdpCompare compare  ) {
    assert(cdp_id_text_valid(domain) && cdp_id_text_valid(tag) && (storage < CDP_STORAGE_COUNT) && (indexing < CDP_INDEX_COUNT));

    cdpStore* store;

    switch (storage) {
      case CDP_STORAGE_LINKED_LIST: {
        store = list_new();
        break;
      }
      case CDP_STORAGE_ARRAY: {
        assert(capacity);
        store = array_new(capacity);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(capacity  &&  (indexing == CDP_INDEX_BY_INSERTION));
        store = packed_q_new(capacity);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        assert(indexing != CDP_INDEX_BY_INSERTION);
        store = rb_tree_new();
        break;
      }
      case CDP_STORAGE_OCTREE: {
        assert(indexing == CDP_INDEX_BY_FUNCTION);
        store = octree_new();
        break;
      }
    }

    store->domain   = domain;
    store->tag      = tag;
    store->storage  = storage;
    store->indexing = indexing;
    store->autoid   = 1;

    if (indexing == CDP_INDEX_BY_FUNCTION
     || indexing == CDP_INDEX_BY_HASH) {
        assert(compare);
        store->compare = compare;
    }

    return store;
}


void cdp_store_del(cdpStore* store) {
    assert(store);

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        cdpList* list = store;
        list_del_all_children(list);
        list_del(list);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        cdpArray* array = store;
        array_del_all_children(array);
        array_del(array);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        cdpPackedQ* pkdq = store;
        packed_q_del_all_children(pkdq);
        packed_q_del(pkdq);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        cdpRbTree* tree = store;
        rb_tree_del_all_children(tree);
        rb_tree_del(tree);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        cdpOctree* octree = store;
        octree_del_all_children(octree);
        octree_del(octree);
        break;
      }
    }
}


void cdp_store_delete_children(cdpStore* store) {
    assert(store);

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_del_all_children(store);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_del_all_children(store);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_del_all_children(store);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_del_all_children(store);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_del_all_children(store);
        break;
      }
    }

    store->chdCount = 0;
    store->autoid   = 1;
}


/*
    Assign auto-id if necessary
*/

static inline void store_check_auto_id(cdpStore* store, cdpRecord* child) {
    if (cdp_record_id_is_pending(child)) {
        cdp_record_set_name(child, cdp_id_to_numeric(store->autoid++));
    }
    // FixMe: if otherwise.
}




/*
    Initiates a record structure
*/
void cdp_record_initialize(cdpRecord* record, unsigned type, cdpID name, cdpData* data, cdpStore* store) {
    assert(record && cdp_id_valid(name) && (type < CDP_TYPE_COUNT));
    assert((data? cdp_id_text_valid(data->name): true)  &&  (store? cdp_id_text_valid(store->name): true));

    //CDP_0(record);

    record->metarecord.type = type;
    record->metarecord.name = name;
    record->data  = data;
    record->store = store;
}


/*
    Creates a deep copy of record and all its data
*/
void cdp_record_initialize_clone(cdpRecord* clone, cdpID nameID, cdpRecord* record) {
    assert(clone && cdp_record_is_normal(record));

    assert(!cdp_record_has_data(record) && !cdp_record_has_store(record));

    // ToDo: Clone data Pending!

    CDP_0(clone);

    clone->metarecord = record->metarecord;
    //clone->metarecord.name = ...
}


/*
    De-initiates a record
*/
void cdp_record_finalize(cdpRecord* record) {
    assert(!cdp_record_is_void(record) && !cdp_record_is_shadowed(record));

    switch (record->type) {
      case CDP_TYPE_NORMAL: {
        // Delete storage (and children)
        cdpStore* store = record->store;
        if (store) {
            // ToDo: clean shadow.

            cdp_store_del(store);
        }

        // Delete value
        cdpData* data = record->data;
        if (data) {
            cdp_data_del(data);
        }
        break;
      }

      case CDP_TYPE_LINK: {
        // ToDo: deal with linkage here.
        break;
      }

      case CDP_TYPE_LINK: {
        // ToDo: stop agent here.
        break;
      }
    }

    // ToDo: unlink from 'self' list.
}




/*
    Adds/inserts a *copy* of the specified record into another record
*/
cdpRecord* cdp_record_add(cdpRecord* record, cdpRecord* child, cdpValue context) {
    assert(cdp_record_has_store(record) && !cdp_record_is_void(child));

    cdpRecord* newchd;
    cdpStore* store = record->store;

    switch (store->indexing) {
      case CDP_INDEX_BY_INSERTION:
      {
        assert(store->chdCount >= context.size);

        switch (store->storage) {
          case CDP_STORAGE_LINKED_LIST: {
            newchd = list_insert(store, child, context.size);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            newchd = array_insert(store, record, child, context.size);
            break;
          }
          case CDP_STORAGE_PACKED_QUEUE: {
            newchd = packed_q_insert(store, record, child, context.size);
            break;
          }
          default: {
            assert(store->storage >= CDP_STORAGE_RED_BLACK_T);
            return NULL;
          }
        }
        break;
      }

      case CDP_INDEX_BY_NAME:
      {
        switch (store->storage) {
          case CDP_STORAGE_LINKED_LIST: {
            newchd = list_named_insert(store, child);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            newchd = array_named_insert(store, record, child);
            break;
          }
          case CDP_STORAGE_RED_BLACK_T: {
            newchd = rb_tree_named_insert(store, record, child);
            break;
          }
          default: {
            assert(!cdp_record_is_dictionary(record));
            return NULL;
          }
        }
        break;
      }

      case CDP_INDEX_BY_FUNCTION:
      case CDP_INDEX_BY_HASH:
      {
        switch (store->storage) {
          case CDP_STORAGE_LINKED_LIST: {
            newchd = list_sorted_insert(store, child, store->compare, context.pointer);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            newchd = array_sorted_insert(store, record, prepend, child);
            break;
          }
          case CDP_STORAGE_PACKED_QUEUE: {
            assert(parent->store.storage != CDP_STORAGE_PACKED_QUEUE);
            return NULL;
          }
          case CDP_STORAGE_RED_BLACK_T: {
            newchd = rb_tree_sorted_insert(store, record, child);
            break;
          }
          case CDP_STORAGE_OCTREE: {
            newchd = octree_sorted_insert(store, record, child);
            break;
          }
        }
        break;
      }
    }

    store_check_auto_id(store, newchd);

    //cdp_record_transfer(child, newchd);
    CDP_0(child);      // This avoids deleting children during move operations.

    newchd->parent = 1store;
    store->chdCount++;

    return newchd;
}


/*
    Appends/prepends a copy of record into another
*/
cdpRecord* cdp_record_append(cdpRecord* record, cdpRecord* child, bool prepend) {
    assert(cdp_record_has_store(record) && !cdp_record_is_void(child));

    cdpRecord* newchd;
    cdpStore* store = record->store;

    if (store->indexing != CDP_INDEX_BY_INSERTION)) {
        assert(store->indexing == CDP_INDEX_BY_INSERTION);
        return NULL;
    }
    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        newchd = list_append(store, child, prepend);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        newchd = array_append(store, parent, child, prepend);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        newchd = packed_q_append(store, parent, child, prepend);
        break;
      }
      default: {
        assert(store->storage >= CDP_STORAGE_RED_BLACK_T);
        return NULL;
      }
    }
    store_check_auto_id(store, newchd);

    //cdp_record_transfer(child, newchd);
    CDP_0(child);

    newchd->parent = store;
    store->chdCount++;

    return newchd;
}




/*
   Gets data address from a record
*/
void* cdp_record_data(const cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    switch (record->metarecord.type) {
      case CDP_TYPE_NORMAL:
      {
        if (!record->data)
            return NULL;

        switch (record->data.datatype) {
          case CDP_DATATYPE_VALUE: {
            return record->data->value;
          }

          case CDP_DATATYPE_DATA: {
            return record->data->data;
          }

          case CDP_DATATYPE_HANDLE:
          case CDP_DATATYPE_STREAM: {
            // ToDo: pending!
            break;
          }
        }

        return NULL;
      }

      case CDP_TYPE_LINK: {
        // ToDo: follow link.
        break;
      }

      default: {
        assert(record->metarecord.type == CDP_TYPE_NORMAL  ||  record->metarecord.type == CDP_TYPE_LINK);
      }
    }

    return NULL;
}


/*
   Updates the data of a record
*/
void* cdp_record_update(cdpRecord* record, size_t size, size_t capacity, cdpValue value, bool swap) {
    assert(cdp_record_has_data(record) && size && capacity);

    cdpData* data = record->data;
    if (!data->writable)
        return NULL;

    switch (data.datatype) {
      case CDP_DATATYPE_VALUE: {
        assert(data->capacity == capacity);
        data->value = value;
        data->size = size;
        return data->value;
      }

      case CDP_DATATYPE_DATA: {
        assert(value.pointer);
        if (swap) {
            if (data->destructor)
                data->destructor(data->data);
            data->data = value.pointer;
            data->capacity = capacity;
        } else {
            assert(data->capacity >= capacity);
            memcpy(data->data, value.pointer, size);
        }
        data->size = size;
        return data->data;
      }

      case CDP_DATATYPE_HANDLE:
      case CDP_DATATYPE_STREAM: {
        // ToDo: pending!
        return NULL;
      }
    }

    return NULL;
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
    Gets the first child record
*/
cdpRecord* cdp_record_first(const cdpRecord* record) {
    if (!cdp_record_children(record))
        return NULL;

    cdpStore* store = record->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_first(store);
      }
      case CDP_STORAGE_ARRAY: {
        return array_first(store);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_first(store);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_first(store);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_first(store);
      }
    }

    return NULL;
}


/*
    Gets the last child record
*/
cdpRecord* cdp_record_last(const cdpRecord* record) {
    if (!cdp_record_children(record))
        return NULL;

    cdpStore* store = record->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_last(store);
      }
      case CDP_STORAGE_ARRAY: {
        return array_last(store);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_last(store);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_last(store);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_last(store);
      }
   }

    return NULL;
}


/*
    Retrieves a child record by its ID
*/
cdpRecord* cdp_record_find_by_name(const cdpRecord* record, cdpID name) {
    assert(cdp_id_valid(name));

    if (!cdp_record_children(record))
        return NULL;

    cdpStore* store = record->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_name(store, name);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_name(store, name, record);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_find_by_name(store, name);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_name(store, name, record);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_name(store, name, record);
      }
    }

    return NULL;
}



/*
    Finds a child record based on specified key
*/
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context) {
    assert(!cdp_record_is_void(key) && compare);

    if (!cdp_record_children(record))
        return NULL;

    cdpStore* store = record->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_key(store, key, compare, context);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_key(store, key, compare, context);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(record->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_key(store, key, compare, context);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_key(store, key, compare, context);
      }
    }

    return NULL;
}


/*
    Gets the record at index position from book
*/
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t position) {
    if (position >= cdp_record_children(record))
        return NULL;

    cdpStore* store = record->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_position(store, position);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_position(store, position);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_find_by_position(store, position);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_position(store, position, record);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_position(store, position, record);
      }
    }

    return NULL;
}


/*
    Gets the record by its path from start record
*/
cdpRecord* cdp_record_find_by_path(const cdpRecord* start, const cdpPath* path) {
    assert(!cdp_record_is_void(start) && path && path->length);
    if (!cdp_record_children(start))
        return NULL;
    cdpRecord* record = start;

    for (unsigned depth = 0;  depth < path->length;  depth++) {
        record = cdp_record_find_by_name(record, path->id[depth]);
        if (!record)
            return NULL;
    }

    return record;
}





/*
    Retrieves the previous sibling of record
*/
cdpRecord* cdp_record_prev(const cdpRecord* parent, cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    if (!parent)
        parent = cdp_record_parent(record);
    cdpStore* store = parent->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_prev(record);
      }
      case CDP_STORAGE_ARRAY: {
        return array_prev(store, record);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_prev(store, record);
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
    Retrieves the next sibling of record (sorted or unsorted)
*/
cdpRecord* cdp_record_next(const cdpRecord* parent, cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    if (!parent)
        parent = cdp_record_parent(record);
    cdpStore* store = parent->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_next(record);
      }
      case CDP_STORAGE_ARRAY: {
        return array_next(store, record);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_next(store, record);
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
    Retrieves the first/next child record by its ID
*/
cdpRecord* cdp_record_find_next_by_name(const cdpRecord* record, cdpID id, uintptr_t* childIdx) {
    assert(cdp_id_valid(id));

    if (!cdp_record_children(record))
        return NULL;

    if (cdp_record_is_dictionary(record) || !childIdx) {
        CDP_PTR_SEC_SET(childIdx, 0);
        return cdp_record_find_by_name(record, id);
    }

    cdpStore* store = record->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_next_by_name(store, id, (cdpListNode**)childIdx);
      }
      case CDP_STORAGE_ARRAY: {
        return array_next_by_name(store, id, childIdx);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_next_by_name(store, id, (cdpPackedQNode**)childIdx);
      }
      case CDP_STORAGE_RED_BLACK_T: {    // Unused.
        break;
      }
      case CDP_STORAGE_OCTREE: {
        return octree_next_by_name(store, id, (cdpListNode**)childIdx);
      }
    }

    return NULL;
}


/*
    Gets the next record with the (same) ID as specified for each branch
*/
cdpRecord* cdp_record_find_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev) {
    assert(cdp_record_children(start) && path && path->length);
    if (!cdp_record_children(start))  return NULL;
    cdpRecord* record = start;

    for (unsigned depth = 0;  depth < path->length;  depth++) {
        // FixMe: depth must be stored in a stack as well!
        // ...(pending)
        record = cdp_record_find_next_by_name(record, path->id[depth], prev);
        if (!record)
            return NULL;
    }

    return record;
}


/*
    Traverses the children of a record, applying a function to each one
*/
bool cdp_record_traverse(cdpRecord* record, cdpTraverse func, void* context, cdpEntry* entry) {
    assert(!cdp_record_is_void(record) && func);

    size_t children = cdp_record_children(record);
    if (!children)
        return true;

    if (!entry)
        entry = cdp_alloca(sizeof(cdpEntry));
    CDP_0(entry);

    cdpStore* store = record->store;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_traverse(store, record, func, context, entry);
      }
      case CDP_STORAGE_ARRAY: {
        return array_traverse(store, record, func, context, entry);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_traverse(store, record, func, context, entry);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_traverse(store, record, cdp_bitson(children) + 2, func, context, entry);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_traverse(store, record, children, func, context, entry);
      }
    }

    return true;
}


/*
    Traverses each child branch and *sub-branch* of a record, applying a function to each one
*/
bool cdp_record_deep_traverse(cdpRecord* record, cdpTraverse func, cdpTraverse endFunc, void* context, cdpEntry* entry) {
    assert(!cdp_record_is_void(record) && (func || endFunc));

    if (!cdp_record_children(record))
        return true;

    bool ok = true;
    cdpRecord* child;
    unsigned depth = 0;
    if (!entry)
        entry = cdp_alloca(sizeof(cdpEntry));
    CDP_0(entry);
    entry->parent = record;
    entry->record = cdp_record_first(record);

    // Non-recursive version of branch descent:
    size_t stackSize = MAX_DEPTH * sizeof(cdpEntry);
    cdpEntry* stack = (MAX_DEPTH > CDP_MAX_FAST_STACK_DEPTH)?  cdp_malloc(stackSize):  cdp_alloca(stackSize);
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
        switch (entry->parent->store.storage) {
          case CDP_STORAGE_LINKED_LIST: {
            entry->next = list_next(entry->record);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            entry->next = array_next(entry->parent->store, entry->record);
            break;
          }
          case CDP_STORAGE_PACKED_QUEUE: {
            entry->next = packed_q_next(entry->parent->store, entry->record);
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
    Converts an unsorted record into a dictionary
*/
void cdp_record_to_dictionary(cdpRecord* record) {
    assert(cdp_record_has_store(record));

    cdpStore* store = record->store;

    if (store->storage == CDP_INDEX_BY_NAME)
        return;
    else
        store->storage = CDP_INDEX_BY_NAME;

    if (store->chdCount <= 1)
        return;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_sort(store, record_compare_by_name, NULL);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_sort(store, record_compare_by_name, NULL);
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
    Sorts unsorted records according to user defined function
*/
void cdp_record_sort(cdpRecord* record, cdpCompare compare, void* context) {
    assert(cdp_record_has_store(record) && compare);

    cdpStore* store = record->store;
    store->compare = compare;

    if (store->storage == CDP_INDEX_BY_FUNCTION)
        return;
    else
        store->storage = CDP_INDEX_BY_FUNCTION;     // FixMe: by hash?

    if (store->chdCount <= 1)
        return;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_sort(store, compare, context);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_sort(store, compare, context);
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
    Removes last child from record
*/
bool cdp_record_child_take(cdpRecord* record, cdpRecord* target) {
    assert(!cdp_record_is_void(record) && target);

    cdpStore* store = record->store;
    if (!store->chdCount)
        return false;

    // Remove this record from its parent (re-organizing siblings).
    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_take(store, target);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_take(store, target);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_take(store, target);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_take(store, target);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_take(store, target);
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

    cdpStore* store = record->store;
    if (!store->chdCount)
        return false;

    // Remove this record from its parent (re-organizing siblings).
    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_pop(store, target);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_pop(store, target);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_pop(store, target);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_pop(store, target);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_pop(store, target);
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

    cdpStore*  store  = record->store;
    cdpRecord* parent = store->owner;

    if (target)
        cdp_record_transfer(record, target);  // Save record.
    else
        cdp_record_finalize(record);          // Delete record (along children, if any).

    // Remove this record from its parent (re-organizing siblings).
    switch (store->storage) {
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

