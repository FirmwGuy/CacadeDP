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




cdpID AUTOID = cdp_id_from_naming(CDP_NAMING_GLOBAL);   // Global autoid.

#define CDP_MAX_FAST_STACK_DEPTH  16

unsigned MAX_DEPTH = CDP_MAX_FAST_STACK_DEPTH;     // FixMe: (used by path/traverse) better policy than a global for this.

#define cdpFunc     void*




static inline int record_compare_by_name(const cdpRecord* restrict key, const cdpRecord* restrict rec, void* unused) {
    return cdp_record_get_id(key) - cdp_record_get_id(rec);
}


#define STORE_TECH_SELECT(structure)                                           \
    assert((structure) < CDP_STORAGE_COUNT);                                   \
    static void* const chdStoreTech[] = {&&LINKED_LIST, &&ARRAY,               \
        &&PACKED_QUEUE, &&RED_BLACK_T};                                        \
    goto *chdStoreTech[structure];                                             \
    do

#define REC_DATA_SELECT(recdata)                                       \
    static void* const _recData[] = {&&NONE, &&NEAR, &&DATA, &&FAR};   \
    goto *_recData[recdata];                                           \
    do

#define SELECTION_END                                                  \
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


cdpRecord CDP_ROOT;


/*
    Initiates the record system.
*/
void cdp_record_system_initiate(void) {
    // The root dictionary is the same as "/" in text paths.
    cdp_record_initialize_dictionary(&CDP_ROOT, CDP_NAME_ROOT, CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
}


/*
    Shutdowns the record system.
*/
void cdp_record_system_shutdown(void) {
    cdp_record_finalize(&CDP_ROOT);
}




static inline void* book_create_storage(unsigned storage, unsigned capacity) {
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


static inline void store_check_auto_id(cdpChdStore* store, cdpRecord* record) {
    if (CDP_AUTO_ID_LOCAL == record->metarecord.name)
        cdp_record_set_id(record, cdp_id_local(store->autoID++));
    else if (CDP_AUTO_ID_GLOBAL == record->metarecord.name)
        cdp_record_set_id(record, cdp_id_global(AUTOID++));
}




/*
    Initiates a record struct with the requested parameters.
*/
bool cdp_record_initialize( cdpRecord* record, cdpID name,
                            bool dictionary, unsigned storage, size_t basez,
                            cdpID metadata, size_t size, size_t capacity,
                            cdpValue data, cdpDel destructor) {
    assert(record && cdp_id_valid(name) && storage < CDP_STORAGE_COUNT);
    if (dictionary && storage == CDP_STORAGE_PACKED_QUEUE) {
        assert(storage != CDP_STORAGE_PACKED_QUEUE);
        return false;
    }
    if (storage == CDP_STORAGE_ARRAY
    ||  storage == CDP_STORAGE_PACKED_QUEUE) {
        if (!basez) {
            assert(basez);
            return false;
        }
    }
    //CDP_0(record);

    record->metarecord.name       = name;
    record->metarecord.dictionary = dictionary? 1: 0;
    record->metarecord.storage    = storage;
    record->metarecord.basez      = basez;

    if (capacity) {
        if (destructor) {
            assert(data.pointer && size);

            record->data = cdp_malloc(sizeof(cdpData));
            record->data->size       = size;
            record->data->capacity   = capacity;
            record->data->_far       = data.pointer;
            record->data->destructor = destructor;

            metarecord->recdata = CDP_RECDATA_FAR;
        } else if (capacity > sizeof(record->_near)) {
            dmax = cdp_max(sizeof((cdpData)->_data), capacity);
            if (data.pointer) {
                assert(size);

                record->data = cdp_malloc(sizeof(cdpData) - sizeof((cdpData)->_data) + dmax);
                record->data->capacity   = dmax;
                record->data->destructor = NULL;
                record->data->size       = size;

                memcpy(record->data->_data, data.pointer, capacity);
            } else {
                record->data = cdp_malloc0(sizeof(cdpData) - sizeof((cdpData)->_data) + dmax);
                record->data->capacity   = dmax;
                record->data->destructor = NULL;
                record->data->size       = size;
            }
            metarecord->recdata = CDP_RECDATA_DATA;
        } else {
            record->_near = data;
            metarecord->recdata = CDP_RECDATA_NEAR;
        }
    }

    return true;
}




/* Creates a deep copy of record and all its data.
*/
void cdp_record_initialize_clone(cdpRecord* newClone, cdpID nameID, cdpRecord* record) {
    assert(newClone && !cdp_record_is_void(record));

    newClone->metadata = record->metadata;
    newClone->recData  = record->recData;
    newClone->store    = NULL;

    RECORD_TYPE_SELECT(cdp_record_type(newClone)) {
      BOOK: {
        // Clone children: Pending!
        assert(!cdp_record_children(record));
      }

      REGISTER: {
        // Clone data: Pending!
        assert(!cdp_record_has_data(record));
      }

      LINK: {
        // Clone shadowing: Pending!
        assert(!cdp_record_is_link(record));
      }
    } SELECTION_END;
}




/*
    Adds/inserts a *copy* of the specified record into another record.
*/
cdpRecord* cdp_record_add(cdpRecord* parent, cdpRecord* record, bool prepend) {
    assert(!cdp_record_is_void(parent) && !cdp_record_is_void(record));    // 'Void' records are never used.
    if (preped && !cdp_record_is_insertable(parent)) {
        assert(!prepend);
        return NULL;
    }

    cdpChdStore* store;
    if (parent->metarecord.withstore) {
        store = parent->children;
    } else {
        store = book_create_storage(parent->metarecord.storage, parent->basez);

        // Link parent record with its children storage.
        store->owner = parent;
        parent->children = store;
        parent->metarecord.withstore = 1;
    }
    store_check_auto_id(store, parent);
    cdpRecord* child;

    // Add new record to parent parent.
    STORE_TECH_SELECT(parent->metarecord.storage) {
      LINKED_LIST: {
        child = list_add(store, parent, prepend, record);
        break;
      }
      ARRAY: {
        child = array_add(store, parent, prepend, record);
        break;
      }
      PACKED_QUEUE: {
        assert(!cdp_record_is_dictionary(parent));
        child = packed_q_add(store, parent, prepend, record);
        break;
      }
      RED_BLACK_T: {
        child = rb_tree_add(store, parent, record);
        break;
      }
    } SELECTION_END;

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
    assert(!cdp_record_is_void(parent) && !cdp_record_is_void(record) && compare);    // 'None' role of records are never used.

    if (parent->metarecord.withstore) {
        store = parent->children;
    } else {
        store = book_create_storage(parent->metarecord.storage, parent->basez);

        // Link parent record with its children storage.
        store->owner = parent;
        parent->children = store;
        parent->metarecord.withstore = 1;
    }
    store_check_auto_id(store, parent);
    cdpRecord* child;

    // Add new record to parent.
    STORE_TECH_SELECT(parent->metarecord.storage) {
      LINKED_LIST: {
        child = list_sorted_insert(store, record, compare, context);
        break;
      }
      ARRAY: {
        child = array_sorted_insert(store, record, compare, context);
        break;
      }
      PACKED_QUEUE: {
        assert(parent->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        return NULL;
      }
      RED_BLACK_T: {
        child = rb_tree_sorted_insert(store, record, compare, context);
        break;
      }
    } SELECTION_END;

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
cdpValue cdp_record_read(const cdpRecord* record, void* data, size_t* size, size_t* capacity) {
    assert(!cdp_record_is_void(record));

    REC_DATA_SELECT(record->metadata.recdata) {
      NONE: {
        assert(cdp_record_has_data(record));
        return (cdpValue){.pointer = NULL};
      }

      NEAR: {
        return record->_near;
      }

      DATA: {
        return (cdpValue){.pointer = record->data->_data};
      }

      FAR: {
        return (cdpValue){.pointer = record->data->_far};
      }

    // Copy the data from the register to the provided buffer (if any).
    void* pointed = cdp_ptr_off(reg->recData.reg.data.ptr, position);
    if (data) {
        memcpy(data, pointed, readableSize);
        return data;
    }
}


/*
   Writes the data of a register record at position (atomically and it may reallocate memory).
*/
void* cdp_register_write(cdpRecord* reg, size_t position, const void* data, size_t size) {
    assert(cdp_record_has_data(reg) && !cdp_record_is_factual(reg) && data && size);

    // Ensure the buffer is large enough to accommodate the write
    size_t newSize = position + size;
    if (newSize > reg->recData.reg.size) {
        // Resize the buffer
        CDP_REALLOC(reg->recData.reg.data.ptr, newSize);
        reg->recData.reg.size = newSize;
    }

    // Copy the provided data into the register's buffer at the specified position
    memcpy(cdp_ptr_off(reg->recData.reg.data.ptr, position), data, size);

    return reg->recData.reg.data.ptr;
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
        tempPath->id[tempPath->capacity - tempPath->length - 1] = current->metadata.id;
        tempPath->length++;
    }

    return true;
}


/*
    Gets the first child record.
*/
cdpRecord* cdp_book_first(const cdpRecord* book) {
    assert(cdp_record_children(book));
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount) return NULL;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_first(book->children);
      }
      ARRAY: {
        return array_first(book->children);
      }
      PACKED_QUEUE: {
        return packed_q_first(book->children);
      }
      RED_BLACK_T: {
        return rb_tree_first(book->children);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the last record from a book.
*/
cdpRecord* cdp_book_last(const cdpRecord* book) {
    assert(cdp_record_children(book));
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return NULL;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_last(book->children);
      }
      ARRAY: {
        return array_last(book->children);
      }
      PACKED_QUEUE: {
        return packed_q_last(book->children);
      }
      RED_BLACK_T: {
        return rb_tree_last(book->children);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Retrieves a child record by its id.
*/
cdpRecord* cdp_book_find_by_name(const cdpRecord* book, cdpID id) {
    assert(cdp_record_children(book));
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return NULL;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_find_by_name(book->children, id);
      }
      ARRAY: {
        return array_find_by_name(book->children, id, book);
      }
      PACKED_QUEUE: {
        return packed_q_find_by_name(book->children, id);
      }
      RED_BLACK_T: {
        return rb_tree_find_by_name(book->children, id, book);
      }
    } SELECTION_END;

    return NULL;
}



/*
    Finds a child record based on specified key.
*/
cdpRecord* cdp_book_find_by_key(const cdpRecord* book, cdpRecord* key, cdpCompare compare, void* context) {
    assert(cdp_record_children(book) && !cdp_record_is_void(key) && compare);
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return NULL;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_find_by_key(book->children, key, compare, context);
      }
      ARRAY: {
        return array_find_by_key(book->children, key, compare, context);
      }
      PACKED_QUEUE: {
        assert(book->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);   // Unsupported.
        break;
      }
      RED_BLACK_T: {
        return rb_tree_find_by_key(book->children, key, compare, context);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the record at index position from book.
*/
cdpRecord* cdp_book_find_by_position(const cdpRecord* book, size_t position) {
    assert(cdp_record_children(book));
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return NULL;
    assert(position < store->chdCount);

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_find_by_position(book->children, position);
      }
      ARRAY: {
        return array_find_by_position(book->children, position);
      }
      PACKED_QUEUE: {
        return packed_q_find_by_position(book->children, position);
      }
      RED_BLACK_T: {
        return rb_tree_find_by_position(book->children, position, book);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the record by its path from start record.
*/
cdpRecord* cdp_book_find_by_path(const cdpRecord* start, const cdpPath* path) {
    assert(cdp_record_children(start) && path && path->length);
    if (!cdp_record_children(start))  return NULL;
    const cdpRecord* record = start;

    for (unsigned depth = 0;  depth < path->length;  depth++) {
        record = cdp_book_find_by_name(record, path->id[depth]);
        if (!record) return NULL;
    }

    return (cdpRecord*)record;
}





/*
    Retrieves the previous sibling of record.
*/
cdpRecord* cdp_book_prev(const cdpRecord* book, cdpRecord* record) {
    assert(record);
    cdpChdStore* store;
    if (book) {
        assert(cdp_record_children(book));
        store = CDP_CHD_STORE(book->children);
    } else {
        store = cdp_record_par_store(record);
        book = store->owner;
    }
    if (!store->chdCount)   return NULL;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_prev(record);
      }
      ARRAY: {
        return array_prev(book->children, record);
      }
      PACKED_QUEUE: {
        return packed_q_prev(book->children, record);
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
cdpRecord* cdp_book_next(const cdpRecord* book, cdpRecord* record) {
    assert(record);
    cdpChdStore* store;
    if (book) {
        assert(cdp_record_children(book));
        store = CDP_CHD_STORE(book->children);
    } else {
        store = cdp_record_par_store(record);
        book = store->owner;
    }
    if (!store->chdCount)   return NULL;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_next(record);
      }
      ARRAY: {
        return array_next(book->children, record);
      }
      PACKED_QUEUE: {
        return packed_q_next(book->children, record);
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
cdpRecord* cdp_book_find_next_by_name(const cdpRecord* book, cdpID id, uintptr_t* childIdx) {
    if (cdp_record_is_dictionary(book) || !childIdx) {
        CDP_PTR_SEC_SET(childIdx, 0);
        return cdp_book_find_by_name(book, id);
    }
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return NULL;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_next_by_name(book->children, id, (cdpListNode**)childIdx);
      }
      ARRAY: {
        return array_next_by_name(book->children, id, childIdx);
      }
      PACKED_QUEUE: {
        return packed_q_next_by_name(book->children, id, (cdpPackedQNode**)childIdx);
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
cdpRecord* cdp_book_find_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev) {
    assert(cdp_record_children(start) && path && path->length);
    if (!cdp_record_children(start))  return NULL;
    const cdpRecord* record = start;

    for (unsigned depth = 0;  depth < path->length;  depth++) {
        // FixMe: depth must be stored in a stack as well!
        // ...(pending)
        record = cdp_book_find_next_by_name(record, path->id[depth], prev);
        if (!record) return NULL;
    }

    return (cdpRecord*)record;
}


/*
    Traverses the children of a book record, applying a function to each.
*/
bool cdp_book_traverse(cdpRecord* book, cdpTraverse func, void* context, cdpBookEntry* entry) {
    assert(cdp_record_children(book) && func);
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount) return true;
    if (!entry) entry = cdp_alloca(sizeof(cdpBookEntry));
    CDP_0(entry);

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        return list_traverse(book->children, book, func, context, entry);
      }
      ARRAY: {
        return array_traverse(book->children, book, func, context, entry);
      }
      PACKED_QUEUE: {
        return packed_q_traverse(book->children, book, func, context, entry);
      }
      RED_BLACK_T: {
        return rb_tree_traverse(book->children, book, cdp_bitson(store->chdCount) + 2, func, context, entry);
      }
    } SELECTION_END;

    return true;
}


/*
    Traverses each child branch and sub-branch of a book record, applying a function to each.
*/
bool cdp_book_deep_traverse(cdpRecord* book, cdpTraverse func, cdpTraverse endFunc, void* context, cdpBookEntry* entry) {
    assert(cdp_record_children(book) && (func || endFunc));
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return true;

    bool ok = true;
    cdpRecord* child;
    unsigned depth = 0;
    if (!entry) entry = cdp_alloca(sizeof(cdpBookEntry));
    CDP_0(entry);
    entry->parent = book;
    entry->record = cdp_book_first(book);

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

      NEXT_SIBLING:
        // Get sibling...
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
        && ((child = cdp_book_first(entry->record)))) {
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
    Converts an unsorted book into a dictionary.
*/
void cdp_book_to_dictionary(cdpRecord* book) {
    if (!cdp_record_is_dictionary(book))    return;
    cdpChdStore* store = CDP_CHD_STORE(book->children);

    CDP_RECORD_SET_ATTRIB(book, CDP_ATTRIB_DICTIONARY);
    if (store->chdCount <= 1)   return;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        list_sort(book->children, record_compare_by_name, NULL);
        break;
      }
      ARRAY: {
        array_sort(book->children, record_compare_by_name, NULL);
        break;
      }
      PACKED_QUEUE: {
        assert(book->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      RED_BLACK_T: {    // Unused.
        break;
      }
    } SELECTION_END;
}


/*
    Sorts unsorted books according to user defined function.
*/
void cdp_book_sort(cdpRecord* book, cdpCompare compare, void* context) {
    assert(cdp_record_children(book) && compare);
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (store->chdCount <= 1)   return;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        list_sort(book->children, compare, context);
        break;
      }
      ARRAY: {
        array_sort(book->children, compare, context);
        break;
      }
      PACKED_QUEUE: {
        assert(book->metarecord.storage == CDP_STORAGE_PACKED_QUEUE);    // Unsupported.
        break;
      }
      RED_BLACK_T: {
        // ToDo: re-sort RB-tree.
        assert(book->metarecord.storage == CDP_STORAGE_RED_BLACK_T);
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
    if (record->metarecord.withstore) {
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
    if (record->metarecord.recdata != CDP_RECDATA_NEAR) {
        if (record->metarecord.recdata == CDP_RECDATA_FAR) {
            record->data->destructor(record->data->_far);
        cdp_free(record->data);
    }

    // ToDo: unlink from 'self' list.
}


/*
    Removes the last record from a book.
*/
bool cdp_book_take(cdpRecord* book, cdpRecord* target) {
    assert(cdp_record_children(book) && target);
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return false;

    // Remove this record from its parent (re-organizing siblings).
    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        list_take(book->children, target);
        break;
      }
      ARRAY: {
        array_take(book->children, target);
        break;
      }
      PACKED_QUEUE: {
        packed_q_take(book->children, target);
        break;
      }
      RED_BLACK_T: {
        rb_tree_take(book->children, target);
        break;
      }
    } SELECTION_END;

    store->chdCount--;

    return true;
}


/*
    Removes the first record from a book.
*/
bool cdp_book_pop(cdpRecord* book, cdpRecord* target) {
    assert(cdp_record_children(book) && target);
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    if (!store->chdCount)   return false;

    // Remove this record from its parent (re-organizing siblings).
    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        list_pop(book->children, target);
        break;
      }
      ARRAY: {
        array_pop(book->children, target);
        break;
      }
      PACKED_QUEUE: {
        packed_q_pop(book->children, target);
        break;
      }
      RED_BLACK_T: {
        rb_tree_pop(book->children, target);
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
    assert(record && !cdp_record_is_shadowed(record));
    cdpChdStore* store = cdp_record_par_store(record);
    cdpRecord* book = store->owner;

    if (target) {
        // Save record.
        cdp_record_transfer(record, target);
    } else {
        // Delete record (along children, if any).
        cdp_record_finalize(record);
    }

    // Remove this record from its parent (re-organizing siblings).
    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        list_remove_record(book->children, record);
        break;
      }
      ARRAY: {
        array_remove_record(book->children, record);
        break;
      }
      PACKED_QUEUE: {
        packed_q_remove_record(book->children, record);
        break;
      }
      RED_BLACK_T: {
        rb_tree_remove_record(book->children, record);
        break;
      }
    } SELECTION_END;

    store->chdCount--;
}


/*
    Deletes all children of a record.
*/
void cdp_book_reset(cdpRecord* book) {
    assert(cdp_record_children(book));
    cdpChdStore* store = CDP_CHD_STORE(book->children);
    size_t children = store->chdCount;
    if (!children)  return;

    STORE_TECH_SELECT(book->metarecord.storage) {
      LINKED_LIST: {
        list_del_all_children(book->children);
        break;
      }
      ARRAY: {
        array_del_all_children(book->children);
        break;
      }
      PACKED_QUEUE: {
        packed_q_del_all_children(book->children);
        break;
      }
      RED_BLACK_T: {
        rb_tree_del_all_children(book->children);
        break;
      }
    } SELECTION_END;

    store->chdCount = 0;

    // ToDo: delete all children-related properties.
}




#if 0
#include <pthread.h> // Required for pthread_mutex_t and related functions

// To manage concurrent access to records safely, each record could include a mutex as part of its structure.
// Here is a conceptual addition to the cdpRecord structure:
typedef struct cdpRecordStruct {
    // Existing fields...
    pthread_mutex_t mutex; // Mutex for synchronizing access to this record
} cdpRecord;

// Initializes a record's mutex
bool cdp_record_init_mutex(cdpRecord* record) {
    if (!record) {
        return false;
    }
    pthread_mutex_init(&record->mutex, NULL);
    return true;
}

// Locks a record for exclusive access
bool cdp_record_lock(cdpRecord* record) {
    if (!record) {
        return false;
    }
    pthread_mutex_lock(&record->mutex);
    return true;
}

// Unlocks a record
bool cdp_record_unlock(cdpRecord* record) {
    if (!record) {
        return false;
    }
    pthread_mutex_unlock(&record->mutex);
    return true;
}
#endif

