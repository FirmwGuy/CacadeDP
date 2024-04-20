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
#include <alloca.h>




static int record_compare_by_name(const cdpRecord* restrict key, const cdpRecord* restrict record, void* context) {
    //if (context) *(cdpRecord**)context = record;
    return key->metadata.nameID - record->metadata.nameID;
}

static int rec_cmp_by_nameID(const cdpRecord* restrict key, const cdpRecord* restrict record) {
    return key->metadata.nameID - record->metadata.nameID;
}




/*
   Utilities for arrays
*/
.static inline cdpRecord* array_search(cdpArray* array, void* key, size_t* index) {
    size_t imax = cdp_ptr_has_val(index)? *index - 1: array->parent.chdCount - 1;
    size_t imin = 0, i;
    cdpRecord* record;
    do {
        i = (imax + imin) >> 1;  // (max + min) / 2
        record = &array->record[i];
        int res = array->parent.compare(key, record, array->parent.context);
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
    while (record <= last){
        if (cdp_record_is_book_or_dic(record)) {
            cdpParentEx* parentEx = record->recData.book.children;
            parentEx->book = record;    // Update grandchild link to its parent.
        }
        record++;
    }
}



/*
    Utilities for Red-black tree rebalancing.
*/
static inline void rb_tree_rotate_left(cdpRbTree* tree, cdpRbTreeNode* x) {
    cdpRbTreeNode* y = x->right;
    x->right = y->left;
    if (y->left)
        y->left->tParent = x;
    y->tParent = x->tParent;
    if (!x->tParent)
        tree->root = y;
    else if (x == x->tParent->left)
        x->tParent->left = y;
    else
        x->tParent->right = y;
    y->left = x;
    x->tParent = y;
}

static inline void rb_tree_rotate_right(cdpRbTree* tree, cdpRbTreeNode* x) {
    cdpRbTreeNode* y = x->left;
    x->left = y->right;
    if (y->right)
        y->right->tParent = x;
    y->tParent = x->tParent;
    if (!x->tParent)
        tree->root = y;
    else if (x == x->tParent->right)
        x->tParent->right = y;
    else
        x->tParent->left = y;
    y->right = x;
    x->tParent = y;
}

static inline void rb_tree_fix_insert(cdpRbTree* tree, cdpRbTreeNode* z) {
    while (z != tree->root && z->tParent->isRed) {
        if (z->tParent == z->tParent->tParent->left) {
            cdpRbTreeNode* y = z->tParent->tParent->right;
            if (y && y->isRed) {
                z->tParent->isRed = false;
                y->isRed = false;
                z->tParent->tParent->isRed = true;
                z = z->tParent->tParent;
            } else {
                if (z == z->tParent->right) {
                    z = z->tParent;
                    rb_tree_rotate_left(tree, z);
                }
                z->tParent->isRed = false;
                z->tParent->tParent->isRed = true;
                rb_tree_rotate_right(tree, z->tParent->tParent);
            }
        } else {
            cdpRbTreeNode* y = z->tParent->tParent->left;
            if (y && y->isRed) {
                z->tParent->isRed = false;
                y->isRed = false;
                z->tParent->tParent->isRed = true;
                z = z->tParent->tParent;
            } else {
                if (z == z->tParent->left) {
                    z = z->tParent;
                    rb_tree_rotate_right(tree, z);
                }
                z->tParent->isRed = false;
                z->tParent->tParent->isRed = true;
                rb_tree_rotate_left(tree, z->tParent->tParent);
            }
        }
    }
    tree->root->isRed = false;
}



/* 
    Creates a new record of the specified style on parent book.
*/
cdpRecord* cdp_record_create(cdpRecord* parent, unsigned style, cdpNameID nameID, uint32_t typeID, bool push, bool priv, ...) {
    assert(cdp_record_is_book_or_dic(parent) && nameID && typeID);
    cdpParentEx* parentEx = CDP_PARENTEX(parent->recData.book.children);
    cdpRecord metadata = {.metadata = {.proFlag = priv? CDP_FLAG_PRIVATE: 0; .reStyle = style, .typeID = typeID, .nameID = nameID}};
    cdpRecord* child;
    va_list args;
    va_start(args, priv);
    
    // Fast switch-case for (parent's) child storage techniques:
    assert(parent->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[parent->metadata.stoTech];
    do {
      LINKED_LIST:
      {
        cdpList* list = parent->recData.book.children;
        if (cdp_record_is_dictionary(parent)) {
            // Sorted insert
            assert(parentEx->compare);
            // ToDo:sorted insert.
            assert(cdp_record_is_dictionary(parent));
        } else {
            CDP_NEW(cdpListNode, node);
            if (push) {
                // Prepend node
                node->prev = NULL;
                node->next = list->head;
                if (list->head)
                    list->head->prev = node;
                else
                    list->tail = node;
                list->head = node;
            } else {
                // Append node
                node->next = NULL;
                node->prev = list->tail;
                if (list->tail)
                    list->tail->next = node;
                else
                    list->head = node;
                list->tail = node;
            }
            child = &node->record;
        }
        break;
      }
        
      CIRC_BUFFER: {
        break;
      }
        
      ARRAY:
      {
        cdpArray* array = parent->recData.book.children;
        if (array->capacity == parentEx->chdCount) {
            // Increase array space if necessary
            assert(array->capacity);
            array->capacity *= 2;
            array->record = cdp_realloc(array->record, array->capacity * sizeof(cdpRecord));
            memset(&array->record[parentEx->chdCount], 0, parentEx->chdCount * sizeof(cdpRecord));
            array_update_children_parent_ptr(array->record, &array->record[parentEx->chdCount - 1]);
        }
        
        if (parentEx->chdCount) {
            if (cdp_record_is_dictionary(parent)) {
                // Sorted insert
                assert(parentEx->compare);
                size_t index = 0;
                cdpRecord* prev = array_search(array, &metadata, &index);
                if (prev) {
                    // FixMe: delete children.
                    assert(prev);
                }
                child = &array->record[index];
                memmove(child + 1, child, parentEx->chdCount * sizeof(cdpRecord)); 
                *child = metadata;
                array_update_children_parent_ptr(child + 1, &array->record[parentEx->chdCount - 1]);
            } else if (push) {
                // Prepend child
                child = array->record;
                memmove(child + 1, child, parentEx->chdCount * sizeof(cdpRecord)); 
                *child = metadata;
                array_update_children_parent_ptr(child + 1, &array->record[parentEx->chdCount - 1]);
            } else {
                // Append child
                child = &array->record[parentEx->chdCount];
                *child = metadata;
            }
        } else {
                child = array->record;
                *child = metadata;
        }
        break;
      }
       
      PACKED_LIST: {
        break;
      }
        
      RED_BLACK_T:
      {
        assert(cdp_record_is_dictionary(parent) && parentEx->compare);
        cdpRbTree* tree = &parent->recData.book.children;
        CDP_NEW(cdpRbTreeNode, tnode);
        tnode->isRed = true;
        child = &tnode->record;
        *child = metadata;
        if (tree->root) {
            cdpRbTreeNode* y = NULL;
            cdpRbTreeNode* x = tree->root;
            while (x) {
                y = x;
                if (0 > parentEx->compare(&metadata, x->record, parentEx->context)) {
                    x = x->left;
                } else {
                    x = x->right;
                }
            }
            tnode->tParent = y;
            if (0 > parentEx->compare(&metadata, y->record, parentEx->context)) {
                y->left = tnode;
            } else {
                y->right = tnode;
            }
            rb_tree_fix_insert(tree, tnode);
        } else {
            tree->root = tnode;
        }
        break;
      }
    } while(0);
    
    // Update child.
    child->storage = parentEx;
    
    // Update parent.
    parentEx->chdCount++;
    
    // Fast switch-case for record styles:
    assert(style < CDP_REC_STYLE_COUNT);
    static void* const recordStyle[] = {&&BOOK, &&DICTIONARY, &&REGISTER, &&LINK};
    goto *recordStyle[style];
    do {
      BOOK:;
      DICTIONARY: {
        unsigned storage = va_arg(args, unsigned);
        cdpParentEx* chdParentEx;

        // Fast switch-case for (child's) child storage techniques:
        assert(storage < CDP_CHD_STO_COUNT);
        static void* const reqStoTech[] = {&&REQ_LINKED_LIST, &&REQ_CIRC_BUFFER, &&REQ_ARRAY, &&REQ_PACKED_LIST, &&REQ_RED_BLACK_T};
        goto *reqStoTech[storage];
        do {
          REQ_LINKED_LIST: {
            CDP_NEW(cdpList, chdList);
            chdParentEx = &chdList->parentEx;
            break;
          }
          
          REQ_CIRC_BUFFER: {
            break;
          }
          
          REQ_ARRAY: {
            size_t capacity = va_arg(args, size_t);
            assert(capacity);
            CDP_NEW(cdpArray, chdArray);
            chdArray->capacity = capacity;
            chdArray->record = cdp_malloc0(capacity * sizeof(cdpRecord));
            chdParentEx = &chdArray->parentEx;
            break;
          }

          REQ_PACKED_LIST: {
            break;
          }
          
          REQ_RED_BLACK_T: {
            CDP_NEW(cdpRbTree, chdTree);
            chdParentEx = &chdTree->parentEx;
            break;
          }
        } while(0);
        
        // Link child book with its own child storage.
        if (style == CDP_REC_STYLE_DICTIONARY) {
            chdParentEx->compare = va_arg(args, cdpCmp);
            chdParentEx->context = va_arg(args, void*);
        }
        chdParentEx->book = child;
        child->recData.book.children = chdParentEx;
        break;
      }
      
      REGISTER: {
        bool   borrow = va_arg(args, bool);
        void*  data   = va_arg(args, void*);
        size_t size   = va_arg(args, size_t);
        assert(size);
        
        // Allocate storage for child register.
        if (borrow) {
            assert(priv && data);
            child->recData.reg.data.ptr = data;
            child->metadata.stoTech = CDP_REG_STO_BORROWED;
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
    } while(0);

    va_end(args);
        
    return child;
}




/*
   Reads register data from position and puts it on data buffer (atomically).
*/
bool cdp_record_register_read(cdpRecord* reg, size_t position, void** data, size_t* size) {
    assert(cdp_record_is_register(reg) && data && size);

    // Calculate the actual number of bytes that can be read
    size_t readableSize = reg->recData.reg.size - position;
    if (!*size || *size > readableSize)
        *size = readableSize;

    // Copy the data from the register to the provided buffer
    void* pointed = cdp_ptr_off(reg->recData.reg.ptr, position);
    if (*data) {
        memcpy(*data, pointed, *size);
    } else {
        assert(cdp_record_is_private(reg);
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
            cdpPath* newPath = cdp_dyn_malloc(cdpPath, cdpNameID, newCapacity);
            memcpy(&newPath->nameID[tempPath->capacity], tempPath->nameID, tempPath->capacity);
            newPath->length   = tempPath->capacity;
            newPath->capacity = newCapacity;
            cdp_ptr_overw(tempPath, newPath);
            *path = tempPath;
        }

        // Prepend the current record's nameID to the path
        tempPath[tempPath->capacity - tempPath->length - 1] = current->metadata.nameID;
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
    
    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpList* list = book->recData.book.children;
        assert(list);
        record = last? &list->tail.record: &list->head.record;
        break;
      }
        
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array && array->capacity > parentEx->chdCount);
        record = last?  &array->record[parentEx->chdCount - 1]:  array->record;
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
    } while(0);
    
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
    
    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpList* list = book->recData.book.children;
        for (cdpListNode* node = list->head;  node;  node = node->next) {
            if (node->record.metadata.nameID == nameID)
                return &node->record;
        }
        break;
      }
        
      CIRC_BUFFER:
        break;
    
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        if (cdp_record_is_dictionary(book) && array->parentEx.compare == record_compare_by_name) {
            cdpRecord key = {.metadata.nameID = nameID};
            return array_search(array, &key, NULL);
        } else {
            cdpRecord* record = array->record;
            for (size_t index = 0; index < parentEx->chdCount; index++, record++) {
                if (record->metadata.nameID == nameID)
                    return record;
            }
        }
      }
        
      PACKED_LIST: {
        break;
      }
        
      RED_BLACK_T: {
        assert(cdp_record_is_dictionary(book));
        break;
     }
    } while(0);
    break;
  }

    return NULL;
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

    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpList* list = book->recData.book.children;
        size_t n = 0;
        for (cdpListNode* node = list->head;  node;  node = node->next, n++) {
            if (n == index)
                return &node->record;
        }
        break;
      }
      
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array->capacity >= parentEx->chdCount);
        return &array->record[index];
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
    } while(0);

    return NULL;
}


/*
    Gets the record by its path from start record.
*/
cdpRecord* cdp_record_by_path(cdpRecord* start, const cdpPath* path) {
    assert(cdp_record_is_book_or_dic(start) && path && path->length);
    CDP_CK(start->recData.book.chdCount);
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
        book = parentEx->record;
    }
    CDP_CK(parentEx->chdCount);

    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpListNode* node = cdp_list_node_from_record(record);
        return node->prev? &node->prev.record: NULL;
      }
      
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array->capacity >= parentEx->chdCount);
        return (record > array->record)? record - 1: NULL;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
    } while(0);

    return NULL;
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
        book = parentEx->record;
    }
    CDP_CK(parentEx->chdCount);

    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpListNode* node = cdp_list_node_from_record(record);
        return node->next? &node->next.record: NULL;
      }
      
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array->capacity >= parentEx->chdCount);
        cdpRecord* last = &array->record[parentEx->chdCount - 1];
        return (record < last)? record + 1: NULL;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
    } while(0);

    return NULL;
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
    assert(book->recData.book.children);
    
    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_NON_PUSHABLE);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpList* list = book->recData.book.children;
        cdpListNode* node = childIdx?  (*(cdpListNode*)childIdx)->next:  list->head;
        while (node) {
            if (node->record.metadata.nameID == nameID) {
                *(cdpListNode*)childIdx = node;
                return &node->record;
            }
            node = node->next;
        }
        break;
      }
      
      CIRC_BUFFER: {
        for (size_t i = startIdx; i < book->data.book.childrenCount; i++) {
            if (children->record[i].metadata.nameID == nameID) {
                CDP_PTR_SEC_SET(childIdx, i + 1); // Prepare childIdx for the next search
                return &children->record[i]; // Found a match
            }
        }
        break;
      }
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        cdpRecord* record = array->record;
        for (size_t i = childIdx? *childIdx: 0;  i < parentEx->chdCount;  i++, record++){
            if (record->metadata.nameID == nameID)
                return record;
        }
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
cdpRecord* cdp_record_next_by_path(cdpRecord* start, cdpNameID* path, uintptr_t* childIdx) {
    assert(cdp_record_is_book_or_dic(start) && path && path->length);
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);    
    CDP_CK(parentEx->chdCount);
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
    assert(cdp_record_is_book_or_dic(book) && func);
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);    
    CDP_GO(!parentEx->chdCount);

    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpList* list = book->recData.book.children;
        cdpBookEntry entry = {.parent = book};
        cdpListNode* node = list->head, *next;
        while (node) {
            next = node->next;
            entry.record = &node->record;
            entry.next = next? &next->record: NULL;
            if (!func(&entry, 0, context))
                return false;
            entry.prev = entry.record;
            entry.index++;
            node = next;
        }
        break;
      }
        
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array && array->capacity >= parentEx->chdCount);
        cdpBookEntry entry = {.record = array->record, .parent = book, .next = (parentEx->chdCount > 1)? (array->record + 1): NULL};
        while (entry.index < parentEx->chdCount) {
            if (!func(&entry, 0, context))
                return false;
            entry.prev   = entry.record;
            entry.record = entry.next;
            entry.next++;
            entry.index++;
        }
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
    } while(0);
    
    return true;
}


/*
    Traverses each child branch and sub-branch of a book record, applying a function to each.
*/
#define CDP_MAX_FAST_STACK_DEPTH  16

bool cdp_record_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, cdpRecordTraverse endFunc, void* context) {
    assert(cdp_record_is_book_or_dic(book) && maxDepth && (func || endFunc);
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);    
    CDP_GO(!parentEx->chdCount);
    
    bool ok = true;
    cdpRecord* child;
    unsigned depth = 0;
    cdpBookEntry entry = {.record = cdp_record_top(book, false), .parent = book};
    
    // Non-recursive version of branch descent:
    cdpBookEntry* stack = (maxDepth > CDP_MAX_FAST_STACK_DEPTH)?  cdp_malloc(maxDepth * sizeof(cdpBookEntry)):  alloca(maxDepth * sizeof(cdpBookEntry));
    for (;;) {
        // Ascend to parent if no more siblings in branch.
        if (!entry.record) {
            if UNIP_RARELY(!depth)  break;    // endFunc is never called on root book.
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
        // Fast switch-case for child storage techniques:
        assert(entry.parent->metadata.stoTech < CDP_CHD_STO_COUNT);
        static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
        goto *chdStoTech[entry.parent->metadata.stoTech];
        do {
          LINKED_LIST: {
            cdpListNode* node = cdp_list_node_from_record(entry.record);
            cdpListNode* next = node->next;
            entry.next = next? &next->record: NULL;
            break;
          }
            
          CIRC_BUFFER:
            break;
            
          ARRAY: {
            cdpArray* array = entry.record->storage;
            assert(array && array->capacity >= array.parentEx.chdCount);
            entry.next = (entry.index < array.parentEx.chdCount)?  entry.record + 1:  NULL;
            break;
          }
          
          PACKED_LIST:
            break;
            
          RED_BLACK_T:
            break;
        } while(0);

        if (func) {
            ok = func(&entry, depth, context);
            if (!ok)  break;
        }
        
        // Descent to children if it's a book.
        if (cdp_record_is_book_or_dic(record) && ((child = cdp_record_top(record, false)))) {
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




/*
    Converts an unsorted book into a dictionary.
*/
void cdp_record_sort(cdpRecord* book, cdpCmp compare, void* context) {
    CDP_AB(cdp_record_is_dictionary(book));
    assert(cdp_record_is_book(book));
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);
    assert(!parentEx->compare);
    
    book->metadata.reStyle = CDP_REC_STYLE_DICTIONARY;
    parentEx->compare = compare? compare: record_compare_by_name;
    parentEx->context = context;    
    CDP_AB(parentEx->chdCount <= 1);
    
    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpList* list = book->recData.book.children;
        cdpListNode* prev = list->head, *next;
        cdpListNode* node = prev->next, *smal;
        while (node) {
            if (0 > compare(&node->record, &prev->record, context)) {
                // Unlink node.
                next = node->next;
                prev->next = next;
                if (next) next->prev = prev;
                
                // Look backwards for a smaller nameID.
                for (smal = prev->prev;  smal;  smal = smal->prev) {
                    if (0 <= compare(&node->record, &smal->record, context))
                        break;
                }
                if (smal) {
                    // Insert node after smaller.
                    node->prev = smal;
                    node->next = smal->next;
                    smal->next->prev = node;
                    smal->next = node;
                } else {
                    // Make node the new list head.
                    node->prev = NULL;
                    node->next = list->head;
                    list->head->prev = node;
                    list->head = node;
                }
                node = prev->next;
            } else {
                prev = node;
                node = node->next;
            }
        }
        break;
      }
        
      CIRC_BUFFER:
        break;
    
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(compare == rec_cmp_by_nameID);
        qsort(array->record, parentEx->chdCount, sizeof(cdpRecord), rec_cmp_by_nameID);
        array_update_children_parent_ptr(array->record, &array->record[parentEx->chdCount - 1]);
      }
        
      PACKED_LIST: {
        break;
      }
        
      RED_BLACK_T:
        break;

    } while(0);
    break;
  }
}


static bool record_delete_unlink(cdpBookEntry* entry, unsigned depth, void* p) {
    assert(entry && entry->record && entry->parent);

    // Fast switch-case for record styles:
    assert(entry.record->metadata.reStyle < CDP_REC_STYLE_COUNT);
    static void* const recordStyle[] = {&&BOOK, &&DICTIONARY, &&REGISTER, &&LINK};
    goto *recordStyle[entry.record->metadata.reStyle];
    do {
      BOOK: {
        break;
      }

      DICTIONARY: {
        break;
      }

      REGISTER: {
        if (entry.record->metadata.stoTech != 0)   // Data isn't borrowed
            cdp_free(entry.record->recData.reg.data.ptr);
        break;
      }
      
      LINK: {
        // Go to pointed record and unlink this from parent list.
        break;
      }
    } while(0);
        
    return true;
}

static bool record_delete_store(cdpBookEntry* entry, unsigned depth, void* p) {
    assert(entry && entry->record && entry->parent);
    
    // Fast switch-case for record styles:
    assert(entry.record->metadata.reStyle < CDP_REC_STYLE_COUNT);
    static void* const recordStyle[] = {&&BOOK, &&DICTIONARY, &&REGISTER, &&LINK};
    goto *recordStyle[entry.record->metadata.reStyle];
    do {
      BOOK:;
      DICTIONARY: {
        // Fast switch-case for child storage techniques:
        assert(entry.record->metadata.stoTech < CDP_CHD_STO_COUNT);
        static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
        goto *chdStoTech[entry.record->metadata.stoTech];
        do {
          LINKED_LIST: {
            cdpList* list = entry.record->recData.book.children;
            assert(list);
            cdp_free(list);
            break;
          }
            
          CIRC_BUFFER:
            break;
            
          ARRAY: {
            cdpArray* array = entry.record->recData.book.children;
            assert(array);
            cdp_free(array->record);
            cdp_free(array);
            break;
          }
          
          PACKED_LIST:
            break;
            
          RED_BLACK_T:
            break;
        } while(0);
        break;
      }
      
      REGISTER: {
        // This shouldn't happen.
        assert(cdp_record_is_book_or_dic(entry.record));
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
    assert(!cdp_record_has_shadows(record));
    cdpParentEx* parentEx = cdp_record_parent_ex(record);
    cdpRecord* book = parentEx->book;

    // Fast switch-case for record styles:
    assert(record->metadata.reStyle < CDP_REC_STYLE_COUNT);
    static void* const recordStyle[] = {&&BOOK, &&DICTIONARY, &&REGISTER, &&LINK};
    goto *recordStyle[record->metadata.reStyle];
    do {
      BOOK:;
      DICTIONARY: {
        cdp_record_deep_traverse(record, maxDepth, record_unlink, record_delete_store, NULL);
        break;
      }
      
      REGISTER: {
        record_delete_unlink(&(cdpBookEntry){.record=record, .parent=book}, 0, NULL);
        break;
      }
        
      LINK:
        break;
    } while(0);
       
    /* Delete from parent (re-organizing siblings) */
    // Fast switch-case for child storage techniques:
    assert(book->metadata.stoTech < CDP_CHD_STO_COUNT);
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER, &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};
    goto *chdStoTech[book->metadata.stoTech];
    do {
      LINKED_LIST: {
        cdpList* list = book->recData.book.children;
        assert(list && list->head);
        cdpListNode* node = cdp_list_node_from_record(record);
        cdpListNode* next = node->next;
        cdpListNode* prev = node->prev;
        // Unlink node.
        if (next) next->prev = prev;
        else      list->tail = prev;
        if (prev) prev->next = next;
        else      list->head = next;
        cdp_free(node);
        break;
      }
        
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        cdpArray* array = book->recData.book.children;
        assert(array && array->capacity >= parentEx->chdCount);
        cdpRecord* last = &array->record[parentEx->chdCount - 1];
        if (record < last)
            memmove(record, record + 1, cdp_ptr_dif(record, last));
        CDP_0(last);
        break;
      }
      
      PACKED_LIST:
        break;
        
      RED_BLACK_T:
        break;
    } while(0);
    
    parentEx->chdCount--;
        
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

