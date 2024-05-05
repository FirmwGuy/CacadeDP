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


typedef struct _cdpListNode     cdpListNode;

struct _cdpListNode {
    cdpListNode*  next;           // Previous node.
    cdpListNode*  prev;           // Next node.
    //
    cdpRecord     record;         // Child record.
};

typedef struct {
    cdpParentEx   parentEx;       // Parent info.
    //
    cdpListNode*  head;           // Head of the doubly linked list
    cdpListNode*  tail;           // Tail of the doubly linked list for quick append
} cdpList;




/*
    Double linked list implementation
*/

#define list_new()      cdp_new(cdpList)
#define list_del(list)  cdp_free(list)


static inline cdpListNode* list_node_from_record(cdpRecord* record) {
    return cdp_ptr_dif(record, offsetof(cdpListNode, record));
}


static inline cdpRecord* list_add(cdpList* list, cdpRecord* parent, bool push, cdpRecMeta* metadata) {
    CDP_NEW(cdpListNode, node);
    node->record.metadata = *metadata;
    
    if (list->parentEx.chdCount && cdp_record_is_dictionary(parent)) {
        // Sorted insert
        cdpListNode* next;
        for (next = list->head;  next;  next = next->next) {
            int cmp = record_compare_by_name(&node->record, &next->record);
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


static inline void list_del_all_children(cdpList* list, unsigned maxDepth) {
    cdpListNode* node = list->head, *toDel;
    while (node) {
        record_delete_storage(&node->record, maxDepth - 1);
        toDel = node;
        node = node->next;
        cdp_free(toDel);
    }
}

