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


/*
    Cascade Data Processing System - Layer 1
    ------------------------------------------

    CascadeDP Layer 1 is designed to represent and manage hierarchical 
    data structures in a distributed execution environment, similar in 
    flexibility to representing complex XML or JSON data models. It 
    facilitates the storage, navigation, and manipulation of records, 
    which can be mainly data registers (holding actual data values or 
    pointers to data) or books (acting as nodes in the hierarchical 
    structure with potential to have unique or repeatable fields).

    Key Components
    --------------

    * Record: The fundamental unit within the system, capable of acting 
    as either a book, a dictionary, a register or a link.
    
    * Book: A type of record that contains child records. Records are 
    kept in the order they are added. Duplicated keys are allowed.
    
    * Dictionary: A book that always keep its child records in sorted 
    order (defined by a compare function). Duplicates are not allowed.
    
    * Register: A type of record designed to store actual data, either 
    directly within the record if small enough or through a pointer to 
    larger data sets.
    
    * Link: A record that points to another record.
    
    * Metadata and Flags: Each record contains metadata, including 
    flags that specify the record's characteristics, an identifier 
    indicating the record's role or "name" within its parent, and a 
    "type" tag, enabling precise navigation and representation within 
    the hierarchy.

    The system is optimized for efficient storage and access, fitting 
    within processor cache lines to enhance performance. It supports 
    navigating from any record to the root of the database, 
    reconstructing paths within the data hierarchy based on field 
    identifiers in parent records.

    Goals
    -----

    * Flexibility: To accommodate diverse data models, from strictly 
    structured to loosely defined, allowing for dynamic schema changes.
    
    * Efficiency: To ensure data structures are compact, minimizing 
    memory usage and optimizing for cache access patterns.
    
    * Navigability: To allow easy traversal of the hierarchical data 
    structure, facilitating operations like queries, updates, and path 
    reconstruction.

    The system is adept at managing hierarchical data structures 
    through a variety of storage techniques, encapsulated within the 
    cdpVariantBook type. This flexibility allows the system to adapt to 
    different use cases and optimization requirements, particularly 
    focusing on cache efficiency and operation speed for insertions, 
    deletions, and lookups.

    Book and Storage Techniques
    ---------------------------
    
    * cdpVariantBook: Serves as a versatile container within the 
    system, holding child records through diverse storage strategies. 
    Each cdpVariantBook can adopt one of several child storage 
    mechanisms, determined by the stoTech indicator in its metadata. 
    This design enables tailored optimization based on specific needs, 
    such as operation frequency, data volume, and access patterns.

    * Storage Techniques: Each storage technique is selected to 
    optimize specific aspects of data management, addressing the 
    system's goals of flexibility, efficiency, and navigability.
    
      Array: Offers fast access and efficient cache utilization for 
      densely packed records. Ideal for situations where the number of 
      children is relatively static and operations are predominantly at 
      the tail end.
      
      Circular Buffer: Enhances the array model with efficient head and 
      tail operations, suitable for queues or streaming data scenarios.
      
      Doubly Linked List: Provides flexibility for frequent insertions 
      and deletions at arbitrary positions with minimal overhead per 
      operation.
      
      Packed List: Strikes a balance between the cache efficiency of 
      arrays and the flexibility of linked lists. It's optimized for 
      scenarios where both random access and dynamic modifications are 
      common.
      
      Red-Black Tree: Ensures balanced tree structure for ordered data, 
      offering logarithmic time complexity for insertions, deletions, 
      and lookups. Particularly useful for datasets requiring sorted 
      access.
    
    * Locking System: The lock mechanism is implemented by serializing 
    all IOs and keeping a list of currently locked book paths and a 
    list of locked register pointers. On each input/output the system 
    checks on the lists depending of the specified record.
    
    * Private Records: Records may be private, in which case no locking 
    is necessary since they are never made public.

*/


#include "cdp_record.h"
#include <stdarg.h>




/*
    General utilities
*/

#define cdpFunc     void*

static inline int record_compare_by_name(const cdpRecord* restrict key, const cdpRecord* restrict record) {
    return key->metadata.nameID - record->metadata.nameID;
}

static inline int record_compare_by_name_s(const cdpRecord* restrict key, const cdpRecord* restrict record, void* unused) {
    return record_compare_by_name(key, record);
}


#define STORAGE_TECH_BEGIN(stoTech)                                    \
    assert((stoTech) < CDP_STO_CHD_COUNT);                             \
    static void* const chdStoTech[] = {&&LINKED_LIST, &&CIRC_BUFFER,   \
        &&ARRAY, &&PACKED_LIST, &&RED_BLACK_T};                        \
    goto *chdStoTech[stoTech];                                         \
    do


#define RECORD_STYLE_BEGIN(reStyle)                                    \
    assert((reStyle) < CDP_REC_STYLE_COUNT);                           \
    static void* const recordStyle[] = {&&BOOK, &&DICTIONARY,          \
        &&REGISTER, &&LINK};                                           \
    goto *recordStyle[reStyle];                                        \
    do

#define RECORD_LABEL_END                                               \
    while (0)




/*
   Double linked list implementation
*/


#define list_new()      cdp_new(cdpList)
#define list_del(list)  cdp_free(list)


static inline cdpListNode* list_node_from_record(cdpRecord* record) {
    return cdp_ptr_dif(record, offsetof(cdpListNode, record));
}


static inline void list_remove_record(cdpList* list, cdpRecord* record) {
    assert(list && list->head);
    cdpListNode* node = list_node_from_record(record);
    cdpListNode* next = node->next;
    cdpListNode* prev = node->prev;
    
    // Unlink node.
    if (next) next->prev = prev;
    else      list->tail = prev;
    if (prev) prev->next = next;
    else      list->head = next;
    
    cdp_free(node);
}


static inline cdpRecord* list_add(cdpList* list, cdpRecord* parent, bool push, cdpRecord* metadata) {
    CDP_NEW(cdpListNode, node);
    if (list->parentEx.chdCount && cdp_record_is_dictionary(parent)) {
        // Sorted insert
        assert(!list->parentEx.compare);     // Only sort by nameID is allowed here.
        cdpListNode* next;
        for (next = list->head;  next;  next = next->next) {
            int cmp = record_compare_by_name(metadata, &next->record);
            if (0 > cmp) {
                // Insert node before next.
                if (next->prev) {
                    next->prev->next = node;
                    node->prev = next->prev;
                } else {
                    list->head = node;
                }
                next->prev = node;
                node->next = next;
                break;
            }
            assert(0 != cmp);   // Duplicates are not allowed in dictionaries.
        }
        if (!next) {
            // Make node the new list tail.
            list->tail->next = node;
            node->prev = list->tail;
            list->tail = node;
        }
    } else {
        if (push) {
            // Prepend node
            node->next = list->head;
            if (list->head)
                list->head->prev = node;
            else
                list->tail = node;
            list->head = node;
        } else {
            // Append node
            node->prev = list->tail;
            if (list->tail)
                list->tail->next = node;
            else
                list->head = node;
            list->tail = node;
        }
    }
    return &node->record;
}


static inline cdpRecord* list_top(cdpList* list, bool last) {
   return last?  &list->tail->record:  &list->head->record;
}


static inline cdpRecord* list_find_by_name(cdpList* list, cdpNameID nameID) {
    for (cdpListNode* node = list->head;  node;  node = node->next) {
        if (node->record.metadata.nameID == nameID)
            return &node->record;
    }
    return NULL;
}


static inline cdpRecord* list_find_by_index(cdpList* list, size_t index) {
    size_t n = 0;
    for (cdpListNode* node = list->head;  node;  node = node->next, n++) {
        if (n == index)
            return &node->record;
    }
    return NULL;
}


static inline cdpRecord* list_prev(cdpRecord* record) {
    cdpListNode* node = list_node_from_record(record);
    return node->prev? &node->prev->record: NULL;
}


static inline cdpRecord* list_next(cdpRecord* record) {
    cdpListNode* node = list_node_from_record(record);
    return node->next? &node->next->record: NULL;
}


static inline cdpRecord* list_next_by_name(cdpList* list, cdpNameID nameID, cdpListNode** prev) {
    cdpListNode* node = *prev?  (*prev)->next:  list->head;
    while (node) {
        if (node->record.metadata.nameID == nameID) {
            *prev = node;
            return &node->record;
        }
        node = node->next;
    }
    *prev = NULL;
    return NULL;
}


static inline bool list_traverse(cdpList* list, cdpRecord* book, cdpRecordTraverse func, void* context) {
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
    return true;
}


static inline void list_sort(cdpList* list, cdpCompare compare, void* context) {
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
}




/*
   Dynamic array implementation
*/


static inline cdpArray* array_new(size_t capacity) {
    assert(capacity);
    CDP_NEW(cdpArray, array);
    array->capacity = capacity;
    array->record = cdp_malloc0(capacity * sizeof(cdpRecord));
    return array;
}


static inline void array_del(cdpArray* array) {
    assert(array);
    cdp_free(array->record);
    cdp_free(array);
}


static inline cdpRecord* array_search(cdpArray* array, void* key, size_t* index) {
    size_t imax = cdp_ptr_has_val(index)? *index - 1: array->parentEx.chdCount - 1;
    size_t imin = 0, i;
    cdpRecord* record;
    do {
        i = (imax + imin) >> 1;  // (max + min) / 2
        record = &array->record[i];
        int res = array->parentEx.compare(key, record, array->parentEx.context);
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
        if (cdp_record_is_book_or_dic(record)) {
            cdpParentEx* parentEx = record->recData.book.children;
            parentEx->book = record;    // Update (grand) child link to the (child) parent.
        }
    }
}


static inline cdpRecord* array_add(cdpArray* array, cdpRecord* parent, bool push, cdpRecord* metadata) {
    if (array->capacity == array->parentEx.chdCount) {
        // Increase array space if necessary
        assert(array->capacity);
        array->capacity *= 2;
        CDP_REALLOC(array->record, array->capacity * sizeof(cdpRecord));
        memset(&array->record[array->parentEx.chdCount], 0, array->parentEx.chdCount * sizeof(cdpRecord));
        array_update_children_parent_ptr(array->record, &array->record[array->parentEx.chdCount - 1]);
    }
    cdpRecord* child;
    
    if (array->parentEx.chdCount) {
        if (cdp_record_is_dictionary(parent)) {
            // Sorted insert
            assert(array->parentEx.compare);
            size_t index = 0;
            cdpRecord* prev = array_search(array, &metadata, &index);
            if (prev) {
                // FixMe: delete children.
                assert(prev);
            }
            child = &array->record[index];
            memmove(child + 1, child, array->parentEx.chdCount * sizeof(cdpRecord)); 
            *child = *metadata;
            array_update_children_parent_ptr(child + 1, &array->record[array->parentEx.chdCount - 1]);
        } else if (push) {
            // Prepend child
            child = array->record;
            memmove(child + 1, child, array->parentEx.chdCount * sizeof(cdpRecord)); 
            *child = *metadata;
            array_update_children_parent_ptr(child + 1, &array->record[array->parentEx.chdCount - 1]);
        } else {
            // Append child
            child = &array->record[array->parentEx.chdCount];
            *child = *metadata;
        }
    } else {
            child = array->record;
            *child = *metadata;
    }
    return child;
}


static inline cdpRecord* array_top(cdpArray* array, bool last) {
    assert(array->capacity > array->parentEx.chdCount);
    return last?  &array->record[array->parentEx.chdCount - 1]:  array->record;
}


static inline cdpRecord* array_find_by_name(cdpArray* array, cdpNameID nameID, cdpRecord* book) {
    if (cdp_record_is_dictionary(book) && !array->parentEx.compare) {
        cdpRecord key = {.metadata.nameID = nameID};
        return bsearch(&key, array->record, array->parentEx.chdCount, sizeof(cdpRecord), (cdpFunc) record_compare_by_name);
    } else {
        cdpRecord* record = array->record;
        for (size_t i = 0; i < array->parentEx.chdCount; i++, record++) {
            if (record->metadata.nameID == nameID)
                return record;
        }
    }
    return NULL;
}


static inline cdpRecord* array_find_by_index(cdpArray* array, size_t index) {
    assert(index < array->parentEx.chdCount);
    return &array->record[index];
}


static inline cdpRecord* array_prev(cdpArray* array, cdpRecord* record) {
    return (record > array->record)? record - 1: NULL;
}


static inline cdpRecord* array_next(cdpArray* array, cdpRecord* record) {
    cdpRecord* last = &array->record[array->parentEx.chdCount - 1];
    return (record < last)? record + 1: NULL;
}


static inline cdpRecord* array_next_by_name(cdpArray* array, cdpNameID nameID, uintptr_t* prev) {
    cdpRecord* record = array->record;
    for (size_t i = prev? (*prev + 1): 0;  i < array->parentEx.chdCount;  i++, record++){
        if (record->metadata.nameID == nameID)
            return record;
    }
    return NULL;
}

static inline bool array_traverse(cdpArray* array, cdpRecord* book, cdpRecordTraverse func, void* context){
    assert(array && array->capacity >= array->parentEx.chdCount);
    cdpBookEntry entry = {.record = array->record, .parent = book, .next = (array->parentEx.chdCount > 1)? (array->record + 1): NULL};
    while (entry.index < array->parentEx.chdCount) {
        if (!func(&entry, 0, context))
            return false;
        entry.prev   = entry.record;
        entry.record = entry.next;
        entry.next++;
        entry.index++;
    }
    return true;
}


static inline void array_sort(cdpArray* array, cdpCompare compare, void* context) {
    if (!compare) {
        qsort(array->record, array->parentEx.chdCount, sizeof(cdpRecord), (cdpFunc) record_compare_by_name);
    } else {
        qsort_s(array->record, array->parentEx.chdCount, sizeof(cdpRecord), (cdpFunc) compare, context);
    }
    array_update_children_parent_ptr(array->record, &array->record[array->parentEx.chdCount - 1]);
}


static inline void array_remove_record(cdpArray* array, cdpRecord* record) {
    assert(array && array->capacity >= array->parentEx.chdCount);
    cdpRecord* last = &array->record[array->parentEx.chdCount - 1];
    if (record < last)
        memmove(record, record + 1, (size_t) cdp_ptr_dif(record, last));
    CDP_0(last);
}





/*
    Red-black tree implementation
*/

#define rb_tree_new()      cdp_new(cdpRbTree)
#define rb_tree_del(tree)  cdp_free(tree)


static inline cdpRbTreeNode* rb_tree_node_from_record(cdpRecord* record) {
    return cdp_ptr_dif(record, offsetof(cdpRbTreeNode, record));
}


static inline void rb_tree_rotate_left(cdpRbTree* tree, cdpRbTreeNode* x) {
    cdpRbTreeNode* y = x->right;
    x->right = y->left;
    if (y->left)
        y->left->tParent = x;
    y->tParent = x->tParent;
    if (!x->tParent) {
        tree->root = y;
    } else if (x == x->tParent->left) {
        x->tParent->left = y;
    } else {
        x->tParent->right = y;
    }
    y->left = x;
    x->tParent = y;
}

static inline void rb_tree_rotate_right(cdpRbTree* tree, cdpRbTreeNode* x) {
    cdpRbTreeNode* y = x->left;
    x->left = y->right;
    if (y->right)
        y->right->tParent = x;
    y->tParent = x->tParent;
    if (!x->tParent) {
        tree->root = y;
    } else if (x == x->tParent->right) {
        x->tParent->right = y;
    } else {
        x->tParent->left = y;
    }
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

static inline cdpRecord* rb_tree_add(cdpRbTree* tree, cdpRecord* parent, bool push, cdpRecord* metadata) {
    assert(cdp_record_is_dictionary(parent));
    CDP_NEW(cdpRbTreeNode, tnode);
    tnode->isRed = true;
    
    cdpCompare compare;
    void* context;
    if (tree->parentEx.compare) {
        compare = tree->parentEx.compare;
        context = tree->parentEx.context;
    } else {
        compare = record_compare_by_name_s;
        context = NULL;
    }
    cdpRecord* child = &tnode->record;
    *child = *metadata;
    
    if (tree->root) {
        cdpRbTreeNode* x = tree->root;
        cdpRbTreeNode* y;
        do {
            y = x;
            int cmp = compare(metadata, &x->record, context);
            if CDP_RARELY(0 == cmp) {
                // FixMe: delete children.
                assert(0 == cmp);
            } else if (0 > cmp) {
                x = x->left;
            } else {
                x = x->right;
            }
        } while (x);
        tnode->tParent = y;
        if (0 > compare(metadata, &y->record, context)) {
            y->left = tnode;
        } else {
            y->right = tnode;
        }
        rb_tree_fix_insert(tree, tnode);
    } else {
        tree->root = tnode;
    }
    
    return child;
}


static inline cdpRecord* rb_tree_top(cdpRbTree* tree, bool last) {
    cdpRbTreeNode* tnode = tree->root;
    if (last) {
        while (tnode->right)  tnode = tnode->right;
    } else {
        while (tnode->left)   tnode = tnode->left;        
    }
    return &tnode->record;
}


static inline bool rb_tree_traverse(cdpRbTree* tree, cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, void* context) {
  cdpRbTreeNode* tnode = tree->root, *tnodePrev = NULL;
  cdpRbTreeNode* stack[maxDepth];
  unsigned top = -1;  // Stack index initialized to empty.
  cdpBookEntry entry = {.parent = book};
  do {
      if (tnode) {
          assert(top < (maxDepth - 1));
          stack[++top] = tnode;
          tnode = tnode->left;
      } else {
          tnode = stack[top--];
          if CDP_EXPECT(tnodePrev != NULL) {
              entry.next = &tnode->record;
              entry.record = &tnodePrev->record;
              if (!func(&entry, 0, context))
                  return false;
              entry.prev = entry.record;
              entry.index++;
          }
          tnodePrev = tnode;
          tnode = tnode->right;
      }
  } while (top != -1 || tnode);
  
  entry.next = NULL;
  entry.record = &tnodePrev->record;
  return func(&entry, 0, context);
}


struct RbFindByName {cdpNameID nameID; cdpRecord* found;};

static inline int rb_traverse_func_break_at_name(cdpBookEntry* entry, unsigned u, struct RbFindByName* fbn) {
    if (entry->record->metadata.nameID == fbn->nameID) {
        fbn->found = entry->record;
        return false;
    }
    return true;
}

static inline cdpRecord* rb_tree_find_by_name(cdpRbTree* tree, cdpNameID nameID, cdpRecord* book) {
    assert(cdp_record_is_dictionary(book));
    if (!tree->parentEx.compare) {
        cdpRbTreeNode* tnode = tree->root;
        do {
            if (tnode->record.metadata.nameID < nameID) {
                tnode = tnode->left;
            } else if (tnode->record.metadata.nameID > nameID) {
                tnode = tnode->right;
            } else {
                return &tnode->record;
            }
        } while (tnode);
    } else {
        struct RbFindByName fbn = {.nameID = nameID};
        rb_tree_traverse(tree, book, cdp_bitson(tree->parentEx.chdCount) + 2, (cdpFunc) rb_traverse_func_break_at_name, &fbn);
        return fbn.found;
    }
    return NULL;
}


struct RbBreakAtIndex {size_t index; cdpRecord* record;};

static inline int rb_traverse_func_break_at_index(cdpBookEntry* entry, unsigned u, struct RbBreakAtIndex* bai) {
    if (entry->index == bai->index) {
        bai->record = entry->record;
        return false;
    }
    return true;
}

static inline cdpRecord* rb_tree_find_by_index(cdpRbTree* tree, size_t index, cdpRecord* book) {
    struct RbBreakAtIndex bai = {.index = index};
    if (rb_tree_traverse(tree, book, cdp_bitson(tree->parentEx.chdCount) + 2, (void*) rb_traverse_func_break_at_index, &bai))
        return bai.record;
    return NULL;
}


static inline cdpRecord* rb_tree_prev(cdpRecord* record) {
    cdpRbTreeNode* tnode = rb_tree_node_from_record(record);
    if (tnode->left) {
        tnode = tnode->left;
        while (tnode->right) tnode = tnode->right;
        return &tnode->record;
    }
    cdpRbTreeNode* tParent = tnode->tParent;
    while (tParent && tnode == tParent->left) {
        tnode = tParent;
        tParent = tParent->tParent;
    }
    return tParent? &tParent->record: NULL;
}


static inline cdpRecord* rb_tree_next(cdpRecord* record) {
    cdpRbTreeNode* tnode = rb_tree_node_from_record(record);
    if (tnode->right) {
        tnode = tnode->right;
        while (tnode->left) tnode = tnode->left;
        return &tnode->record;
    }
    cdpRbTreeNode* tParent = tnode->tParent;
    while (tParent && tnode == tParent->right) {
        tnode = tParent;
        tParent = tParent->tParent;
    }
    return tParent? &tParent->record: NULL;
}


static inline void rb_tree_transplant(cdpRbTree* tree, cdpRbTreeNode* u, cdpRbTreeNode* v) {
    if (!u->tParent) {
        tree->root = v;
    } else if (u == u->tParent->left) {
        u->tParent->left = v;
    } else {
        u->tParent->right = v;
    }
    if (v)
        v->tParent = u->tParent;
}

static inline void rb_tree_fixremove_node(cdpRbTree* tree, cdpRbTreeNode* x) {
    while (x != tree->root && !x->isRed) {
        if (x == x->tParent->left) {
            cdpRbTreeNode* w = x->tParent->right;
            if (!w) break;
            if (w->isRed) {
                w->isRed = false;
                x->tParent->isRed = true;
                rb_tree_rotate_left(tree, x->tParent);
                w = x->tParent->right;
            }
            if (!w || !w->left || !w->right) break;
            if (!w->left->isRed && !w->right->isRed) {
                w->isRed = true;
                x = x->tParent;
            } else {
                if (!w->right->isRed) {
                    w->left->isRed = false;
                    w->isRed = true;
                    rb_tree_rotate_right(tree, w);
                    w = x->tParent->right;
                }
                w->isRed = x->tParent->isRed;
                x->tParent->isRed = false;
                w->right->isRed = false;
                rb_tree_rotate_left(tree, x->tParent);
                x = tree->root;
            }
        } else {
            cdpRbTreeNode* w = x->tParent->left;
            if (!w) break;
            if (w->isRed) {
                w->isRed = false;
                x->tParent->isRed = true;
                rb_tree_rotate_right(tree, x->tParent);
                w = x->tParent->left;
            }
            if (!w || !w->right || !w->left) break;
            if (!w->right->isRed && !w->left->isRed) {
                w->isRed = true;
                x = x->tParent;
            } else {
                if (!w->left->isRed) {
                    w->right->isRed = false;
                    w->isRed = true;
                    rb_tree_rotate_left(tree, w);
                    w = x->tParent->left;
                }
                w->isRed = x->tParent->isRed;
                x->tParent->isRed = false;
                w->left->isRed = false;
                rb_tree_rotate_right(tree, x->tParent);
                x = tree->root;
            }
        }
    }
    x->isRed = false;
}

static inline void rb_tree_remove_record(cdpRbTree* tree, cdpRecord* record) {
    cdpRbTreeNode* tnode = rb_tree_node_from_record(record);
    cdpRbTreeNode* y = tnode, *x;
    bool wasRed = tnode->isRed;

    if (!tnode->left) {
        x = tnode->right;
        rb_tree_transplant(tree, tnode, x);
    } else if (!tnode->right) {
        x = tnode->left;
        rb_tree_transplant(tree, tnode, x);
    } else {
        for (y = tnode->right;  y->left;  y = y->left);
        wasRed = y->isRed;
        x = y->right;

        if (y->tParent == tnode) {
            if (x)  x->tParent = y;
        } else {
            rb_tree_transplant(tree, y, x);
            y->right = tnode->right;
            y->right->tParent = y;
        }
        rb_tree_transplant(tree, tnode, y);
        y->left = tnode->left;
        y->left->tParent = y;
        y->isRed = tnode->isRed;
    }
    if (x && !wasRed)
        rb_tree_fixremove_node(tree, x);

    cdp_free(tnode);
}      




/* 
    Creates a new record of the specified style on parent book.
*/
cdpRecord* cdp_record_create(cdpRecord* parent, unsigned style, cdpNameID nameID, uint32_t typeID, bool push, bool priv, ...) {
    assert(cdp_record_is_book_or_dic(parent) && nameID && typeID);
    cdpParentEx* parentEx = CDP_PARENTEX(parent->recData.book.children);
    cdpRecord metadata = {.metadata = {.proFlag = priv? CDP_FLAG_PRIVATE: 0, .reStyle = style, .typeID = typeID, .nameID = nameID}};
    cdpRecord* child;
    va_list args;
    va_start(args, priv);
    
    // Add new record to parent book.
    STORAGE_TECH_BEGIN(parent->metadata.stoTech) {
      LINKED_LIST: {
        child = list_add(parent->recData.book.children, parent, push, &metadata);
        break;
      }
        
      CIRC_BUFFER: {
        break;
      }
        
      ARRAY: {
        child = array_add(parent->recData.book.children, parent, push, &metadata);
        break;
      }
       
      PACKED_LIST: {
        break;
      }
        
      RED_BLACK_T:
      {
        child = rb_tree_add(parent->recData.book.children, parent, push, &metadata);
        break;
      }
    } RECORD_LABEL_END;
    
    // Update child.
    child->storage = parentEx;
    
    // Update parent.
    parentEx->chdCount++;
    
    RECORD_STYLE_BEGIN(style) {
      BOOK:;
      DICTIONARY: {
        unsigned storage = va_arg(args, unsigned);
        cdpParentEx* chdParentEx;

        // Fast switch-case for (child's) child storage techniques:
        assert(storage < CDP_STO_CHD_COUNT);
        static void* const reqStoTech[] = {&&REQ_LINKED_LIST, &&REQ_CIRC_BUFFER, &&REQ_ARRAY, &&REQ_PACKED_LIST, &&REQ_RED_BLACK_T};
        goto *reqStoTech[storage];
        do {
          REQ_LINKED_LIST: {
            cdpList* chdList = list_new();
            chdParentEx = &chdList->parentEx;
            break;
          }
          
          REQ_CIRC_BUFFER: {
            break;
          }
          
          REQ_ARRAY: {
            size_t capacity = va_arg(args, size_t);
            cdpArray* chdArray = array_new(capacity);
            chdParentEx = &chdArray->parentEx;
            break;
          }

          REQ_PACKED_LIST: {
            break;
          }
          
          REQ_RED_BLACK_T: {
            cdpRbTree* chdTree = rb_tree_new();
            chdParentEx = &chdTree->parentEx;
            break;
          }
        } while(0);
        
        // Link child book with its own child storage.
        if (style == CDP_REC_STYLE_DICTIONARY) {
            chdParentEx->compare = va_arg(args, cdpCompare);
            chdParentEx->context = va_arg(args, void*);
        }
        chdParentEx->book = child;
        child->recData.book.children = chdParentEx;
        break;
      }
      
      REGISTER: {
        bool   borrow = va_arg(args, int);
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
    } RECORD_LABEL_END;

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
    void* pointed = cdp_ptr_off(reg->recData.reg.data.ptr, position);
    if (*data) {
        memcpy(*data, pointed, *size);
    } else {
        assert(cdp_record_is_private(reg));
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
        CDP_REALLOC(reg->recData.reg.data.ptr, newSize);
        reg->recData.reg.size = newSize;
    }

    // Copy the provided data into the register's buffer at the specified position
    memcpy(cdp_ptr_off(reg->recData.reg.data.ptr, position), data, size);

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
    
    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_top(book->recData.book.children, last);
        break;
      }
        
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        record = array_top(book->recData.book.children, last);
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T: {
        record = rb_tree_top(book->recData.book.children, last);
        break;
      }
    } RECORD_LABEL_END;
    
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
    
    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_find_by_name(book->recData.book.children, nameID);
        break;
      }
        
      CIRC_BUFFER:
        break;
    
      ARRAY: {
        record = array_find_by_name(book->recData.book.children, nameID, book);
        break;
      }
        
      PACKED_LIST:
        break;
        
      RED_BLACK_T: {
        record = rb_tree_find_by_name(book->recData.book.children, nameID, book);
        break;
     }
    } RECORD_LABEL_END;

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

    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_find_by_index(book->recData.book.children, index);
        break;
      }
      
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        record = array_find_by_index(book->recData.book.children, index);
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T: {
        record = rb_tree_find_by_index(book->recData.book.children, index, book);
        break;
      }
    } RECORD_LABEL_END;

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

    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        prev = list_prev(record);
        break;
      }
      
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        prev = array_prev(book->recData.book.children, record);
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T: {
        prev = rb_tree_prev(record);
        break;
      }
    } RECORD_LABEL_END;

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

    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        next = list_next(record);
        break;
      }
      
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        next = array_next(book->recData.book.children, record);
        break;
      }
        
      PACKED_LIST:
        break;
        
      RED_BLACK_T: {
        next = rb_tree_next(record);
        break;
      }

    } RECORD_LABEL_END;

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
    
    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        record = list_next_by_name(book->recData.book.children, nameID, (cdpListNode**)childIdx);
        break;
      }
      
      CIRC_BUFFER: {
        break;
      }
        
      ARRAY: {
        record = array_next_by_name(book->recData.book.children, nameID, childIdx);
        break;
      }
       
      PACKED_LIST:
        break;
      
      RED_BLACK_T: {    // Unused.
        break;
      }
    } RECORD_LABEL_END;

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

    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        done = list_traverse(book->recData.book.children, book, func, context);
        break;
      }
        
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        done = array_traverse(book->recData.book.children, book, func, context);
        break;
      }
       
      PACKED_LIST:
        break;
        
      RED_BLACK_T: {
        done = rb_tree_traverse(book->recData.book.children, book, cdp_bitson(parentEx->chdCount) + 2, func, context);
        break;
      }
    } RECORD_LABEL_END;
    
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
        STORAGE_TECH_BEGIN(entry.parent->metadata.stoTech) {
          LINKED_LIST: {
            entry.next = list_next(entry.record);
            break;
          }
            
          CIRC_BUFFER:
            break;
            
          ARRAY: {
            entry.next = array_next(entry.record->storage, entry.record);
            break;
          }
          
          PACKED_LIST:
            break;
            
          RED_BLACK_T: {
            entry.next = rb_tree_next(entry.record);
            break;
          }
        } RECORD_LABEL_END;

        if (func) {
            ok = func(&entry, depth, context);
            if (!ok)  break;
        }
        
        // Descent to children if it's a book.
        if (cdp_record_is_book_or_dic(entry.record)
        && ((child = cdp_record_top(entry.record, false)))) {
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
void cdp_record_sort(cdpRecord* book, cdpCompare compare, void* context) {
    CDP_AB(cdp_record_is_dictionary(book));
    assert(cdp_record_is_book(book));
    cdpParentEx* parentEx = CDP_PARENTEX(book->recData.book.children);
    assert(!parentEx->compare);
    
    book->metadata.reStyle = CDP_REC_STYLE_DICTIONARY;
    parentEx->compare = compare? compare: record_compare_by_name_s;
    parentEx->context = context;    
    CDP_AB(parentEx->chdCount <= 1);
    
    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        list_sort(book->recData.book.children, compare, context);
        break;
      }
        
      CIRC_BUFFER:
        break;
    
      ARRAY: {
        array_sort(book->recData.book.children, compare, context);
        break;
      }
        
      PACKED_LIST: {
        break;
      }
        
      RED_BLACK_T: {    // Unused.
        break;
      }
    } RECORD_LABEL_END;
}


static bool record_delete_unlink(cdpBookEntry* entry, unsigned depth, void* p) {
    assert(entry && entry->record && entry->parent);

    RECORD_STYLE_BEGIN(entry->record->metadata.reStyle) {
      BOOK: {
        break;
      }

      DICTIONARY: {
        break;
      }

      REGISTER: {
        if (entry->record->metadata.stoTech != 0)   // Data isn't borrowed
            cdp_free(entry->record->recData.reg.data.ptr);
        break;
      }
      
      LINK: {
        // Go to pointed record and unlink this from parent list.
        break;
      }
    } RECORD_LABEL_END;
        
    return true;
}

static bool record_delete_store(cdpBookEntry* entry, unsigned depth, void* p) {
    assert(entry && entry->record && entry->parent);
    
    RECORD_STYLE_BEGIN(entry->record->metadata.reStyle) {
      BOOK:;
      DICTIONARY: {
        STORAGE_TECH_BEGIN(entry->record->metadata.stoTech) {
          LINKED_LIST: {
            list_del(entry->record->recData.book.children);
            break;
          }
            
          CIRC_BUFFER:
            break;
            
          ARRAY: {
            array_del(entry->record->recData.book.children);
            break;
          }
          
          PACKED_LIST:
            break;
            
          RED_BLACK_T: {
            rb_tree_del(entry->record->recData.book.children);
            break;
          }
        } RECORD_LABEL_END;
        break;
      }
      
      REGISTER: {
        // This shouldn't happen.
        assert(cdp_record_is_book_or_dic(entry->record));
        break;
      }
        
      LINK:
        break;
    } RECORD_LABEL_END;
            
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

    // Delete children first.
    RECORD_STYLE_BEGIN(record->metadata.reStyle) {
      BOOK:;
      DICTIONARY: {
        cdp_record_deep_traverse(record, maxDepth, record_delete_unlink, record_delete_store, NULL);
        break;
      }
      
      REGISTER: {
        record_delete_unlink(&(cdpBookEntry){.record=record, .parent=book}, 0, NULL);
        break;
      }
        
      LINK:
        break;
    } RECORD_LABEL_END;
       
    // Delete this record from its parent (re-organizing siblings).
    STORAGE_TECH_BEGIN(book->metadata.stoTech) {
      LINKED_LIST: {
        list_remove_record(book->recData.book.children, record);
        break;
      }
        
      CIRC_BUFFER:
        break;
        
      ARRAY: {
        array_remove_record(book->recData.book.children, record);
        break;
      }
      
      PACKED_LIST:
        break;
        
      RED_BLACK_T: {
        rb_tree_remove_record(book->recData.book.children, record);
        break;
      }
    } RECORD_LABEL_END;
    
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

