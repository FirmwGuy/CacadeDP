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
    cdp_record_initialize_dictionary(&CDP_ROOT, CDP_WORD_ROOT, CDP_WORD_ROOT, CDP_WORD_ROOT, CDP_STORAGE_RED_BLACK_T);   // The root dictionary is the same as "/" in text paths.
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
                data->value[0] = value;
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
        assert(stream && library);

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
   Gets data address
*/
void* cdp_data(const cdpData* data) {
    assert(cdp_data_valid(data));

    switch (data->datatype) {
      case CDP_DATATYPE_VALUE: {
        return CDP_P(data->value);
      }

      case CDP_DATATYPE_DATA: {
        return data->data;
      }

      case CDP_DATATYPE_HANDLE:
      case CDP_DATATYPE_STREAM: {
        // ToDo: pending!
        break;
      }
    }

    return NULL;
}


/*
   Updates the data
*/
static inline void* cdp_data_update(cdpData* data, size_t size, size_t capacity, cdpValue value, bool swap) {
    assert(cdp_data_valid(data) && size && capacity);

    if (!data->writable)
        return NULL;

    switch (data->datatype) {
      case CDP_DATATYPE_VALUE: {
        assert(data->capacity >= capacity);
        data->value[0] = value;
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
    Creates a new children store for records
*/
cdpStore* cdp_store_new(cdpID domain, cdpID tag, unsigned storage, unsigned indexing, ...) {
    assert(cdp_id_text_valid(domain) && cdp_id_text_valid(tag) && (storage < CDP_STORAGE_COUNT) && (indexing < CDP_INDEX_COUNT));

    cdpStore* store;
    va_list  args;
    va_start(args, indexing);

    switch (storage) {
      case CDP_STORAGE_LINKED_LIST: {
        store = (cdpStore*) list_new();
        break;
      }
      case CDP_STORAGE_ARRAY: {
        size_t capacity = va_arg(args, size_t);
        assert(capacity);
        store = (cdpStore*) array_new(capacity);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        size_t capacity = va_arg(args, size_t);
        assert(capacity  &&  (indexing == CDP_INDEX_BY_INSERTION));
        store =(cdpStore*) packed_q_new(capacity);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        assert(indexing != CDP_INDEX_BY_INSERTION);
        store = (cdpStore*) rb_tree_new();
        break;
      }
      case CDP_STORAGE_OCTREE: {
        float*   center  = va_arg(args, float*);
        cdpValue subwide = va_arg(args, cdpValue);
        assert(center  &&  (indexing == CDP_INDEX_BY_FUNCTION));
        cdpOctreeBound bound;
        bound.subwide = subwide.float32;
        memcpy(&bound.center, center, sizeof(bound.center));
        store = (cdpStore*) octree_new(&bound);
        break;
      }
    }

    if (indexing == CDP_INDEX_BY_FUNCTION
     || indexing == CDP_INDEX_BY_HASH) {
        cdpCompare compare = va_arg(args, cdpCompare);
        assert(compare);
        store->compare = compare;
    }

    va_end(args);

    store->domain   = domain;
    store->tag      = tag;
    store->storage  = storage;
    store->indexing = indexing;
    store->writable = true;
    store->autoid   = 1;

    return store;
}


void cdp_store_del(cdpStore* store) {
    assert(cdp_store_valid(store));

    // ToDo: cleanup shadows.

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_del_all_children((cdpList*) store);
        list_del((cdpList*) store);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_del_all_children((cdpArray*) store);
        array_del((cdpArray*) store);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_del_all_children((cdpPackedQ*) store);
        packed_q_del((cdpPackedQ*) store);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_del_all_children((cdpRbTree*) store);
        rb_tree_del((cdpRbTree*) store);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_del_all_children((cdpOctree*) store);
        octree_del((cdpOctree*) store);
        break;
      }
    }
}


void cdp_store_delete_children(cdpStore* store) {
    assert(cdp_store_valid(store));

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_del_all_children((cdpList*) store);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_del_all_children((cdpArray*) store);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_del_all_children((cdpPackedQ*) store);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_del_all_children((cdpRbTree*) store);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_del_all_children((cdpOctree*) store);
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
    Adds/inserts a *copy* of the specified record into a store
*/
static inline cdpRecord* store_add_child(cdpStore* store, cdpValue context, cdpRecord* child) {
    assert(cdp_store_valid(store) && !cdp_record_is_void(child));

    if (!store->writable)
        return NULL;

    cdpRecord* record;

    switch (store->indexing) {
      case CDP_INDEX_BY_INSERTION:
      {
        assert(store->chdCount >= context.size);

        switch (store->storage) {
          case CDP_STORAGE_LINKED_LIST: {
            record = list_insert((cdpList*) store, child, context.size);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            record = array_insert((cdpArray*) store, child, context.size);
            break;
          }
          default: {
            assert(store->storage < CDP_STORAGE_PACKED_QUEUE);
            return NULL;
          }
        }
        break;
      }

      case CDP_INDEX_BY_NAME:
      {
        switch (store->storage) {
          case CDP_STORAGE_LINKED_LIST: {
            record = list_named_insert((cdpList*) store, child);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            record = array_named_insert((cdpArray*) store, child);
            break;
          }
          case CDP_STORAGE_RED_BLACK_T: {
            record = rb_tree_named_insert((cdpRbTree*) store, child);
            break;
          }
          default: {
            assert(store->indexing != CDP_INDEX_BY_NAME);
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
            record = list_sorted_insert((cdpList*) store, child, store->compare, context.pointer);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            record = array_sorted_insert((cdpArray*) store, child, store->compare, context.pointer);
            break;
          }
          case CDP_STORAGE_PACKED_QUEUE: {
            assert(store->storage != CDP_STORAGE_PACKED_QUEUE);
            return NULL;
          }
          case CDP_STORAGE_RED_BLACK_T: {
            record = rb_tree_sorted_insert((cdpRbTree*) store, child, store->compare, context.pointer);
            break;
          }
          case CDP_STORAGE_OCTREE: {
            record = octree_sorted_insert((cdpOctree*) store, child, store->compare, context.pointer);
            break;
          }
        }
        break;
      }
    }

    store_check_auto_id(store, record);

    //cdp_record_transfer(child, record);
    CDP_0(child);      // This avoids deleting children during move operations.

    record->parent = store;
    store->chdCount++;

    return record;
}


/*
    Appends/prepends a copy of record into store
*/
static inline cdpRecord* store_append_child(cdpStore* store, bool prepend, cdpRecord* child) {
    assert(cdp_store_valid(store) && !cdp_record_is_void(child));

    if (!store->writable)
        return NULL;

    cdpRecord* record;

    if (store->indexing != CDP_INDEX_BY_INSERTION) {
        assert(store->indexing == CDP_INDEX_BY_INSERTION);
        return NULL;
    }

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        record = list_append((cdpList*) store, child, prepend);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        record = array_append((cdpArray*) store, child, prepend);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        record = packed_q_append((cdpPackedQ*) store, child, prepend);
        break;
      }
      default: {
        assert(store->storage < CDP_STORAGE_RED_BLACK_T);
        return NULL;
      }
    }

    store_check_auto_id(store, record);

    //cdp_record_transfer(child, record);
    CDP_0(child);

    record->parent = store;
    store->chdCount++;

    return record;
}


/*
    Gets the first child record from store
*/
static inline cdpRecord* store_first_child(const cdpStore* store) {
    assert(cdp_store_valid(store));

    if (!store->chdCount)
        return NULL;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_first((cdpList*) store);
      }
      case CDP_STORAGE_ARRAY: {
        return array_first((cdpArray*) store);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_first((cdpPackedQ*) store);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_first((cdpRbTree*) store);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_first((cdpOctree*) store);
      }
    }
    return NULL;
}


/*
    Gets the last child record from store
*/
static inline cdpRecord* store_last_child(const cdpStore* store) {
    assert(cdp_store_valid(store));

    if (!store->chdCount)
        return NULL;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_last((cdpList*) store);
      }
      case CDP_STORAGE_ARRAY: {
        return array_last((cdpArray*) store);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_last((cdpPackedQ*) store);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_last((cdpRbTree*) store);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_last((cdpOctree*) store);
      }
    }
    return NULL;
}


/*
    Retrieves a child record by its ID
*/
static inline cdpRecord* store_find_child_by_name(const cdpStore* store, cdpID name) {
    assert(cdp_store_valid(store) && cdp_id_valid(name));

    if (!store->chdCount)
        return NULL;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_name((cdpList*) store, name);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_name((cdpArray*) store, name);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_find_by_name((cdpPackedQ*) store, name);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_name((cdpRbTree*) store, name);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_name((cdpOctree*) store, name);
      }
    }
    return NULL;
}



/*
    Finds a child record based on specified key
*/
static inline cdpRecord* store_find_child_by_key(const cdpStore* store, cdpRecord* key, cdpCompare compare, void* context) {
    assert(cdp_store_valid(store) && !cdp_record_is_void(key) && compare);

    if (!store->chdCount)
        return NULL;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_key((cdpList*) store, key, compare, context);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_key((cdpArray*) store, key, compare, context);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(store->storage != CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_key((cdpRbTree*) store, key, compare, context);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_key((cdpOctree*) store, key, compare, context);
      }
    }
    return NULL;
}


/*
    Gets the record at index position from store
*/
static inline cdpRecord* store_find_child_by_position(const cdpStore* store, size_t position) {
    assert(cdp_store_valid(store));

    if (store->chdCount <= position)
        return NULL;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_find_by_position((cdpList*) store, position);
      }
      case CDP_STORAGE_ARRAY: {
        return array_find_by_position((cdpArray*) store, position);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_find_by_position((cdpPackedQ*) store, position);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_find_by_position((cdpRbTree*) store, position);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_find_by_position((cdpOctree*) store, position);
      }
    }

    return NULL;
}


/*
    Retrieves the previous sibling of record
*/
static inline cdpRecord* store_prev_child(const cdpStore* store, cdpRecord* child) {
    assert(!cdp_record_is_void(child));

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_prev(child);
      }
      case CDP_STORAGE_ARRAY: {
        return array_prev((cdpArray*) store, child);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_prev((cdpPackedQ*) store, child);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_prev(child);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_prev(child);
      }
    }

    return NULL;
}


/*
    Retrieves the next sibling of record (sorted or unsorted)
*/
static inline cdpRecord* store_next_child(const cdpStore* store, cdpRecord* child) {
    assert(!cdp_record_is_void(child));

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_next(child);
      }
      case CDP_STORAGE_ARRAY: {
        return array_next((cdpArray*) store, child);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_next((cdpPackedQ*) store, child);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_next(child);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_next(child);
      }
    }

    return NULL;
}




/*
    Retrieves the first/next child record by its ID
*/
static inline cdpRecord* store_find_next_child_by_name(const cdpStore* store, cdpID id, uintptr_t* childIdx) {
    assert(cdp_store_valid(store) && cdp_id_valid(id));

    if (!store->chdCount)
        return NULL;

    if (store->indexing == CDP_INDEX_BY_NAME  ||  !childIdx) {
        CDP_PTR_SEC_SET(childIdx, 0);
        return store_find_child_by_name(store, id);
    }

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_next_by_name((cdpList*) store, id, (cdpListNode**)childIdx);
      }
      case CDP_STORAGE_ARRAY: {
        return array_next_by_name((cdpArray*) store, id, childIdx);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_next_by_name((cdpPackedQ*) store, id, (cdpPackedQNode**)childIdx);
      }
      case CDP_STORAGE_RED_BLACK_T: {    // Unused.
        break;
      }
      case CDP_STORAGE_OCTREE: {
        //return octree_next_by_name(store, id, (cdpListNode**)childIdx);
      }
    }

    return NULL;
}


/*
    Traverses all the children in a store, applying a function to each one
*/
static inline bool store_traverse(cdpStore* store, cdpTraverse func, void* context, cdpEntry* entry) {
    assert(cdp_store_valid(store) && func);

    size_t children = store->chdCount;
    if (!children)
        return true;

    if (!entry)
        entry = cdp_alloca(sizeof(cdpEntry));
    CDP_0(entry);

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        return list_traverse((cdpList*) store, func, context, entry);
      }
      case CDP_STORAGE_ARRAY: {
        return array_traverse((cdpArray*) store, func, context, entry);
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        return packed_q_traverse((cdpPackedQ*) store, func, context, entry);
      }
      case CDP_STORAGE_RED_BLACK_T: {
        return rb_tree_traverse((cdpRbTree*) store, cdp_bitson(children) + 2, func, context, entry);
      }
      case CDP_STORAGE_OCTREE: {
        return octree_traverse((cdpOctree*) store, func, context, entry);
      }
    }

    return true;
}


/*
    Converts an unsorted store into a dictionary
*/
static inline void store_to_dictionary(cdpStore* store) {
    assert(cdp_store_valid(store));

    if (store->storage == CDP_INDEX_BY_NAME)
        return;

    store->storage = CDP_INDEX_BY_NAME;

    if (store->chdCount <= 1)
        return;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_sort((cdpList*) store, record_compare_by_name, NULL);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_sort((cdpArray*) store, record_compare_by_name, NULL);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(store->storage != CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {    // Unneeded.
        break;
      }
      case CDP_STORAGE_OCTREE: {
        assert(store->storage != CDP_STORAGE_OCTREE);    // Unsupported.
        break;
      }
    }
}


/*
    Sorts unsorted store according to a user defined function
*/
static inline void store_sort(cdpStore* store, cdpCompare compare, void* context) {
    assert(cdp_store_valid(store) && compare);

    store->compare = compare;

    if (store->storage == CDP_INDEX_BY_FUNCTION)
        return;

    store->storage = CDP_INDEX_BY_FUNCTION;   // FixMe: by hash?

    if (store->chdCount <= 1)
        return;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_sort((cdpList*) store, compare, context);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_sort((cdpArray*) store, compare, context);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        assert(store->storage == CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        // ToDo: re-sort RB-tree.
        assert(store->storage != CDP_STORAGE_RED_BLACK_T);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        // ToDo: re-sort Octree.
        assert(store->storage != CDP_STORAGE_OCTREE);
        break;
      }
    }
}


/*
    Removes last child from store (re-organizing siblings)
*/
static inline bool store_take_record(cdpStore* store, cdpRecord* target) {
    assert(cdp_store_valid(store) && target);

    if (!store->chdCount || !store->writable)
        return NULL;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_take((cdpList*) store, target);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_take((cdpArray*) store, target);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_take((cdpPackedQ*) store, target);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_take((cdpRbTree*) store, target);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_take((cdpOctree*) store, target);
        break;
      }
    }

    store->chdCount--;

    return true;
}


/*
    Removes first child from store (re-organizing siblings)
*/
static inline bool store_pop_child(cdpStore* store, cdpRecord* target) {
    assert(cdp_store_valid(store) && target);

    if (!store->chdCount || !store->writable)
        return NULL;

    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_pop((cdpList*) store, target);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_pop((cdpArray*) store, target);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_pop((cdpPackedQ*) store, target);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_pop((cdpRbTree*) store, target);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_pop((cdpOctree*) store, target);
        break;
      }
    }

    store->chdCount--;

    return true;
}


/*
    Deletes a record and all its children re-organizing (sibling) storage
*/
static inline void store_remove_child(cdpStore* store, cdpRecord* record, cdpRecord* target) {
    assert(cdp_store_valid(store) && store->chdCount);

    if (target)
        cdp_record_transfer(record, target);  // Save record.
    else
        cdp_record_finalize(record);          // Delete record (along children, if any).

    // Remove this record from its parent (re-organizing siblings).
    switch (store->storage) {
      case CDP_STORAGE_LINKED_LIST: {
        list_remove_record((cdpList*) store, record);
        break;
      }
      case CDP_STORAGE_ARRAY: {
        array_remove_record((cdpArray*) store, record);
        break;
      }
      case CDP_STORAGE_PACKED_QUEUE: {
        packed_q_remove_record((cdpPackedQ*) store, record);
        break;
      }
      case CDP_STORAGE_RED_BLACK_T: {
        rb_tree_remove_record((cdpRbTree*) store, record);
        break;
      }
      case CDP_STORAGE_OCTREE: {
        octree_remove_record((cdpOctree*) store, record);
        break;
      }
    }

    store->chdCount--;
}




/*
    Initiates a record structure
*/
void cdp_record_initialize(cdpRecord* record, unsigned type, cdpID name, cdpData* data, cdpStore* store) {
    assert(record && cdp_id_valid(name) && (type < CDP_TYPE_COUNT));
    assert((data? cdp_data_valid(data): true)  &&  (store? cdp_store_valid(store): true));

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

    switch (record->metarecord.type) {
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

      case CDP_TYPE_AGENT: {
        // ToDo: stop agent here.
        break;
      }
    }

    // ToDo: unlink from 'self' list.
}




#define RECORD_FOLLOW_LINK_TO_STORE(record, store, ...)                        \
    assert(!cdp_record_is_void(record));                                       \
                                                                               \
    if (record->metarecord.type == CDP_TYPE_LINK)                              \
        record = cdp_link(CDP_P(record));                                      \
                                                                               \
    cdpStore* store = record->store;                                           \
    if (!store)                                                                \
        return __VA_ARGS__


/*
    Adds/inserts a *copy* of the specified record into another record
*/
cdpRecord* cdp_record_add(cdpRecord* record, cdpValue context, cdpRecord* child) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_add_child(store, context, child);
}


/*
    Appends/prepends a copy of record into another
*/
cdpRecord* cdp_record_append(cdpRecord* record, bool prepend, cdpRecord* child) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_append_child(store, prepend, child);
}




/*
   Gets data address from a record
*/
void* cdp_record_data(const cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    if (record->metarecord.type == CDP_TYPE_LINK)
        record = cdp_link(CDP_P(record));

    cdpData* data = record->data;
    if (!data)
        return NULL;

    return cdp_data(data);
}


/*
   Updates the data of a record
*/
void* cdp_record_update(cdpRecord* record, size_t size, size_t capacity, cdpValue value, bool swap) {
    assert(!cdp_record_is_void(record) && size && capacity);

    if (record->metarecord.type == CDP_TYPE_LINK)
        record = cdp_link(record);

    cdpData* data = record->data;
    if (!data) {
        assert(data);
        return NULL;
    }

    return cdp_data_update(data, size, capacity, value, swap);
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
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_first_child(store);
}


/*
    Gets the last child record
*/
cdpRecord* cdp_record_last(const cdpRecord* record) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_last_child(store);
}


/*
    Retrieves a child record by its ID
*/
cdpRecord* cdp_record_find_by_name(const cdpRecord* record, cdpID name) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_find_child_by_name(store, name);
}


/*
    Finds a child record based on specified key
*/
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_find_child_by_key(store, key, compare, context);
}


/*
    Gets the record at index position in branch
*/
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t position) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_find_child_by_position(store, position);
}


/*
    Gets the record by its path from start record
*/
cdpRecord* cdp_record_find_by_path(const cdpRecord* start, const cdpPath* path) {
    assert(!cdp_record_is_void(start) && path && path->length);
    if (!cdp_record_children(start))
        return NULL;
    cdpRecord* record = CDP_P(start);

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
cdpRecord* cdp_record_prev(const cdpRecord* record, cdpRecord* child) {
    if (!record)
        record = cdp_record_parent(child);
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_prev_child(store, child);
}


/*
    Retrieves the next sibling of record (sorted or unsorted)
*/
cdpRecord* cdp_record_next(const cdpRecord* record, cdpRecord* child) {
    if (!record)
        record = cdp_record_parent(child);
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_next_child(store, child);
}




/*
    Retrieves the first/next child record by its ID
*/
cdpRecord* cdp_record_find_next_by_name(const cdpRecord* record, cdpID id, uintptr_t* childIdx) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_find_next_child_by_name(store, id, childIdx);
}


/*
    Gets the next record with the (same) ID as specified for each branch
*/
cdpRecord* cdp_record_find_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev) {
    assert(cdp_record_children(start) && path && path->length);
    if (!cdp_record_children(start))  return NULL;
    cdpRecord* record = CDP_P(start);

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
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_traverse(store, func, context, entry);
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
        switch (entry->parent->store->storage) {
          case CDP_STORAGE_LINKED_LIST: {
            entry->next = list_next(entry->record);
            break;
          }
          case CDP_STORAGE_ARRAY: {
            entry->next = array_next((cdpArray*) entry->parent->store, entry->record);
            break;
          }
          case CDP_STORAGE_PACKED_QUEUE: {
            entry->next = packed_q_next((cdpPackedQ*) entry->parent->store, entry->record);
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
    RECORD_FOLLOW_LINK_TO_STORE(record, store);
    store_to_dictionary(store);
}


/*
    Sorts unsorted records according to user defined function
*/
void cdp_record_sort(cdpRecord* record, cdpCompare compare, void* context) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store);
    store_sort(store, compare, context);
}




/*
    Removes last child from record
*/
bool cdp_record_child_take(cdpRecord* record, cdpRecord* target) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_take_record(store, target);
}


/*
    Removes first child from record.
*/
bool cdp_record_child_pop(cdpRecord* record, cdpRecord* target) {
    RECORD_FOLLOW_LINK_TO_STORE(record, store, NULL);
    return store_pop_child(store, target);
}


/*
    Deletes a record and all its children re-organizing (sibling) storage
*/
void cdp_record_remove(cdpRecord* record, cdpRecord* target) {
    assert(record && record != &CDP_ROOT);
    cdpStore* store = record->parent;
    store_remove_child(store, record, target);
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
    assert(cdp_id_text_valid(acro));
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
          case '-': encoded_char = 29;  break;
          case '.': encoded_char = 30;  break;
          case '/': encoded_char = 31;  break;

          default:
            return 0;   // Uncodable characters.
        }

        coded |= (cdpID)encoded_char << (5 * ((WORD_MAX_CHARS - 1) - n));  // Shift and encode each character.
    }

    return cdp_id_to_word(coded);
}


size_t cdp_word_to_text(cdpID word, char s[12]) {
    assert(cdp_id_text_valid(word));
    cdpID coded = cdp_id(word);

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

