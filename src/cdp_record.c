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

static inline int record_compare_by_name(const cdpRecord* restrict key, const cdpRecord* restrict record) {
    return key->metadata.nameID - record->metadata.nameID;
}

static inline int record_compare_by_name_s(const cdpRecord* restrict key, const cdpRecord* restrict record, void* unused) {
    return record_compare_by_name(key, record);
}


#define STORAGE_TECH_SELECT(stoTech)                                   \
    assert((stoTech) < CDP_STO_CHD_COUNT);                             \
    static void* const chdStoTech[] = {&&LINKED_LIST, &&ARRAY,         \
        &&PACKED_QUEUE, &&RED_BLACK_T};                                \
    goto *chdStoTech[stoTech];                                         \
    do


#define RECORD_STYLE_SELECT(reStyle)                                   \
    assert((reStyle) < CDP_REC_STYLE_COUNT);                           \
    static void* const recordStyle[] = {&&BOOK, &&DICTIONARY,          \
        &&REGISTER, &&LINK};                                           \
    goto *recordStyle[reStyle];                                        \
    do

#define SELECTION_END                                                  \
    while (0)


static inline void record_delete_storage(cdpRecord* record, unsigned maxDepth);



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


/* The root dictionary is the same as "/" in text paths.
   Both name and type must be equal to '0x05' (see cdp_process.h).
*/
cdpRecord ROOT = {.metadata = {.nameID = 5, typeID = 5, .reStyle = CDP_REC_STYLE_DICTIONARY, .stoTech = CDP_STO_CHD_ARRAY}};


/*
    Initiates the record system.
*/
void cdp_record_system_initiate(void) {
    assert(!ROOT.recData.book.children);
    cdpArray* array = array_new(8);
    array->parentEx.book = &ROOT;
    ROOT.recData.book.children = array;
}


/* 
    Shutdowns the record system.
*/
void cdp_record_system_shutdown(void) {
    assert(ROOT.recData.book.children);
    cdpArray* array = ROOT.recData.book.children;
    ROOT.recData.book.children = NULL;
    array_del(array);
}
 
static inline void* record_create_storage(unsigned storage, va_list args) {
    STORAGE_TECH_SELECT(storage) {
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


/* 
    Creates a new record of the specified style on parent book.
*/
cdpRecord* cdp_record_create(cdpRecord* parent, unsigned style, cdpNameID nameID, uint32_t typeID, bool prepend, bool priv, ...) {
    assert(cdp_record_is_book_or_dic(parent) && nameID && typeID);
    cdpParentEx* parentEx = CDP_PARENTEX(parent->recData.book.children);
    if (cdp_record_is_dictionary(parent))
        assert(!prepend && !parentEx->compare);    // Only sorting by name is allowed here.
    cdpRecMeta metadata = {.proFlag = priv? CDP_FLAG_PRIVATE: 0, .reStyle = style, .typeID = typeID, .nameID = nameID};
    cdpRecord* child;
    
    // Add new record to parent book/dict.
    STORAGE_TECH_SELECT(parent->metadata.stoTech) {
      LINKED_LIST: {
        child = list_add(parent->recData.book.children, parent, prepend, &metadata);
        break;
      }
      ARRAY: {
        child = array_add(parent->recData.book.children, parent, prepend, &metadata);
        break;
      }
      PACKED_QUEUE: {
        child = packed_q_add(parent->recData.book.children, parent, prepend, &metadata);
        break;
      }        
      RED_BLACK_T: {
        child = rb_tree_add(parent->recData.book.children, parent, &metadata);
        break;
      }
    } SELECTION_END;
    
    // Update child.
    child->storage = parentEx;
    
    // Update parent.
    parentEx->chdCount++;

    // Create child record storage.
    //
    va_list args;
    va_start(args, priv);

    RECORD_STYLE_SELECT(style) {
      BOOK: {
        unsigned storage = va_arg(args, unsigned);
        assert(storage != CDP_STO_CHD_RED_BLACK_T);
        
        cdpParentEx* chdParentEx = record_create_storage(storage, args);
        
        // Link child book with its own (grand) child storage.
        chdParentEx->book = child;
        child->metadata.stoTech = storage;
        child->recData.book.children = chdParentEx;
        break;
      }
      
      DICTIONARY: {
        unsigned   storage = va_arg(args, unsigned);
        cdpCompare compare = va_arg(args, cdpCompare);
        void*      context = va_arg(args, void*);
        assert(storage != CDP_STO_CHD_PACKED_QUEUE);
      
        cdpParentEx* chdParentEx = record_create_storage(storage, args);
        
        // Link child dictionary with its own (grand) child storage.
        chdParentEx->compare = compare;
        chdParentEx->context = context;
        chdParentEx->book = child;
        child->metadata.stoTech = storage;
        child->recData.book.children = chdParentEx;
        break;
      }
      
      REGISTER: {
        bool   borrow = va_arg(args, unsigned);
        void*  data   = va_arg(args, void*);
        size_t size   = va_arg(args, size_t);
        assert(size);
        
        // Allocate storage for child register.
        if (borrow) {
            assert(priv && data);
            child->recData.reg.data.ptr = data;
            child->metadata.stoTech = CDP_STO_REG_BORROWED;
        } else if (data) {
            child->recData.reg.data.ptr = cdp_malloc(size);
            memcpy(child->recData.reg.data.ptr, data, size);
        }
        else
            child->recData.reg.data.ptr = cdp_malloc0(size);
        child->recData.reg.size = size;
        break;
      }
        
      LINK:
        break;
    } SELECTION_END;

    va_end(args);
        
    return child;
}




/*
   Reads register data from position and puts it on data buffer (atomically).
*/
void* cdp_record_register_read(cdpRecord* reg, size_t position, void* data, size_t* size) {
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

    assert(cdp_record_is_private(reg));
    return pointed;
}


/*
   Writes the data of a register record at position (atomically and it may reallocate memory).
*/
void* cdp_record_register_write(cdpRecord* reg, size_t position, const void* data, size_t size) {
    assert(cdp_record_is_register(reg) && data && size);

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
    Constructs the full path (sequence of nameIDs) for a given record, returning the depth.
    The cdpPath structure may be reallocated.
*/
#define CDP_RECORD_PATH_INITIAL_LENGTH  16

bool cdp_record_path(cdpRecord* record, cdpPath** path) {
    assert(record && path);
    
    cdpPath* tempPath;
    if (*path) {
        tempPath = *path;
        assert(tempPath->capacity);
    } else {
        tempPath = cdp_dyn_malloc(cdpPath, cdpNameID, CDP_RECORD_PATH_INITIAL_LENGTH);    // FixMe: pre-compute this to be power of 2.
        tempPath->capacity = CDP_RECORD_PATH_INITIAL_LENGTH;
        *path = tempPath;
    }
    tempPath->length = 0;
    
    // Traverse up the hierarchy to construct the path in reverse order
    for (cdpRecord* current = record;  current;  current = cdp_record_parent(current)) {  // FixMe: assuming single parenthood for now.
        if (tempPath->length >= tempPath->capacity) {
            unsigned newCapacity = tempPath->capacity * 2;
            cdpPath* newPath = cdp_dyn_malloc(cdpPath, cdpNameID, newCapacity);     // FixMe: use realloc.
            memcpy(&newPath->nameID[tempPath->capacity], tempPath->nameID, tempPath->capacity);
            newPath->length   = tempPath->capacity;
            newPath->capacity = newCapacity;
            CDP_PTR_OVERW(tempPath, newPath);
            *path = tempPath;
        }

        // Prepend the current record's nameID to the path
        tempPath->nameID[tempPath->capacity - tempPath->length - 1] = current->metadata.nameID;
        tempPath->length++;
    }

    return true;
}


/*
    Gets the first (or last) record from a book.
*/
cdpRecord* cdp_record_top(cdpRecord* book, bool last) {
    assert(cdp_record_is_book_or_dic(book));
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);
    CDP_CK(parentEx->chdCount);
    cdpRecord* record;
    
    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_top(book->recData.book.children, last);
        break;
      }
      ARRAY: {
        record = array_top(book->recData.book.children, last);
        break;
      }
      PACKED_QUEUE: {
        record = packed_q_top(book->recData.book.children, last);
        break;
      }
      RED_BLACK_T: {
        record = rb_tree_top(book->recData.book.children, last);
        break;
      }
    } SELECTION_END;
    
    return record;
}



/*
    Retrieves a child record by its nameID.
*/
cdpRecord* cdp_record_by_name(cdpRecord* book, cdpNameID nameID) {
    assert(cdp_record_is_book_or_dic(book) && nameID);
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);
    CDP_CK(parentEx->chdCount);
    cdpRecord* record;
    
    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_find_by_name(book->recData.book.children, nameID);
        break;
      }
      ARRAY: {
        record = array_find_by_name(book->recData.book.children, nameID, book);
        break;
      }
      PACKED_QUEUE: {
        record = packed_q_find_by_name(book->recData.book.children, nameID);
        break;
      }
      RED_BLACK_T: {
        assert(cdp_record_is_dictionary(book));
        record = rb_tree_find_by_name(book->recData.book.children, nameID, book);
        break;
      }
    } SELECTION_END;

    return record;
}



/*
    Finds a child record based on specified key.
*/
cdpRecord* cdp_record_by_key(cdpRecord* book, cdpRecord* key) {
    assert(cdp_record_is_book_or_dic(book) && key);
    // ToDo: fully define key storage system.
    return NULL;
}


/*
    Gets the record at index position from book.
*/
cdpRecord* cdp_record_by_index(cdpRecord* book, size_t index) {
    assert(cdp_record_is_book_or_dic(book));
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);
    CDP_CK(parentEx->chdCount);
    assert(index < parentEx->chdCount);
    cdpRecord* record;

    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_find_by_index(book->recData.book.children, index);
        break;
      }
      ARRAY: {
        record = array_find_by_index(book->recData.book.children, index);
        break;
      }
      PACKED_QUEUE: {
        record = packed_q_find_by_index(book->recData.book.children, index);
        break;
      }
      RED_BLACK_T: {
        record = rb_tree_find_by_index(book->recData.book.children, index, book);
        break;
      }
    } SELECTION_END;

    return record;
}


/*
    Gets the record by its path from start record.
*/
cdpRecord* cdp_record_by_path(cdpRecord* start, const cdpPath* path) {
    assert(cdp_record_is_book_or_dic(start) && path && path->length);
    CDP_CK(cdp_record_book_or_dic_children(start));
    cdpRecord* record = start;
    
    for (unsigned depth = 0;  depth < path->length;  depth++) {
        record = cdp_record_by_name(record, path->nameID[depth]);
        if (!record) return NULL;
    }

    return record;
}





/*
    Retrieves the previous sibling of record.
*/
cdpRecord* cdp_record_prev(cdpRecord* book, cdpRecord* record) {
    assert(record);
    cdpParentEx* parentEx;
    if (book) {
        assert(cdp_record_is_book_or_dic(book));
        parentEx = CDP_PARENTEX(book->recData.book.children);
    } else {
        parentEx = cdp_record_parent_ex(record);
        book = parentEx->book;
    }
    CDP_CK(parentEx->chdCount);
    cdpRecord* prev;

    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        prev = list_prev(record);
        break;
      }
      ARRAY: {
        prev = array_prev(book->recData.book.children, record);
        break;
      }
      PACKED_QUEUE: {
        prev = packed_q_prev(book->recData.book.children, record);
        break;
      }
      RED_BLACK_T: {
        prev = rb_tree_prev(record);
        break;
      }
    } SELECTION_END;

    return prev;
}


/*
    Retrieves the next sibling of record (sorted or unsorted).
*/
cdpRecord* cdp_record_next(cdpRecord* book, cdpRecord* record) {
    assert(record);
    cdpParentEx* parentEx;
    if (book) {
        assert(cdp_record_is_book_or_dic(book));
        parentEx = CDP_PARENTEX(book->recData.book.children);
    } else {
        parentEx = cdp_record_parent_ex(record);
        book = parentEx->book;
    }
    CDP_CK(parentEx->chdCount);
    cdpRecord* next;

    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        next = list_next(record);
        break;
      }
      ARRAY: {
        next = array_next(book->recData.book.children, record);
        break;
      }
      PACKED_QUEUE: {
        next = packed_q_next(book->recData.book.children, record);
        break;
      }
      RED_BLACK_T: {
        next = rb_tree_next(record);
        break;
      }
    } SELECTION_END;

    return next;
}




/*
    Retrieves the first/next child record by its nameID.
*/
cdpRecord* cdp_record_next_by_name(cdpRecord* book, cdpNameID nameID, uintptr_t* childIdx) {
    if (cdp_record_is_dictionary(book) || !childIdx) {
        CDP_PTR_SEC_SET(childIdx, 0);
        return cdp_record_by_name(book, nameID);
    }
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);    
    CDP_CK(parentEx->chdCount);
    cdpRecord* record;
    
    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_next_by_name(book->recData.book.children, nameID, (cdpListNode**)childIdx);
        break;
      }
      ARRAY: {
        record = array_next_by_name(book->recData.book.children, nameID, childIdx);
        break;
      }
      PACKED_QUEUE: {
        record = packed_q_next_by_name(book->recData.book.children, nameID, (cdpPackedQNode**)childIdx);
        break;
      }
      RED_BLACK_T: {    // Unused.
        break;
      }
    } SELECTION_END;

    return record;
}


/*
    Gets the next record with the (same) nameID as specified for each branch.
*/
cdpRecord* cdp_record_next_by_path(cdpRecord* start, cdpPath* path, uintptr_t* prev) {
    assert(cdp_record_is_book_or_dic(start) && path && path->length);
    CDP_CK(cdp_record_book_or_dic_children(start));
    cdpRecord* record = start;
    
    for (unsigned depth = 0;  depth < path->length;  depth++) {
        // FixMe: depth must be stored in a stack as well!
        // ...(pending)
        record = cdp_record_next_by_name(record, path->nameID[depth], prev);
        if (!record) return NULL;
    }

    return record;
}


/*
    Traverses the children of a book record, applying a function to each.
*/
bool cdp_record_traverse(cdpRecord* book, cdpRecordTraverse func, void* context) {
    assert(cdp_record_is_book_or_dic(book) && func);
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);    
    CDP_GO(!parentEx->chdCount);
    bool done;

    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        done = list_traverse(book->recData.book.children, book, func, context);
        break;
      }
      ARRAY: {
        done = array_traverse(book->recData.book.children, book, func, context);
        break;
      }
      PACKED_QUEUE: {
        done = packed_q_traverse(book->recData.book.children, book, func, context);
        break;
      }
      RED_BLACK_T: {
        done = rb_tree_traverse(book->recData.book.children, book, cdp_bitson(parentEx->chdCount) + 2, func, context);
        break;
      }
    } SELECTION_END;
    
    return done;
}


/*
    Traverses each child branch and sub-branch of a book record, applying a function to each.
*/
#define CDP_MAX_FAST_STACK_DEPTH  16

bool cdp_record_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, cdpRecordTraverse endFunc, void* context) {
    assert(cdp_record_is_book_or_dic(book) && maxDepth && (func || endFunc));
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);    
    CDP_GO(!parentEx->chdCount);
    
    bool ok = true;
    cdpRecord* child;
    unsigned depth = 0;
    cdpBookEntry entry = {.record = cdp_record_top(book, false), .parent = book};
    
    // Non-recursive version of branch descent:
    cdpBookEntry* stack = (maxDepth > CDP_MAX_FAST_STACK_DEPTH)?  cdp_malloc(maxDepth * sizeof(cdpBookEntry)):  cdp_alloca(maxDepth * sizeof(cdpBookEntry));
    for (;;) {
        // Ascend to parent if no more siblings in branch.
        if (!entry.record) {
            if CDP_RARELY(!depth)  break;    // endFunc is never called on root book.
            depth--;
            
            if (endFunc) {
                ok = endFunc(&stack[depth], depth, context);
                if (!ok)  break;
            }
            
            // Next record.
            entry.record = stack[depth].next;
            entry.parent = stack[depth].parent;
            entry.prev   = stack[depth].record;
            entry.next   = NULL;
            entry.index  = stack[depth].index + 1;
            continue;
        }
        
      NEXT_SIBLING:
        // Get sibling...
        STORAGE_TECH_SELECT(entry.parent->metadata.stoTech) {
          LINKED_LIST: {
            entry.next = list_next(entry.record);
            break;
          }
          ARRAY: {
            entry.next = array_next(entry.record->storage, entry.record);
            break;
          }
          PACKED_QUEUE: {
            entry.next = packed_q_next(entry.record->storage, entry.record);
            break;
          }
          RED_BLACK_T: {
            entry.next = rb_tree_next(entry.record);
            break;
          }
        } SELECTION_END;

        if (func) {
            ok = func(&entry, depth, context);
            if (!ok)  break;
        }
        
        // Descent to children if it's a book.
        if (cdp_record_is_book_or_dic(entry.record)
        && ((child = cdp_record_top(entry.record, false)))) {
            assert(depth < maxDepth);
            
            stack[depth++] = entry;
            
            entry.parent = entry.record;
            entry.record = child;
            entry.prev   = NULL;
            entry.index  = 0;
            
            goto NEXT_SIBLING;
        }

        // Next record.
        entry.prev   = entry.record;
        entry.record = entry.next;
        entry.index += 1;
    }
    
    if (maxDepth > CDP_MAX_FAST_STACK_DEPTH)
        cdp_free(stack);
    
    return ok;
}




/*
    Converts an unsorted book into a dictionary.
*/
void cdp_record_sort(cdpRecord* book, cdpCompare compare, void* context) {
    CDP_AB(cdp_record_is_dictionary(book));
    assert(cdp_record_is_book(book));
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);
    assert(!parentEx->compare);
    
    book->metadata.reStyle = CDP_REC_STYLE_DICTIONARY;
    parentEx->compare = compare? compare: record_compare_by_name_s;
    parentEx->context = context;    
    CDP_AB(parentEx->chdCount <= 1);
    
    STORAGE_TECH_SELECT(book->metadata.stoTech) {
      LINKED_LIST: {
        list_sort(book->recData.book.children, compare, context);
        break;
      }
      ARRAY: {
        array_sort(book->recData.book.children, compare, context);
        break;
      }
      PACKED_QUEUE: {
        assert(book->metadata.stoTech == CDP_STO_CHD_PACKED_QUEUE);    // Unsupported.
        break;
      }
      RED_BLACK_T: {    // Unused.
        break;
      }
    } SELECTION_END;
}


static inline void record_delete_storage(cdpRecord* record, unsigned maxDepth) {
    assert(record && maxDepth);
    assert(!cdp_record_has_shadows(record));

    // Delete storage (and children).
    RECORD_STYLE_SELECT(record->metadata.reStyle) {
      BOOK:;
      DICTIONARY: {
        STORAGE_TECH_SELECT(record->metadata.stoTech) {
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
        break;
      }
      
      REGISTER: {
        if (!cdp_record_register_is_borrowed(record))
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
    assert(!cdp_record_has_shadows(record));
    cdpParentEx* parentEx = cdp_record_parent_ex(record);
    cdpRecord* book = parentEx->book;

    // Delete storage (along children, if any).
    record_delete_storage(record, maxDepth);
       
    // Remove this record from its parent (re-organizing siblings).
    STORAGE_TECH_SELECT(book->metadata.stoTech) {
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
    
    parentEx->chdCount--;
        
    return true;
}


/*
    Deletes all children of a book/dictionary.
*/
size_t cdp_record_book_reset(cdpRecord* book, unsigned maxDepth) {
    assert(cdp_record_is_book_or_dic(book) && maxDepth);
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);
    size_t children = parentEx->chdCount;
    CDP_CK(children);

    STORAGE_TECH_SELECT(book->metadata.stoTech) {
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
    
    parentEx->chdCount = 0;
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

