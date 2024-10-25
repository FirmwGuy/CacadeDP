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




#define CDP_MAX_FAST_STACK_DEPTH  16

unsigned MAX_DEPTH = CDP_MAX_FAST_STACK_DEPTH;     // FixMe: (used by path/traverse) better policy than a global for this.

#define cdpFunc     void*




static inline int record_compare_by_name(const cdpRecord* restrict key, const cdpRecord* restrict rec, void* unused) {
    cdpID keyName = cdp_record_get_name(key);
    cdpID recName = cdp_record_get_name(rec);
    return (keyName < recName)? -1: (keyName > recName);
}


#define STORE_TECH_SELECT(structure)                                           \
    assert((structure) < CDP_STORAGE_COUNT);                                   \
    static const void* const _chdStoreTech[] = {&&LINKED_LIST, &&ARRAY,        \
        &&PACKED_QUEUE, &&RED_BLACK_T};                                        \
    goto *_chdStoreTech[structure];                                            \
    do

#define REC_DATA_SELECT(record)                                                \
    static const void* const _recData[] = {&&NONE, &&NEAR, &&DATA, &&FAR};     \
    goto *_recData[(record)->data.recdata];                                    \
    do

#define SELECTION_END                                                          \
    while (0)




/*
    Include child storage techs
*/
#include "cdp_storage_linked_list.h"
#include "cdp_storage_dynamic_array.h"
#include "cdp_storage_packed_queue.h"
#include "cdp_storage_red_black_tree.h"




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
    cdp_record_initialize_dictionary(&CDP_ROOT, cdp_id_to_tag(CDP_DOMAIN_RECORD, CDP_TAG_ROOT), CDP_STORAGE_RED_BLACK_T, 0);   // The root dictionary is the same as "/" in text paths.
}


/*
    Shutdowns the record system.
*/
void cdp_record_system_shutdown(void) {
    cdp_record_finalize(&CDP_ROOT);
}




static inline void* record_create_storage(unsigned storage, unsigned capacity) {
    STORE_TECH_SELECT(storage) {
      LINKED_LIST: {
        return list_new();
      }
      ARRAY: {
        assert(capacity > 0);
        return array_new(capacity);
      }
      PACKED_QUEUE: {
        assert(capacity > 0);
        return packed_q_new(capacity);
      }
      RED_BLACK_T: {
        return rb_tree_new();
      }
    } SELECTION_END;
    return NULL;
}


void cdp_record_relink_storage(cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    cdpChdStore* store = record->children;
    assert(store);
    store->owner = record;           // Re-link record with its own children storage.
}


static inline void store_check_auto_id(cdpChdStore* parStore, cdpRecord* record) {
    if (CDP_AUTOID == record->metarecord.name) {
        cdp_record_set_name(record, cdp_id_to_numeric(parStore->autoid++));
    }
    // FixMe: if otherwise.
}




/*
    Initiates a record struct with the requested parameters.
*/
bool cdp_record_initialize( cdpRecord* record, cdpID name, unsigned type,
                            bool dictionary, unsigned storage, size_t basez,
                            cdpMetadata metadata, size_t capacity, size_t size,
                            cdpValue data, cdpDel destructor) {
    assert(record && cdp_id_valid(name) && type && (type < CDP_TYPE_COUNT) && (storage < CDP_STORAGE_COUNT));
    if (dictionary) {
        if CDP_RARELY(storage == CDP_STORAGE_PACKED_QUEUE) {
            assert(storage != CDP_STORAGE_PACKED_QUEUE);
            return false;
        }
    } else if (storage == CDP_STORAGE_RED_BLACK_T) {
        assert(dictionary);
        dictionary = true;
    }
    if (storage == CDP_STORAGE_ARRAY
    ||  storage == CDP_STORAGE_PACKED_QUEUE) {
        if CDP_RARELY(!basez) {
            assert(basez);
            return false;
    }   }

    //CDP_0(record);

    record->metarecord.name       = name;
    record->metarecord.type       = type;
    record->metarecord.dictionary = dictionary? 1: 0;
    record->metarecord.storage    = storage;

    record->metadata = metadata;
    record->basez    = basez;

    if (type == CDP_TYPE_LINK) {
        assert(data.link);
        record->link = data.link;
    }
    else if (type == CDP_TYPE_AGENT) {
        assert(data.agent);
        record->agent = data.agent;
    }
    else if (capacity) {
        if (destructor) {
            assert(data.pointer && size);

            record->data = cdp_malloc(sizeof(cdpData));
            record->data->capacity   = capacity;
            record->data->size       = size;
            record->data->_far       = data.pointer;
            record->data->destructor = destructor;

            record->metadata.recdata = CDP_RECDATA_FAR;
        } else if (capacity > sizeof(record->_near)) {
            size_t dmax = cdp_max(sizeof((cdpData){}._data), capacity);
            if (data.pointer) {
                assert(size);
                record->data = cdp_malloc(sizeof(cdpData) - sizeof(record->data->_data) + dmax);
                memcpy(record->data->_data, data.pointer, capacity);
            } else {
                record->data = cdp_malloc0(sizeof(cdpData) - sizeof(record->data->_data) + dmax);
            }
            record->data->capacity = dmax;
            record->data->size     = size;
            record->metadata.recdata = CDP_RECDATA_DATA;
        } else {
            record->_near = data;
            record->metadata.recdata = CDP_RECDATA_NEAR;
        }
    }

    return true;
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
    Adds/inserts a *copy* of the specified record into another record.
*/
cdpRecord* cdp_record_add(cdpRecord* parent, cdpRecord* record, bool prepend) {
    assert(cdp_record_is_normal(parent) && !cdp_record_is_void(record));    // 'Void' records are never used.
    if CDP_RARELY(prepend && !cdp_record_is_insertable(parent)) {
        assert(!prepend);
        return NULL;
    }

    cdpChdStore* store;
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
    STORE_TECH_SELECT(parent->metarecord.storage) {
      LINKED_LIST: {
        child = list_add(parent->children, parent, prepend, record);
        break;
      }
      ARRAY: {
        child = array_add(parent->children, parent, prepend, record);
        break;
      }
      PACKED_QUEUE: {
        assert(!cdp_record_is_dictionary(parent));
        child = packed_q_add(parent->children, parent, prepend, record);
        break;
      }
      RED_BLACK_T: {
        child = rb_tree_add(parent->children, parent, record);
        break;
      }
    } SELECTION_END;

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

    cdpChdStore* store;
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
    STORE_TECH_SELECT(parent->metarecord.storage) {
      LINKED_LIST: {
        child = list_sorted_insert(parent->children, record, compare, context);
        break;
      }
      ARRAY: {
        child = array_sorted_insert(parent->children, record, compare, context);
        break;
      }
      PACKED_QUEUE: {
        assert(parent->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        return NULL;
      }
      RED_BLACK_T: {
        child = rb_tree_sorted_insert(parent->children, record, compare, context);
        break;
      }
    } SELECTION_END;

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

    REC_DATA_SELECT(record) {
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

    REC_DATA_SELECT(record) {
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

    REC_DATA_SELECT(record) {
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

    REC_DATA_SELECT(record) {
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

    REC_DATA_SELECT(record) {
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

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        return list_first(record->children);
      }
      ARRAY: {
        return array_first(record->children);
      }
      PACKED_QUEUE: {
        return packed_q_first(record->children);
      }
      RED_BLACK_T: {
        return rb_tree_first(record->children);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the last child record.
*/
cdpRecord* cdp_record_last(const cdpRecord* record) {
    if (!cdp_record_children(record))
        return NULL;

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        return list_last(record->children);
      }
      ARRAY: {
        return array_last(record->children);
      }
      PACKED_QUEUE: {
        return packed_q_last(record->children);
      }
      RED_BLACK_T: {
        return rb_tree_last(record->children);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Retrieves a child record by its id.
*/
cdpRecord* cdp_record_find_by_name(const cdpRecord* record, cdpID name) {
    assert(cdp_id_valid(name));

    if (!cdp_record_children(record))
        return NULL;

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        return list_find_by_name(record->children, name);
      }
      ARRAY: {
        return array_find_by_name(record->children, name, record);
      }
      PACKED_QUEUE: {
        return packed_q_find_by_name(record->children, name);
      }
      RED_BLACK_T: {
        return rb_tree_find_by_name(record->children, name, record);
      }
    } SELECTION_END;

    return NULL;
}



/*
    Finds a child record based on specified key.
*/
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context) {
    assert(!cdp_record_is_void(key) && compare);

    if (!cdp_record_children(record))
        return NULL;

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        return list_find_by_key(record->children, key, compare, context);
      }
      ARRAY: {
        return array_find_by_key(record->children, key, compare, context);
      }
      PACKED_QUEUE: {
        assert(record->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        break;
      }
      RED_BLACK_T: {
        return rb_tree_find_by_key(record->children, key, compare, context);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the record at index position from book.
*/
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t position) {
    if (position >= cdp_record_children(record))
        return NULL;

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        return list_find_by_position(record->children, position);
      }
      ARRAY: {
        return array_find_by_position(record->children, position);
      }
      PACKED_QUEUE: {
        return packed_q_find_by_position(record->children, position);
      }
      RED_BLACK_T: {
        return rb_tree_find_by_position(record->children, position, record);
      }
    } SELECTION_END;

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

    STORE_TECH_SELECT(parent->metarecord.storage) {
      LINKED_LIST: {
        return list_prev(record);
      }
      ARRAY: {
        return array_prev(parent->children, record);
      }
      PACKED_QUEUE: {
        return packed_q_prev(parent->children, record);
      }
      RED_BLACK_T: {
        return rb_tree_prev(record);
      }
    } SELECTION_END;

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


    STORE_TECH_SELECT(parent->metarecord.storage) {
      LINKED_LIST: {
        return list_next(record);
      }
      ARRAY: {
        return array_next(parent->children, record);
      }
      PACKED_QUEUE: {
        return packed_q_next(parent->children, record);
      }
      RED_BLACK_T: {
        return rb_tree_next(record);
      }
    } SELECTION_END;

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

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        return list_next_by_name(record->children, id, (cdpListNode**)childIdx);
      }
      ARRAY: {
        return array_next_by_name(record->children, id, childIdx);
      }
      PACKED_QUEUE: {
        return packed_q_next_by_name(record->children, id, (cdpPackedQNode**)childIdx);
      }
      RED_BLACK_T: {    // Unused.
        break;
      }
    } SELECTION_END;

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

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        return list_traverse(record->children, record, func, context, entry);
      }
      ARRAY: {
        return array_traverse(record->children, record, func, context, entry);
      }
      PACKED_QUEUE: {
        return packed_q_traverse(record->children, record, func, context, entry);
      }
      RED_BLACK_T: {
        return rb_tree_traverse(record->children, record, cdp_bitson(children) + 2, func, context, entry);
      }
    } SELECTION_END;

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
        STORE_TECH_SELECT(entry->parent->metarecord.storage) {
          LINKED_LIST: {
            entry->next = list_next(entry->record);
            break;
          }
          ARRAY: {
            entry->next = array_next(entry->record->store, entry->record);
            break;
          }
          PACKED_QUEUE: {
            entry->next = packed_q_next(entry->record->store, entry->record);
            break;
          }
          RED_BLACK_T: {
            entry->next = rb_tree_next(entry->record);
            break;
          }
        } SELECTION_END;

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

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        list_sort(record->children, record_compare_by_name, NULL);
        break;
      }
      ARRAY: {
        array_sort(record->children, record_compare_by_name, NULL);
        break;
      }
      PACKED_QUEUE: {
        assert(record->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      RED_BLACK_T: {    // Unused.
        break;
      }
    } SELECTION_END;
}


/*
    Sorts unsorted records according to user defined function.
*/
void cdp_record_sort(cdpRecord* record, cdpCompare compare, void* context) {
    assert(!cdp_record_is_void(record) && !cdp_record_is_dictionary(record) && compare);

    if (cdp_record_children(record) <= 1)
        return;

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        list_sort(record->children, compare, context);
        break;
      }
      ARRAY: {
        array_sort(record->children, compare, context);
        break;
      }
      PACKED_QUEUE: {
        assert(record->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      RED_BLACK_T: {
        // ToDo: re-sort RB-tree.
        assert(record->metarecord.storage == CDP_STORAGE_RED_BLACK_T);
        break;
      }
    } SELECTION_END;
}


/*
    De-initiates a record.
*/
void cdp_record_finalize(cdpRecord* record) {
    assert(!cdp_record_is_void(record) && !cdp_record_is_shadowed(record));

    // Delete storage (and children)
    if (cdp_record_with_store(record)) {

        // ToDo: clean shadow.

        STORE_TECH_SELECT(record->metarecord.storage) {
          LINKED_LIST: {
            cdpList* list = record->children;
            list_del_all_children(list);
            list_del(list);
            break;
          }
          ARRAY: {
            cdpArray* array = record->children;
            array_del_all_children(array);
            array_del(array);
            break;
          }
          PACKED_QUEUE: {
            cdpPackedQ* pkdq = record->children;
            packed_q_del_all_children(pkdq);
            packed_q_del(pkdq);
            break;
          }
          RED_BLACK_T: {
            cdpRbTree* tree = record->children;
            rb_tree_del_all_children(tree);
            rb_tree_del(tree);
            break;
          }
        } SELECTION_END;
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

    cdpChdStore* store = CDP_CHD_STORE(record->children);
    if (!store->chdCount)
        return false;

    // Remove this record from its parent (re-organizing siblings).
    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        list_take(record->children, target);
        break;
      }
      ARRAY: {
        array_take(record->children, target);
        break;
      }
      PACKED_QUEUE: {
        packed_q_take(record->children, target);
        break;
      }
      RED_BLACK_T: {
        rb_tree_take(record->children, target);
        break;
      }
    } SELECTION_END;

    store->chdCount--;

    return true;
}


/*
    Removes first child from record.
*/
bool cdp_record_child_pop(cdpRecord* record, cdpRecord* target) {
    assert(!cdp_record_is_void(record) && target);

    cdpChdStore* store = CDP_CHD_STORE(record->children);
    if (!store->chdCount)
        return false;

    // Remove this record from its parent (re-organizing siblings).
    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        list_pop(record->children, target);
        break;
      }
      ARRAY: {
        array_pop(record->children, target);
        break;
      }
      PACKED_QUEUE: {
        packed_q_pop(record->children, target);
        break;
      }
      RED_BLACK_T: {
        rb_tree_pop(record->children, target);
        break;
      }
    } SELECTION_END;

    store->chdCount--;

    return true;
}


/*
    Deletes a record and all its children re-organizing (sibling) storage
*/
void cdp_record_remove(cdpRecord* record, cdpRecord* target) {
    assert(record && !cdp_record_is_shadowed(record) && record != &CDP_ROOT);

    cdpChdStore* store = cdp_record_par_store(record);
    cdpRecord*  parent = store->owner;

    if (target)
        cdp_record_transfer(record, target);  // Save record.
    else
        cdp_record_finalize(record);          // Delete record (along children, if any).

    // Remove this record from its parent (re-organizing siblings).
    STORE_TECH_SELECT(parent->metarecord.storage) {
      LINKED_LIST: {
        list_remove_record(parent->children, record);
        break;
      }
      ARRAY: {
        array_remove_record(parent->children, record);
        break;
      }
      PACKED_QUEUE: {
        packed_q_remove_record(parent->children, record);
        break;
      }
      RED_BLACK_T: {
        rb_tree_remove_record(parent->children, record);
        break;
      }
    } SELECTION_END;

    store->chdCount--;
}


/*
    Deletes all children of a record.
*/
void cdp_record_branch_reset(cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    cdpChdStore* store = CDP_CHD_STORE(record->children);
    if (!store->chdCount)
        return;

    STORE_TECH_SELECT(record->metarecord.storage) {
      LINKED_LIST: {
        list_del_all_children(record->children);
        break;
      }
      ARRAY: {
        array_del_all_children(record->children);
        break;
      }
      PACKED_QUEUE: {
        packed_q_del_all_children(record->children);
        break;
      }
      RED_BLACK_T: {
        rb_tree_del_all_children(record->children);
        break;
      }
    } SELECTION_END;

    store->chdCount = 0;
}




/*
    Encoding of names, domains and tags into values.
*/
cdpID cdp_text_to_acronysm(const char *s, bool tag) {
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

    size_t max_chars = tag? 10: 8;
    if (len > max_chars)
        return 0;       // Limit to max_chars characters.

    cdpID coded = 0;
    for (size_t n = 0; n < len; n++) {
        char c = s[n];

        if (c < 0x20  ||  c > 0x5F)
            return 0;   // Uncodable characters.

        coded |= (cdpID)(c - 0x20) << (6 * ((max_chars - 1) - n));  // Shift and encode each character.
    }

    return tag? cdp_tag_to_acronysm(coded): cdp_id_to_acronysm(coded);
}


size_t cdp_acronysm_to_text(cdpID acro, bool tag, char s[11]) {
    assert(tag? cdp_tag_valid(acro): cdp_id_name_valid(acro));
    cdpID coded = tag? cdp_tag(acro): cdp_id(acro);

    size_t max_chars = tag? 10: 8;
    unsigned length;
    for (length = 0; length < max_chars; length++) {
        char c = (char)((coded >> (6 * ((max_chars - 1) - length))) & 0x3F); // Extract 6 bits for each character (starting from the highest bits).

        s[length] = c + 0x20;   // Restore the original ASCII character.
    }
    s[length] = '\0';

    while (length > 0  &&  s[length - 1] == ' ') {
        s[--length] = '\0';     // Replace trailing spaces with null characters.
    }

    return length;
}




cdpID cdp_text_to_word(const char *s, bool tag) {
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
    size_t max_chars = tag? 12: 10;
    if (len > max_chars)
        return 0;       // Limit to max_chars characters.

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

         coded |= (cdpID)encoded_char << (5 * ((max_chars - 1) - n)); // Shift and encode each character.
    }

    return tag? cdp_tag_to_word(coded): cdp_id_to_word(coded);
}


size_t cdp_word_to_text(cdpID coded, bool tag, char s[13]) {
    assert(tag? cdp_tag_valid(coded): cdp_id_name_valid(coded));

    const char translation_table[5] = {':', '_', '-', '.', '/'};        // Reverse translation table for values 27-31.
    size_t max_chars = tag? 12: 10;
    unsigned length;
    for (length = 0; length < max_chars; length++) {
        uint8_t encoded_char = (coded >> (5 * ((max_chars - 1) - length))) & 0x1F;  // Extract each 5-bit segment, starting from the most significant bits.

        if (encoded_char >= 1  &&  encoded_char <= 26) {
            s[length] = (char)(encoded_char - 1 + 0x61);                // 'a' - 'z'.
        } else if (encoded_char == 0) {
            s[length] = ' ';                                            // Space.
        } else if (encoded_char >= 27  &&  encoded_char <= 31) {
            s[length] = translation_table[encoded_char - 27];           // Map 27-31 using table.
        }
    }
    s[length] = '\0';

    while (length > 0  &&  s[length - 1] == ' ') {
        s[--length] = '\0';     // Replace trailing spaces with null characters.
    }

    return length;
}

