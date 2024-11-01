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


typedef struct _cdpListNode     cdpListNode;

struct _cdpListNode {
    cdpListNode*  next;         // Previous node.
    cdpListNode*  prev;         // Next node.
    //
    cdpRecord     record;       // Child record.
};

typedef struct {
    cdpChdStore   store;        // Parent info.
    //
    cdpListNode*  head;         // Head of the doubly linked list
    cdpListNode*  tail;         // Tail of the doubly linked list for quick append
} cdpList;




/*
    Double linked list implementation
*/

#define list_new()      cdp_new(cdpList)
#define list_del        cdp_free


static inline cdpListNode* list_node_from_record(const cdpRecord* record) {
    return cdp_ptr_dif(record, offsetof(cdpListNode, record));
}




static inline void list_prepend_node(cdpList* list, cdpListNode* node) {
    node->next = list->head;
    if (list->head)
        list->head->prev = node;
    else
        list->tail = node;
    list->head = node;
}

static inline void list_append_node(cdpList* list, cdpListNode* node) {
    node->prev = list->tail;
    if (list->tail)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
}

static inline void list_insert_node_before_next(cdpList* list, cdpListNode* node, cdpListNode* next) {
    if (next->prev) {
        next->prev->next = node;
        node->prev = next->prev;
    } else {
        list->head = node;
    }
    next->prev = node;
    node->next = next;
}




static inline cdpRecord* list_insert(cdpList* list, cdpRecord* record, size_t position) {
    CDP_NEW(cdpListNode, node);
    cdp_record_transfer(record, &node->record);

    size_t n = 0;
    cdpListNode* next;
    for (next = list->head;  next;  next = next->next, n++) {
        if (n == position) {
            list_insert_node_before_next(list, node, next);
            break;
        }
    }
    if (!next)
        list_append_node(list, node);

    return &node->record;
}


static inline cdpRecord* list_named_insert(cdpList* list, cdpRecord* record) {
    CDP_NEW(cdpListNode, node);
    cdp_record_transfer(record, &node->record);

    cdpListNode* next;
    for (next = list->head;  next;  next = next->next) {
        int cmp = record_compare_by_name(&node->record, &next->record, NULL);
        if (0 > cmp) {
            list_insert_node_before_next(list, node, next);
            break;
        }
        assert(0 != cmp);   // Duplicates are not allowed.
    }
    if (!next)
        list_append_node(list, node);

    return &node->record;
}


static inline cdpRecord* list_sorted_insert(cdpList* list, cdpRecord* record, cdpCompare compare, void* context) {
    CDP_NEW(cdpListNode, node);
    cdp_record_transfer(record, &node->record);

    cdpListNode* next;
    for (next = list->head;  next;  next = next->next) {
        int cmp = compare(&node->record, &next->record, context);
        if (0 > cmp) {
            list_insert_node_before_next(list, node, next);
            break;
        }
        assert(0 != cmp);   // Duplicates are not allowed.
    }
    if (!next)
        list_append_node(list, node);

    return &node->record;
}


static inline cdpRecord* list_append(cdpList* list, cdpRecord* record, bool prepend) {
    CDP_NEW(cdpListNode, node);
    cdp_record_transfer(record, &node->record);

    if (list->store.chdCount) {
        if (prepend)
            list_prepend_node(list, node);
        else
            list_append_node(list, node);
    } else {
        list->head = list->tail = node;
    }

    return &node->record;
}




static inline cdpRecord* list_first(cdpList* list) {
   return &list->head->record;
}


static inline cdpRecord* list_last(cdpList* list) {
   return &list->tail->record;
}


static inline cdpRecord* list_find_by_name(cdpList* list, cdpID name) {
    for (cdpListNode* node = list->head;  node;  node = node->next) {
        if (node->record.metarecord.name == name)
            return &node->record;
    }
    return NULL;
}



static inline cdpRecord* list_find_by_key(cdpList* list, cdpRecord* key, cdpCompare compare, void* context) {
    for (cdpListNode* node = list->head;  node;  node = node->next) {
        if (0 == compare(key, &node->record, context))
            return &node->record;
    }
    return NULL;
}


static inline cdpRecord* list_find_by_position(cdpList* list, size_t position) {
    // ToDo: use from tail to head if index is closer to it.
    size_t n = 0;
    for (cdpListNode* node = list->head;  node;  node = node->next, n++) {
        if (n == position)
            return &node->record;
    }
    return NULL;
}


static inline cdpRecord* list_prev(const cdpRecord* record) {
    cdpListNode* node = list_node_from_record(record);
    return node->prev? &node->prev->record: NULL;
}


static inline cdpRecord* list_next(const cdpRecord* record) {
    cdpListNode* node = list_node_from_record(record);
    return node->next? &node->next->record: NULL;
}


static inline cdpRecord* list_next_by_name(cdpList* list, cdpID name, cdpListNode** prev) {
    cdpListNode* node = *prev?  (*prev)->next:  list->head;
    while (node) {
        if (node->record.metarecord.name == name) {
            *prev = node;
            return &node->record;
        }
        node = node->next;
    }
    *prev = NULL;
    return NULL;
}


static inline bool list_traverse(cdpList* list, cdpRecord* parent, cdpTraverse func, void* context, cdpBookEntry* entry) {
    entry->parent = parent;
    entry->depth  = 0;
    cdpListNode* node = list->head, *next;
    do {
        next = node->next;
        entry->record = &node->record;
        entry->next = next? &next->record: NULL;
        if (!func(entry, context))
            return false;
        entry->position++;
        entry->prev = entry->record;
        node = next;
    } while (node);
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

            // Look backwards for a smaller id.
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


static inline void list_take(cdpList* list, cdpRecord* target) {
    assert(list && list->tail);
    cdpListNode* node = list->tail;
    cdpListNode* prev = node->prev;

    // Unlink node.
    list->tail = prev;
    if (prev)
        prev->next = NULL;
    else
        list->head = NULL;

    cdp_record_transfer(&node->record, target);
    cdp_free(node);
}


static inline void list_pop(cdpList* list, cdpRecord* target) {
    assert(list && list->head);
    cdpListNode* node = list->head;
    cdpListNode* next = node->next;

    // Unlink node.
    list->head = next;
    if (next)
        next->prev = NULL;
    else
        list->tail = NULL;

    cdp_record_transfer(&node->record, target);
    cdp_free(node);
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


static inline void list_del_all_children(cdpList* list) {
    cdpListNode* node = list->head, *toDel;
    if (node) {
        do {
            cdp_record_finalize(&node->record);
            toDel = node;
            node = node->next;
            cdp_free(toDel);
        } while (node);
        list->head = list->tail = NULL;
    }
}

