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


#include "cdp_record.h"
#include <stdarg.h>




#define cdpFunc     void*


static inline int record_compare_by_name(const cdpRecord* restrict key, const cdpRecord* restrict rec, void* unused) {
    bool keyNamed = cdp_record_is_named(key);
    bool recNamed = cdp_record_is_named(rec);

    if (keyNamed != recNamed)
        return keyNamed - recNamed;   // Unnamed records should come before named records.

    if (keyNamed)
        return rec->metadata.id - key->metadata.id;   // Sorted by decreasing id if named.

    return key->metadata.id - rec->metadata.id;   // Sorted by increasing id if unnamed.
}


#define STORE_TECH_SELECT(storeTech)                                           \
    assert((storeTech) < CDP_STO_CHD_COUNT);                                   \
    static void* const chdStoreTech[] = {&&LINKED_LIST, &&ARRAY,               \
        &&PACKED_QUEUE, &&RED_BLACK_T};                                        \
    goto *chdStoreTech[storeTech];                                             \
    do


#define RECORD_PRIMAL_SELECT(primal)                                   \
    assert((primal) && (primal) < CDP_TYPE_PRIMAL_COUNT);              \
    static void* const recordStyle[] = {&&BOOK, &&REGISTER, &&LINK};   \
    goto *recordStyle[(primal)-1];                                     \
    do

#define SELECTION_END                                                  \
    while (0)


static inline void book_relink_storage(cdpRecord* book);




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
    cdp_record_initialize(&CDP_ROOT, CDP_TYPE_BOOK, 0, CDP_NAME_ROOT, CDP_TYPE_DICTIONARY, CDP_STO_CHD_ARRAY, 16);
}


/*
    Shutdowns the record system.
*/
void cdp_record_system_shutdown(void) {
    cdp_record_finalize(&CDP_ROOT, 64);   // FixMe: maxDepth.
}




static inline void* book_create_storage(unsigned storage, va_list args) {
    STORE_TECH_SELECT(storage) {
      LINKED_LIST: {
        return list_new();
      }
      ARRAY: {
        unsigned capacity = va_arg(args, unsigned);
        assert(capacity > 0);
        return array_new(capacity);
      }
      PACKED_QUEUE: {
        unsigned capacity = va_arg(args, unsigned);
        assert(capacity > 0);
        return packed_q_new(capacity);
      }
      RED_BLACK_T: {
        return rb_tree_new();
      }
    } SELECTION_END;
    return NULL;
}


static inline void book_relink_storage(cdpRecord* book) {
    cdpChdStore* store = book->recData.book.children;
    assert(store);
    store->book = book;         // Re-link book with its own children storage.
    store = book->recData.book.property;
    if (store)
        store->book = book;     // Re-link with its own property storage.
}


static inline void store_check_auto_id(cdpChdStore* store, cdpRecord* record) {
    if (!cdp_record_id_is_pending(record))
        return;
    assert(store->autoID < CDP_AUTO_ID_MAX);
    record->metadata.id = store->autoID++;
}




/*
    Initiates a record struct with the requested parameters.
*/
bool cdp_record_initialize(cdpRecord* record, unsigned primal, unsigned attrib, cdpID id, uint32_t type, ...) {
    assert(record && primal && type);
    //CDP_0(record);

    record->metadata.attribute = attrib & CDP_ATTRIB_PUB_MASK;
    record->metadata.primal    = primal;
    record->metadata.id        = id;
    record->metadata.type      = type;

    // Create child record storage.
    //
    va_list args;
    va_start(args, type);

    RECORD_PRIMAL_SELECT(primal) {
      BOOK: {
        unsigned reqStore = va_arg(args, unsigned);
        CDP_DEBUG(
            if (type == CDP_TYPE_DICTIONARY)
                assert(reqStore != CDP_STO_CHD_PACKED_QUEUE);
            else
                assert(reqStore != CDP_STO_CHD_RED_BLACK_T);    // Any other type of book storage should be prependable.
        );
        cdpChdStore* chdStore = book_create_storage(reqStore, args);

        // Link book record with its children storage.
        chdStore->book = record;
        record->metadata.storeTech = reqStore;
        record->recData.book.children = chdStore;
        break;
      }

      REGISTER: {
        bool   borrow = va_arg(args, unsigned);
        void*  data   = va_arg(args, void*);
        size_t size   = va_arg(args, size_t);
        assert(size);

        // Allocate storage for register record.
        if (borrow) {
            assert(data);
            record->recData.reg.data.ptr = data;
            record->metadata.storeTech = CDP_STO_REG_BORROWED;
        } else if (data) {
            record->recData.reg.data.ptr = cdp_malloc(size);
            memcpy(record->recData.reg.data.ptr, data, size);
        }
        else
            record->recData.reg.data.ptr = cdp_malloc0(size);
        record->recData.reg.size = size;
        break;
      }

      LINK:
        break;
    } SELECTION_END;

    va_end(args);

    return true;
}


/*
    Adds/inserts a *copy* of the specified record to a book.
*/
cdpRecord* cdp_book_add_record(cdpRecord* book, cdpRecord* record, bool prepend) {
    assert(cdp_record_is_book(book) && !cdp_record_is_void(record));    // 'None' type of records are never inserted in books.
    CDP_DEBUG(if (!cdp_book_is_prependable(book)) assert(!prepend));

    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    store_check_auto_id(store, record);
    cdpRecord* child;

    // Add new record to parent book.
    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        child = list_add(book->recData.book.children, book, prepend, record);
        break;
      }
      ARRAY: {
        child = array_add(book->recData.book.children, book, prepend, record);
        break;
      }
      PACKED_QUEUE: {
        child = packed_q_add(book->recData.book.children, book, prepend, record);
        break;
      }
      RED_BLACK_T: {
        child = rb_tree_add(book->recData.book.children, book, record);
        break;
      }
    } SELECTION_END;

    CDP_0(record);

    // Update child.
    child->store = store;

    if (cdp_record_is_book(child))
        book_relink_storage(child);

    // Update parent.
    store->chdCount++;

    return child;
}


/*
    Adds/inserts a *copy* of the specified record to a book.
*/
cdpRecord* cdp_book_sorted_insert(cdpRecord* book, cdpRecord* record, cdpCompare compare, void* context) {
    assert(cdp_record_is_book(book) && !cdp_record_is_void(record) && compare);    // 'None' type of records are never inserted in books.

    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    store_check_auto_id(store, record);
    cdpRecord* child;

    // Add new record to parent book.
    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        child = list_sorted_insert(book->recData.book.children, record, compare, context);
        break;
      }
      ARRAY: {
        child = array_sorted_insert(book->recData.book.children, record, compare, context);
        break;
      }
      PACKED_QUEUE: {
        assert(book->metadata.storeTech == CDP_STO_CHD_PACKED_QUEUE);   // Unsupported.
        return NULL;
      }
      RED_BLACK_T: {
        child = rb_tree_sorted_insert(book->recData.book.children, record, compare, context);
        break;
      }
    } SELECTION_END;

    CDP_0(record);

    // Update child.
    child->store = store;

    if (cdp_record_is_book(child))
        book_relink_storage(child);

    // Update parent.
    store->chdCount++;

    return child;
}


/*
    Inserts a *copy* of the specified record as a book property.
*/
cdpRecord* cdp_book_add_property(cdpRecord* book, cdpRecord* record) {
    assert(cdp_record_is_book(book) && !cdp_record_is_void(record));    // 'None' type of records are never inserted in books.

    cdpRbTree* propTree = book->recData.book.property;
    if (!propTree) {
        propTree = rb_tree_new();
        propTree->store.book = book;
    }
    store_check_auto_id(&propTree->store, record);

    cdpRecord* child = rb_tree_add_property(propTree, record);

    CDP_0(record);

    // Update child.
    child->store = &propTree->store;

    if (cdp_record_is_book(child))
        book_relink_storage(child);

    // Update parent.
    propTree->store.chdCount++;

    return child;
}


/*
    Retrieves a book property by its id.
*/
cdpRecord* cdp_book_get_property(const cdpRecord* book, cdpID id) {
    assert(cdp_record_is_book(book));
    cdpRbTree* propTree = book->recData.book.property;
    CDP_CK(propTree && propTree->store.chdCount);
    return rb_tree_find_by_id(propTree, id);
}




/*
   Reads register data from position and puts it on data buffer (atomically).
*/
void* cdp_register_read(const cdpRecord* reg, size_t position, void* data, size_t* size) {
    assert(cdp_record_is_register(reg));

    // Calculate the actual number of bytes that can be read.
    assert(reg->recData.reg.size > position);
    size_t readableSize = reg->recData.reg.size - position;
    if (size && (!*size || *size > readableSize))
        *size = readableSize;

    // Copy the data from the register to the provided buffer (if any).
    void* pointed = cdp_ptr_off(reg->recData.reg.data.ptr, position);
    if (data) {
        memcpy(data, pointed, readableSize);
        return data;
    }

    //assert(cdp_record_is_private(reg));       // FixMe: return atomic register if possible.
    return pointed;
}


/*
   Writes the data of a register record at position (atomically and it may reallocate memory).
*/
void* cdp_register_write(cdpRecord* reg, size_t position, const void* data, size_t size) {
    assert(cdp_record_is_register(reg) && !cdp_record_is_factual(reg) && data && size);

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
#define CDP_RECORD_PATH_INITIAL_LENGTH  16

bool cdp_record_path(const cdpRecord* record, cdpPath** path) {
    assert(record && path);

    cdpPath* tempPath;
    if (*path) {
        tempPath = *path;
        assert(tempPath->capacity);
    } else {
        tempPath = cdp_dyn_malloc(cdpPath, cdpID, CDP_RECORD_PATH_INITIAL_LENGTH);    // FixMe: pre-compute this to be power of 2.
        tempPath->capacity = CDP_RECORD_PATH_INITIAL_LENGTH;
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
    Gets the first record from a book.
*/
cdpRecord* cdp_book_first(const cdpRecord* book) {
    assert(cdp_record_is_book(book));
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_CK(store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_first(book->recData.book.children);
      }
      ARRAY: {
        return array_first(book->recData.book.children);
      }
      PACKED_QUEUE: {
        return packed_q_first(book->recData.book.children);
      }
      RED_BLACK_T: {
        return rb_tree_first(book->recData.book.children);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the last record from a book.
*/
cdpRecord* cdp_book_last(const cdpRecord* book) {
    assert(cdp_record_is_book(book));
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_CK(store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_last(book->recData.book.children);
      }
      ARRAY: {
        return array_last(book->recData.book.children);
      }
      PACKED_QUEUE: {
        return packed_q_last(book->recData.book.children);
      }
      RED_BLACK_T: {
        return rb_tree_last(book->recData.book.children);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Retrieves a child record by its id.
*/
cdpRecord* cdp_book_find_by_name(const cdpRecord* book, cdpID id) {
    assert(cdp_record_is_book(book));
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_CK(store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_find_by_name(book->recData.book.children, id);
      }
      ARRAY: {
        return array_find_by_name(book->recData.book.children, id, book);
      }
      PACKED_QUEUE: {
        return packed_q_find_by_name(book->recData.book.children, id);
      }
      RED_BLACK_T: {
        return rb_tree_find_by_name(book->recData.book.children, id, book);
      }
    } SELECTION_END;

    return NULL;
}



/*
    Finds a child record based on specified key.
*/
cdpRecord* cdp_book_find_by_key(const cdpRecord* book, cdpRecord* key, cdpCompare compare, void* context) {
    assert(cdp_record_is_book(book) && !cdp_record_is_void(key) && compare);
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_CK(store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_find_by_key(book->recData.book.children, key, compare, context);
      }
      ARRAY: {
        return array_find_by_key(book->recData.book.children, key, compare, context);
      }
      PACKED_QUEUE: {
        assert(book->metadata.storeTech == CDP_STO_CHD_PACKED_QUEUE);   // Unsupported.
        break;
      }
      RED_BLACK_T: {
        return rb_tree_find_by_key(book->recData.book.children, key, compare, context);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the record at index position from book.
*/
cdpRecord* cdp_book_find_by_position(const cdpRecord* book, size_t position) {
    assert(cdp_record_is_book(book));
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_CK(store->chdCount);
    assert(position < store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_find_by_position(book->recData.book.children, position);
      }
      ARRAY: {
        return array_find_by_position(book->recData.book.children, position);
      }
      PACKED_QUEUE: {
        return packed_q_find_by_position(book->recData.book.children, position);
      }
      RED_BLACK_T: {
        return rb_tree_find_by_position(book->recData.book.children, position, book);
      }
    } SELECTION_END;

    return NULL;
}


/*
    Gets the record by its path from start record.
*/
cdpRecord* cdp_book_find_by_path(const cdpRecord* start, const cdpPath* path) {
    assert(cdp_record_is_book(start) && path && path->length);
    CDP_CK(cdp_book_children(start));
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
        assert(cdp_record_is_book(book));
        store = CDP_CHD_STORE(book->recData.book.children);
    } else {
        store = cdp_record_par_store(record);
        book = store->book;
    }
    CDP_CK(store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_prev(record);
      }
      ARRAY: {
        return array_prev(book->recData.book.children, record);
      }
      PACKED_QUEUE: {
        return packed_q_prev(book->recData.book.children, record);
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
        assert(cdp_record_is_book(book));
        store = CDP_CHD_STORE(book->recData.book.children);
    } else {
        store = cdp_record_par_store(record);
        book = store->book;
    }
    CDP_CK(store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_next(record);
      }
      ARRAY: {
        return array_next(book->recData.book.children, record);
      }
      PACKED_QUEUE: {
        return packed_q_next(book->recData.book.children, record);
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
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_CK(store->chdCount);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_next_by_name(book->recData.book.children, id, (cdpListNode**)childIdx);
      }
      ARRAY: {
        return array_next_by_name(book->recData.book.children, id, childIdx);
      }
      PACKED_QUEUE: {
        return packed_q_next_by_name(book->recData.book.children, id, (cdpPackedQNode**)childIdx);
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
    assert(cdp_record_is_book(start) && path && path->length);
    CDP_CK(cdp_book_children(start));
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
bool cdp_book_traverse(cdpRecord* book, cdpRecordTraverse func, void* context, cdpBookEntry* entry) {
    assert(cdp_record_is_book(book) && func);
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_GO(!store->chdCount);
    if (!entry) entry = cdp_alloca(sizeof(cdpBookEntry));
    CDP_0(entry);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        return list_traverse(book->recData.book.children, book, func, context, entry);
      }
      ARRAY: {
        return array_traverse(book->recData.book.children, book, func, context, entry);
      }
      PACKED_QUEUE: {
        return packed_q_traverse(book->recData.book.children, book, func, context, entry);
      }
      RED_BLACK_T: {
        return rb_tree_traverse(book->recData.book.children, book, cdp_bitson(store->chdCount) + 2, func, context, entry);
      }
    } SELECTION_END;

    return true;
}


/*
    Traverses each child branch and sub-branch of a book record, applying a function to each.
*/
#define CDP_MAX_FAST_STACK_DEPTH  16

bool cdp_book_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, cdpRecordTraverse endFunc, void* context, cdpBookEntry* entry) {
    assert(cdp_record_is_book(book) && maxDepth && (func || endFunc));
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_GO(!store->chdCount);

    bool ok = true;
    cdpRecord* child;
    unsigned depth = 0;
    if (!entry) entry = cdp_alloca(sizeof(cdpBookEntry));
    CDP_0(entry);
    entry->parent = book;
    entry->record = cdp_book_first(book);

    // Non-recursive version of branch descent:
    cdpBookEntry* stack = (maxDepth > CDP_MAX_FAST_STACK_DEPTH)?  cdp_malloc(maxDepth * sizeof(cdpBookEntry)):  cdp_alloca(maxDepth * sizeof(cdpBookEntry));
    for (;;) {
        // Ascend to parent if no more siblings in branch.
        if (!entry->record) {
            if CDP_RARELY(!depth)  break;    // endFunc is never called on root book.
            depth--;

            if (endFunc) {
                ok = endFunc(&stack[depth], depth, context);
                if (!ok)  break;
            }

            // Next record.
            entry->record   = stack[depth].next;
            entry->parent   = stack[depth].parent;
            entry->prev     = stack[depth].record;
            entry->next     = NULL;
            entry->position = stack[depth].position + 1;
            continue;
        }

      NEXT_SIBLING:
        // Get sibling...
        STORE_TECH_SELECT(entry->parent->metadata.storeTech) {
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
            ok = func(entry, depth, context);
            if (!ok)  break;
        }

        // Descent to children if it's a book.
        if (cdp_record_is_book(entry->record)
        && ((child = cdp_book_first(entry->record)))) {
            assert(depth < maxDepth);

            stack[depth++]  = *entry;

            entry->parent   = entry->record;
            entry->record   = child;
            entry->prev     = NULL;
            entry->position = 0;

            goto NEXT_SIBLING;
        }

        // Next record.
        entry->prev   = entry->record;
        entry->record = entry->next;
        entry->position += 1;
    }

    if (maxDepth > CDP_MAX_FAST_STACK_DEPTH)
        cdp_free(stack);

    return ok;
}




/*
    Converts an unsorted book into a dictionary.
*/
void cdp_book_to_dictionary(cdpRecord* book) {
    CDP_AB(cdp_record_is_dictionary(book));
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);

    book->metadata.type = CDP_TYPE_DICTIONARY;
    CDP_AB(store->chdCount <= 1);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        list_sort(book->recData.book.children, record_compare_by_name, NULL);
        break;
      }
      ARRAY: {
        array_sort(book->recData.book.children, record_compare_by_name, NULL);
        break;
      }
      PACKED_QUEUE: {
        assert(book->metadata.storeTech == CDP_STO_CHD_PACKED_QUEUE);    // Unsupported.
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
    assert(cdp_record_is_book(book) && compare);
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    CDP_AB(store->chdCount <= 1);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        list_sort(book->recData.book.children, compare, context);
        break;
      }
      ARRAY: {
        array_sort(book->recData.book.children, compare, context);
        break;
      }
      PACKED_QUEUE: {
        assert(book->metadata.storeTech == CDP_STO_CHD_PACKED_QUEUE);    // Unsupported.
        break;
      }
      RED_BLACK_T: {
        // ToDo: re-sort RB-tree.
        assert(book->metadata.storeTech == CDP_STO_CHD_RED_BLACK_T);
        break;
      }
    } SELECTION_END;
}


/*
    De-initiates a record.
*/
void cdp_record_finalize(cdpRecord* record, unsigned maxDepth) {
    assert(!cdp_record_is_void(record) && maxDepth);
    assert(!cdp_record_is_shadowed(record));

    // Delete storage (and children).
    RECORD_PRIMAL_SELECT(record->metadata.primal) {
      BOOK: {
        STORE_TECH_SELECT(record->metadata.storeTech) {
          LINKED_LIST: {
            cdpList* list = record->recData.book.children;
            list_del_all_children(list, maxDepth);
            list_del(list);
            break;
          }
          ARRAY: {
            cdpArray* array = record->recData.book.children;
            array_del_all_children(array, maxDepth);
            array_del(array);
            break;
          }
          PACKED_QUEUE: {
            cdpPackedQ* pkdq = record->recData.book.children;
            packed_q_del_all_children(pkdq, maxDepth);
            packed_q_del(pkdq);
            break;
          }
          RED_BLACK_T: {
            cdpRbTree* tree = record->recData.book.children;
            rb_tree_del_all_children(tree, maxDepth);
            rb_tree_del(tree);
            break;
          }
        } SELECTION_END;

        cdpRbTree* propTree = record->recData.book.property;
        if (propTree) {
            rb_tree_del_all_children(propTree, maxDepth);
            rb_tree_del(propTree);
        }
        break;
      }

      REGISTER: {
        if (!cdp_register_is_borrowed(record))
            cdp_free(record->recData.reg.data.ptr);
        break;
      }

      LINK:
        break;
    } SELECTION_END;
}


/*
    Deletes a record and all its children re-organizing (sibling) storage
*/
bool cdp_record_remove(cdpRecord* record, unsigned maxDepth) {
    assert(record && maxDepth);
    assert(!cdp_record_is_shadowed(record));
    cdpChdStore* store = cdp_record_par_store(record);
    cdpRecord* book = store->book;

    // Delete storage (along children, if any).
    cdp_record_finalize(record, maxDepth);

    // Remove this record from its parent (re-organizing siblings).
    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        list_remove_record(book->recData.book.children, record);
        break;
      }
      ARRAY: {
        array_remove_record(book->recData.book.children, record);
        break;
      }
      PACKED_QUEUE: {
        packed_q_remove_record(book->recData.book.children, record);
        break;
      }
      RED_BLACK_T: {
        rb_tree_remove_record(book->recData.book.children, record);
        break;
      }
    } SELECTION_END;

    store->chdCount--;

    return true;
}


/*
    Deletes all children of a book.
*/
size_t cdp_book_reset(cdpRecord* book, unsigned maxDepth) {
    assert(cdp_record_is_book(book) && maxDepth);
    cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children);
    size_t children = store->chdCount;
    CDP_CK(children);

    STORE_TECH_SELECT(book->metadata.storeTech) {
      LINKED_LIST: {
        list_del_all_children(book->recData.book.children, maxDepth);
        break;
      }
      ARRAY: {
        array_del_all_children(book->recData.book.children, maxDepth);
        break;
      }
      PACKED_QUEUE: {
        packed_q_del_all_children(book->recData.book.children, maxDepth);
        break;
      }
      RED_BLACK_T: {
        rb_tree_del_all_children(book->recData.book.children, maxDepth);
        break;
      }
    } SELECTION_END;

    store->chdCount = 0;
    return children;
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

