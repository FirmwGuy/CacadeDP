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


typedef struct _cdpPackedQNode  cdpPackedQNode;

struct _cdpPackedQNode {
    cdpPackedQNode* pNext;      // Pointer to the next node in the list.
    cdpPackedQNode* pPrev;      // Previous node.
    cdpRecord*      first;      // Points to the first record in buffer.
    cdpRecord*      last;       // The last record.
    //
    cdpRecord       record[];   // Fixed-size buffer for this node.
};

typedef struct {
    cdpChdStore     store;   // Parent info.
    //
    size_t          pSize;      // Pack size in bytes.
    cdpPackedQNode* pHead;      // Head of the buffer list.
    cdpPackedQNode* pTail;      // Tail of the buffer list.
} cdpPackedQ;




/*
    Packed Queue implementation
*/

static inline cdpPackedQ* packed_q_new(int capacity) {
    CDP_NEW(cdpPackedQ, pkdq);
    pkdq->pSize = capacity * sizeof(cdpRecord);
    return pkdq;
}


#define packed_q_del              cdp_free
#define packed_q_node_new(pkdq)   cdp_malloc0(sizeof(cdpPackedQNode) + (pkdq)->pSize)
#define packed_q_node_del         cdp_free


static inline cdpPackedQNode* packed_q_node_from_record(cdpPackedQ* pkdq, cdpRecord* record) {
    for (cdpPackedQNode* pNode = pkdq->pHead;  pNode;  pNode = pNode->pNext) {
        if (pNode->first <= record  &&  pNode->last >= record)
            return pNode;
    }
    return NULL;
}


static inline cdpRecord* packed_q_add(cdpPackedQ* pkdq, cdpRecord* parent, bool prepend, const cdpRecord* record) {
    assert(cdp_record_is_book(parent));

    cdpRecord* child;
    if (pkdq->store.chdCount) {
        if (prepend) {
            // Prepend
            if (pkdq->pHead->first > pkdq->pHead->record) {
                pkdq->pHead->first--;
            } else {
                cdpPackedQNode* pNode = packed_q_node_new(pkdq);
                pNode->first = pNode->last = cdp_ptr_off(pNode->record, pkdq->pSize - sizeof(cdpRecord));
                pNode->pNext = pkdq->pHead;
                pkdq->pHead->pPrev = pNode;
                pkdq->pHead = pNode;
            }
            child = pkdq->pHead->first;
        } else {
            // Append
            if (pkdq->pTail->last < (cdpRecord*)cdp_ptr_off(pkdq->pTail->record, pkdq->pSize - sizeof(cdpRecord))) {
                pkdq->pTail->last++;
            } else {
                cdpPackedQNode* pNode = packed_q_node_new(pkdq);
                pNode->last  = pNode->first = pNode->record;
                pNode->pPrev = pkdq->pTail;
                pkdq->pTail->pNext = pNode;
                pkdq->pTail = pNode;
            }
            child = pkdq->pTail->last;
        }
    } else {
        assert(!pkdq->pTail);
        cdpPackedQNode* pNode = packed_q_node_new(pkdq);
        pNode->last = pNode->first = pNode->record;
        pkdq->pTail = pkdq->pHead = pNode;
        child = pNode->last;
    }
    *child = *record;

    return child;
}


static inline cdpRecord* packed_q_first(cdpPackedQ* pkdq) {
    return pkdq->pHead->first;
}


static inline cdpRecord* packed_q_last(cdpPackedQ* pkdq) {
    return pkdq->pTail->last;
}


static inline cdpRecord* packed_q_find_by_name(cdpPackedQ* pkdq, cdpID id) {
    for (cdpPackedQNode* pNode = pkdq->pHead;  pNode;  pNode = pNode->pNext) {
        for (cdpRecord* record = pNode->first;  record <= pNode->last;  record++) {
            if (record->metadata.id == id)
                return record;
        }
    }
    return NULL;
}


static inline cdpRecord* packed_q_find_by_position(cdpPackedQ* pkdq, size_t position) {
    // ToDo: use from tail to head if index is closer to it.
    for (cdpPackedQNode* pNode = pkdq->pHead;  pNode;  pNode = pNode->pNext) {
        size_t chunk = cdp_ptr_idx(pNode->first, pNode->last, sizeof(cdpRecord)) + 1;
        if (chunk > position) {
            return &pNode->first[position];
        }
        position -= chunk;
    }
    return NULL;
}


static inline cdpRecord* packed_q_prev(cdpPackedQ* pkdq, cdpRecord* record) {
    cdpPackedQNode* pNode = packed_q_node_from_record(pkdq, record);
    assert(pNode);
    if (pNode->first == record)
        return NULL;
    return record - 1;
}


static inline cdpRecord* packed_q_next(cdpPackedQ* pkdq, cdpRecord* record) {
    cdpPackedQNode* pNode = packed_q_node_from_record(pkdq, record);
    assert(pNode);
    if (pNode->last == record)
        return NULL;
    return record + 1;
}


static inline cdpRecord* packed_q_next_by_name(cdpPackedQ* pkdq, cdpID id, cdpPackedQNode** prev) {
    for (cdpPackedQNode* pNode = prev? (*prev)->pNext: pkdq->pHead;  pNode;  pNode = pNode->pNext) {
        for (cdpRecord* record = pNode->first;  record <= pNode->last;  record++) {
            if (record->metadata.id == id)
                return record;
        }
    }
    return NULL;
}


static inline bool packed_q_traverse(cdpPackedQ* pkdq, cdpRecord* book, cdpRecordTraverse func, void* context, cdpBookEntry* entry) {
    entry->parent = book;
    cdpPackedQNode* pNode = pkdq->pHead;
    do {
        entry->next = pNode->first;
        do {
            if (entry->record) {
                if (!func(entry, 0, context))
                    return false;
                entry->position++;
                entry->prev = entry->record;
            }
            entry->record = entry->next;
            entry->next++;
        } while (entry->next <= pNode->last);
        pNode = pNode->pNext;
    } while (pNode);
    entry->next = NULL;
    return func(entry, 0, context);
}


static inline void packed_q_remove_record(cdpPackedQ* pkdq, cdpRecord* record) {
    if (record == pkdq->pHead->first) {
        pkdq->pHead->first++;
        if (pkdq->pHead->first <= pkdq->pHead->last) {
            CDP_0(record);
        } else {
            cdpPackedQNode* pNode = pkdq->pHead;
            pkdq->pHead = pkdq->pHead->pNext;
            if (pkdq->pHead)
                pkdq->pHead->pPrev = NULL;
            else
                pkdq->pTail = NULL;
            packed_q_node_del(pNode);       //ToDo: keep last node for re-use.
        }
    } else if (record == pkdq->pTail->last) {
        pkdq->pTail->last--;
        if (pkdq->pTail->last >= pkdq->pTail->first) {
            CDP_0(record);
        } else {
            cdpPackedQNode* pNode = pkdq->pTail;
            pkdq->pTail = pkdq->pTail->pPrev;
            if (pkdq->pTail)
                pkdq->pTail->pNext = NULL;
            else
                pkdq->pHead = NULL;
            packed_q_node_del(pNode);
        }
    } else {
        // Only popping (first or last) is allowed for circular buffers.
        assert(record == pkdq->pHead->first || record == pkdq->pTail->last);
    }
}


static inline void packed_q_del_all_children(cdpPackedQ* pkdq, unsigned maxDepth) {
    cdpPackedQNode* pNode = pkdq->pHead, *toDel;
    if (pNode) {
        do {
            for (cdpRecord* record = pNode->first;  record <= pNode->last;  record++) {
                cdp_record_finalize(record, maxDepth - 1);
            }
            toDel = pNode;
            pNode = pNode->pNext;
            packed_q_node_del(toDel);
        } while (pNode);
        pkdq->pHead = pkdq->pTail = NULL;
    }
}

