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
#include <search.h>   // For lfind()
#include <alloca.h>




/* 
    Creates a new record of the specified style on parent book.
    
    Collection styles:
    * Array: this is never sorted after insertion so each item *must* be added
    in a sorted order.

*/
cdpRecord* cdp_record_create(cdpRecord* parent, unsigned style, cdpNameID nameID, uint32_t typeID, bool push, bool priv, ...) {
    assert(cdp_record_is_book(parent) && nameID && typeID);
    cdpRecord* child;
    va_list args;
    va_start(args, priv);
    
    // Fast switch-case for child storage techniques:
    assert(parent->metadata.chdSto < CDP_CHD_STO_COUNT);
    static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
    goto *chdStoTech[parent->metadata.chdSto];
    do {
      CIRC_BUFFER:
        break;
        
      LINKED_LIST:
        break;
        
      ARRAY: {
        assert(!push);      // Push is not allowed for array (use circular buffer instead).   
        //
        cdpArray* array = parent->recData.book.children;
        assert(array && array->capacity > parent->recData.book.chdCount);
        child = &array->record[parent->recData.book.chdCount];
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
        
      B_TREE:
        break;
    } while(0);
    
    // Fill in the metadata
    child->metadata.propFg = priv? CDP_FLAG_PRIVATE: 0;
    child->metadata.rStyle = style;
    child->metadata.typeID = typeID;
    child->metadata.nameID = nameID;
    child->parent.single = parent;
    
    // Update parent
    parent->recData.book.chdCount++;
    
    // Fast switch-case for record styles:
    assert(style < CDP_REC_STYLE_COUNT);
    static const void* const recordStyle[] = {&&REGISTER, &&BOOK, &&LINK};
    goto *recordStyle[style];
    do {
      REGISTER: {
        bool   borrow = va_arg(args, bool);
        void*  data   = va_arg(args, void*);
        size_t size   = va_arg(args, size_t);
        assert(size);
        
        // Allocate storage for register
        if (borrow) {
            assert(priv && data);
            child->recData.reg.data.ptr = data;
            child->metadata.chdSto = 1;   // Points to borrowed memory.
        }
        else if (data) {
            child->recData.reg.data.ptr = cdp_malloc(size);
            memcpy(child->recData.reg.data.ptr, data, size);
        }
        else
            child->recData.reg.data.ptr = cdp_malloc0(size);
        child->recData.reg.size = size;
        break;
      }
        
      BOOK: {
        unsigned storage = va_arg(args, unsigned);
        unsigned sorting = va_arg(args, unsigned);
        assert(storage < CDP_CHD_STO_COUNT && sorting < CDP_BOOK_SORT_COUNT);
        assert(!(sorting != CDP_BOOK_SORT_NONE && storage < CDP_CHD_SORTABLE));
        
        child->metadata.sorted = sorting;
        
        if (sorting == CDP_BOOK_SORT_BY_KEY) {
            // Inserts this new record using the key stored on the siblingKeyID field.
            cdpNameID siblingKeyID = va_arg(args, unsigned);
            cdpCmp compare = va_arg(args, unsigned);
            const void* key = va_arg(args, unsigned);
            size_t kSize = va_arg(args, unsigned);
            assert(siblingKeyID && compare && key && kSize);
        }

        break;
      }
      
      LINK:
        break;
    } while(0);

    va_end(args);
        
    return child;
}




/*
   Reads register data from position and puts it on data buffer (atomically).
*/
bool cdp_record_register_read(cdpRecord* reg, size_t position, void** data, size_t* size) {
    assert(cdp_record_is_register(reg) && data && size && *size);

    // Calculate the actual number of bytes that can be read
    size_t readableSize = reg->recData.reg.size - position;
    CDP_TRUNCATE(*size, readableSize);    // Adjust the size to the maximum readable amount

    // Copy the data from the register to the provided buffer
    void* pointed = cdp_ptr_off(reg->recData.reg.ptr, position);
    if (*data)
        memcpy(*data, pointed, *size);
    else {
        assert(reg->metadata.propFg & CDP_FLAG_PRIVATE);
        *data = pointed;
    }

    return true;
}


/*
   Writes the data of a register record at position (atomically and it may reallocate memory).
*/
bool cdp_record_register_write(cdpRecord* reg, size_t position, const void* data, size_t size) {
    assert(cdp_record_is_register(reg) && data && size);

    // Ensure the buffer is large enough to accommodate the write
    size_t newSize = position + size;
    if (newSize > reg->recData.reg.size) {
        // Resize the buffer
        reg->recData.reg.ptr  = cdp_realloc(reg->recData.reg.ptr, newSize);
        reg->recData.reg.size = newSize;
    }

    // Copy the provided data into the register's buffer at the specified position
    memcpy(cdp_ptr_off(reg->recData.reg.ptr, position), data, size);

    return true;
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
    }
    else {
        tempPath = cdp_dyn_malloc(cdpPath, cdpNameID, CDP_RECORD_PATH_INITIAL_LENGTH);    // FixMe: pre-compute this to be power of 2.
        tempPath->capacity = CDP_RECORD_PATH_INITIAL_LENGTH;
    }
    tempPath->length = 0;
    
    // Traverse up the hierarchy to construct the path in reverse order
    for (cdpRecord* current = record;  current;  current = current->parent.single) {  // FixMe: assuming single parenthood for now.
        if (tempPath->length >= tempPath->capacity) {
            unsigned newCapacity = tempPath->capacity * 2;
            cdpPath* newPath = cdp_dyn_malloc(cdpPath, cdpNameID, newCapacity);
            memcpy(&newPath->nameID[tempPath->capacity], tempPath->nameID, tempPath->capacity);
            newPath->length   = tempPath->capacity;
            newPath->capacity = newCapacity;
            cdp_ptr_overw(tempPath, newPath);
        }

        // Prepend the current record's nameID to the path
        tempPath[tempPath->capacity - tempPath->length - 1] = current->metadata.nameID;
        tempPath->length++;
    }

    *path = tempPath;

    return true;
}


/*
    Gets the first (or last) record from a book.
*/
cdpRecord* cdp_record_top(cdpRecord* book, bool last) {
    assert(cdp_record_is_book(book));
    CDP_CK(book->recData.book.chdCount);
    assert(book->recData.book.children);
    cdpRecord* record;
    
     // Fast switch-case for child storage techniques:
    assert(book->metadata.chdSto < CDP_CHD_STO_COUNT);
    static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
    goto *chdStoTech[book->metadata.chdSto];
    do {
      CIRC_BUFFER:
        break;
        
      LINKED_LIST:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array && array->capacity > book->recData.book.chdCount);
        record = last?  &array->record[book->recData.book.chdCount - 1]:  array->record;
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
        
      B_TREE:
        break;
    } while(0);
    
    return record;
}



/*
    Retrieves a child record by its nameID.
*/
cdpRecord* cdp_record_by_name(cdpRecord* book, cdpNameID nameID) {
    assert(cdp_record_book_sorted(book) && nameID);
    CDP_CK(book->recData.book.chdCount);
    assert(book->recData.book.children);
    cdpRecord* record;
    
    // Fast switch-case for children sorting method:
    assert(book->metadata.sorted < CDP_BOOK_SORT_COUNT);
    static const void* const bookSorting[] = {NULL, &&BY_NAME, &&BY_DATA, &&BY_KEY};
    goto *bookSorting[book->metadata.sorted];
    do {
      // Sorted search:
      BY_NAME: {
        cdpRecord key = {.nameID = nameID};
        
        // Fast switch-case for child storage techniques:
        assert(book->metadata.chdSto < CDP_CHD_STO_COUNT && book->metadata.chdSto >= CDP_CHD_SORTABLE);
        static const void* const chdStoTech[] = {NULL, NULL, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
        goto *chdStoTech[book->metadata.chdSto];
        do {
          ARRAY: {
            return bsearch(&key, book->recData.book.children, book->recData.book.chdCount, sizeof(cdpRecord), cdp_record_compare_by_name);
          }
            
          PACKED_LIST: {
            break;
          }
            
          RED_BLACK_T:
            break;
            
          B_TREE:
            break;

        } while(0);
        break;
      }

      BY_DATA:; BY_KEY: {
        // FixMe: add mising sorting methods
        assert(book->metadata.sorted != CDP_BOOK_SORT_BY_NAME);
      }
    } while(0);

    return NULL;
}



/*
    Finds a child record based on specified key.
*/
cdpRecord* cdp_record_by_key(cdpRecord* book, cdpNameID siblingKeyID, cdpCmp compare, const void* key, size_t kSize) {
    assert(cdp_record_is_book(book) && siblingKeyID && compare && key && kSize);
    // ToDo: fully define key storage system.
    return NULL;
}


/*
    Gets the record at index position from book.
*/
cdpRecord* cdp_record_by_index(cdpRecord* book, size_t index) {
    assert(cdp_record_is_book(book) && index <= book->recData.book.chdCount);
    CDP_CK(book->recData.book.chdCount);
    cdpRecord* record;

    // Fast switch-case for child storage techniques:
    assert(book->metadata.chdSto < CDP_CHD_STO_COUNT);
    static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
    goto *chdStoTech[book->metadata.chdSto];
    do {
      CIRC_BUFFER:
        break;
        
      LINKED_LIST:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array && array->capacity >= book->recData.book.chdCount);
        return &array->record[index];
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
        
      B_TREE:
        break;
    } while(0);

    return NULL;
}


/*
    Gets the record by its path from start record.
*/
cdpRecord* cdp_record_by_path(cdpRecord* start, const cdpPath* path) {
    assert(cdp_record_is_book(start) && path && path->length);
    CDP_CK(start->recData.book.chdCount);
    cdpRecord* record = start;
    
    for (unsigned depth = 0;  depth < path->length;  depth++) {
        record = cdp_record_by_name(record, path->nameID[depth]);
        if (!record) return NULL;
    }

    return record;
}


/*
    Retrieves the first/next child record by its nameID.
*/
cdpRecord* cdp_record_next_by_name(cdpRecord* book, cdpNameID nameID, uintptr_t* childIdx) {
    if (cdp_record_book_sorted(book) return cdp_record_by_name(book, nameID);
    
    CDP_CK(book->recData.book.chdCount);
    assert(book->recData.book.children );
    cdpRecord* record;
    
    // Fast switch-case for child storage techniques:
    assert(book->metadata.chdSto < CDP_CHD_NON_PUSHABLE);
    static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST};
    goto *chdStoTech[book->metadata.chdSto];
    do {
      CIRC_BUFFER:
        for (size_t i = startIdx; i < book->data.book.childrenCount; i++) {
            if (children->record[i].metadata.nameID == nameID) {
                if (childIdx) *childIdx = i + 1; // Prepare childIdx for the next search
                return &children->record[i]; // Found a match
            }
        }
        break;
        
      LINKED_LIST:
        break;
        
      ARRAY: {
        cdpRecord key = {.nameID = nameID};
        size_t start = childIdx? childIdx: book->recData.book.chdCount;
        return lfind(&key, book->recData.book.children, start, sizeof(cdpRecord), cdp_record_compare_by_name);
      }
       
      PACKED_LIST:
        break;
      }
    }

    return NULL;
}


/*
    Gets the next record with the (same) nameID as specified for each branch.
*/
cdpRecord* cdp_record_next_unsorted_by_path(cdpRecord* start, cdpNameID* path, uintptr_t* childIdx) {
    assert(cdp_record_is_book(start) && path && path->length);
    CDP_CK(start->recData.book.chdCount);
    cdpRecord* record = start;
    
    for (unsigned depth = 0;  depth < path->length;  depth++) {
        // FixMe: depth must be stored in a stack as well!
        // ...(pending)
        record = cdp_record_next_by_name(record, path->nameID[depth], childIdx);
        if (!record) return NULL;
    }

    return record;
}


/*
    Traverses the children of a book record, applying a function to each.
*/
bool cdp_record_traverse(cdpRecord* book, cdpRecordTraverse func, void* context) {
    assert(cdp_record_is_book(book) && func);
    CDP_GO(!book->recData.book.chdCount);
    cdpRecord* record;

    // Fast switch-case for child storage techniques:
    assert(book->metadata.chdSto < CDP_CHD_STO_COUNT);
    static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
    goto *chdStoTech[book->metadata.chdSto];
    do {
      CIRC_BUFFER:
        break;
        
      LINKED_LIST:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array && array->capacity >= book->recData.book.chdCount);
        
        cdpBookEntry entry = {.record = array->record, .parent = book, .next = (book->recData.book.chdCount > 1)? (array->record + 1): NULL};
        while (entry.index < book->recData.book.chdCount) {
            if (!func(&entry, 0, context))  return false;
            
            entry.index++;
            entry.prev   = entry.record;
            entry.record = entry.next;
            entry.next++;
        }
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
        
      B_TREE:
        break;
    } while(0);
    
    return true;
}


/*
    Traverses each child branch and sub-branch of a book record, applying a function to each.
*/
#define CDP_MAX_FAST_STACK_DEPTH  16

bool cdp_record_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, cdpRecordTraverse listEnd, void* context) {
    assert(cdp_record_is_book(book) && maxDepth && (func || listEnd);
    CDP_GO(!book->recData.book.chdCount);
    
    bool ok = true;
    cdpRecord* child;
    unsigned depth = 0;
    cdpBookEntry entry = {.record = cdp_record_top(book, false), .parent = book};
    
    // Non-recursive version of branch descent:
    cdpBookEntry* stack = (maxDepth > CDP_MAX_FAST_STACK_DEPTH)?  cdp_malloc(maxDepth * sizeof(cdpBookEntry)):  alloca(maxDepth * sizeof(cdpBookEntry));
    for (;;) {
        // Ascend to parent if no more siblings in branch.
        if (!entry.record) {
            if UNIP_RARELY(!depth)  break;    // listEnd is never called on root book.
            depth--;
            
            if (listEnd) {
                ok = listEnd(&stack[depth], depth, context);
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
        // Fast switch-case for child storage techniques:
        assert(entry.parent->metadata.chdSto < CDP_CHD_STO_COUNT);
        static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
        goto *chdStoTech[entry.parent->metadata.chdSto];
        do {
          CIRC_BUFFER:
            break;
            
          LINKED_LIST:
            break;
            
          ARRAY: {
            cdpArray* array = entry.parent->recData.book.children;
            assert(array && array->capacity >= entry.parent->recData.book.chdCount);
            entry.next = (entry.index < entry.parent->recData.book.chdCount)?  entry.record + 1:  NULL;
            break;
          }
          
          PACKED_LIST:
            break;
            
          RED_BLACK_T:
            break;
            
          B_TREE:
            break;
        } while(0);

        if (func) {
            ok = func(&entry, depth, context);
            if (!ok)  break;
        }
        
        // Descent to children if it's a book.
        if (cdp_record_is_book(record) && ((child = cdp_record_top(record, false)))) {
            assert(depth < maxDepth);
            
            stack[depth++] = entry;
            
            entry.record = child;
            entry.parent = entry.record;
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




static bool record_unlink(cdpBookEntry* entry, unsigned depth, void* p) {
    assert(entry && entry->record && entry->parent);

    // Fast switch-case for record styles:
    assert(entry.record->metadata.rStyle < CDP_REC_STYLE_COUNT);
    static const void* const recordStyle[] = {&&REGISTER, &&BOOK, &&LINK};
    goto *recordStyle[entry.record->metadata.rStyle];
    do {
      REGISTER: {
        if (entry.record->metadata.chdSto != 0)   // Data isn't borrowed
            cdp_free(entry.record->recData.reg.data.ptr);
        break;
      }
        
      BOOK: {
        break;
      }
      
      LINK: {
        // Go to pointed record and unlink this from parent list.
        break;
      }
    } while(0);
        
    return true;
}

static bool record_del_store(cdpBookEntry* entry, unsigned depth, void* p) {
    assert(entry && entry->record && entry->parent);
    
    // Fast switch-case for record styles:
    assert(entry.record->metadata.rStyle < CDP_REC_STYLE_COUNT);
    static const void* const recordStyle[] = {&&REGISTER, &&BOOK, &&LINK};
    goto *recordStyle[entry.record->metadata.rStyle];
    do {
      REGISTER: {
        // This shouldn't happen.
        assert(cdp_record_is_book(entry.record));
        break;
      }
        
      BOOK: {
        // Fast switch-case for child storage techniques:
        assert(entry.record->metadata.chdSto < CDP_CHD_STO_COUNT);
        static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
        goto *chdStoTech[entry.record->metadata.chdSto];
        do {
          CIRC_BUFFER:
            break;
            
          LINKED_LIST:
            break;
            
          ARRAY: {
            cdp_free(entry.record->recData.book.children);
            break;
          }
          
          PACKED_LIST:
            break;
            
          RED_BLACK_T:
            break;
            
          B_TREE:
            break;
        } while(0);
        break;
      }
      
      LINK:
        break;
    } while(0);
            
    return true;
}


/*
    Deletes a record and all its children re-organizing sibling storage
*/
bool cdp_record_delete(cdpRecord* record, unsigned maxDepth) {
    assert(record);
    
    assert(!cdp_record_w_coparents(record));
    cdpRecord* book = record->parent.single;
    
    // Fast switch-case for record styles:
    assert(record->metadata.rStyle < CDP_REC_STYLE_COUNT);
    static const void* const recordStyle[] = {&&REGISTER, &&BOOK, &&LINK};
    goto *recordStyle[record->metadata.rStyle];
    do {
      REGISTER: {
        record_unlink(&(cdpBookEntry){.record=record, .parent=book}, 0, NULL);
        break;
      }
        
      BOOK: {
        cdp_record_deep_traverse(record, maxDepth, record_unlink, record_del_store, NULL);
        break;
      }
      
      LINK:
        break;
    } while(0);
       
    /* Delete from parent (re-organize siblings) */
    
    // Fast switch-case for child storage techniques:
    assert(book->metadata.chdSto < CDP_CHD_STO_COUNT);
    static const void* const chdStoTech[] = {&&CIRC_BUFFER, &&LINKED_LIST, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T, &&B_TREE};
    goto *chdStoTech[book->metadata.chdSto];
    do {
      CIRC_BUFFER:
        break;
        
      LINKED_LIST:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array && array->capacity >= book->recData.book.chdCount);
        assert(record == &array->record[book->recData.book.chdCount - 1]);    // Only the last record may be removed.
        // No need to do anything for array siblings.
        break;
      }
      
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
        
      B_TREE:
        break;
    } while(0);
    
    book->recData.book.chdCount--;
        
    return true;
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
