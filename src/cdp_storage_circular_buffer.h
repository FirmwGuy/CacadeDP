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


typedef struct _cdpCircBufNode  cdpCircBufNode;

struct _cdpCircBufNode {
    cdpCircBufNode* pNext;      // Pointer to the next node in the list.
    cdpCircBufNode* pPrev;      // Previous node.
    cdpRecord*      first;      // Points to the first record in buffer.
    cdpRecord*      last;       // The last record.
    //
    cdpRecord       record[];   // Fixed-size buffer for this node.
};

typedef struct {
    cdpParentEx     parentEx;   // Parent info.
    //                          
    size_t          bufSize;    // Buffer size in bytes.
    cdpCircBufNode* pHead;      // Head of the buffer list.
    cdpCircBufNode* pTail;      // Tail of the buffer list.
} cdpCircBuf;




/*
    Dynamic Circular buffer implementation
*/

static inline cdpCircBuf* circ_buf_new(int capacity) {
    CDP_NEW(cdpCircBuf, circ);
    circ->bufSize = capacity * sizeof(cdpRecord);
    return circ;
}


static inline cdpCircBufNode* circ_buf_node_new(cdpCircBuf* circ) {
    cdpCircBufNode* cNode = cdp_malloc0(sizeof(cdpCircBufNode) + circ->bufSize);
    cNode->first = cNode->last = cNode->record;
    return cNode;
}


#define circ_buf_del        cdp_free
#define circ_buf_node_del   cdp_free


static inline cdpCircBufNode* circ_buf_node_from_record(cdpCircBuf* circ, cdpRecord* record) {
    for (cdpCircBufNode* cNode = circ->pHead;  cNode;  cNode = cNode->pNext) {
        if (cNode->first <= record  &&  cNode->last >= record)
            return cNode;
    }
    return NULL;
}


static inline cdpRecord* circ_buf_add(cdpCircBuf* circ, cdpRecord* parent, bool push, cdpRecMeta* metadata) {   
    assert(cdp_record_is_book(parent));
    cdpRecord* child;
    if (circ->parentEx.chdCount) {
        if (push) {
            // Prepend
            if (circ->pHead->first > circ->pHead->record) {
                circ->pHead->first--;
            } else {
                cdpCircBufNode* cNode = circ_buf_node_new(circ);
                cNode->pNext = circ->pHead;
                circ->pHead->pPrev = cNode;
                circ->pHead = cNode;
            }
            child = circ->pHead->first;
        } else {
            // Append
            if ((void*)circ->pTail->last < cdp_ptr_off(circ->pTail->record, circ->bufSize - sizeof(cdpRecord))) {
                circ->pTail->last++;
            } else {
                cdpCircBufNode* cNode = circ_buf_node_new(circ);
                cNode->pPrev = circ->pTail;
                circ->pTail->pNext = cNode;
                circ->pTail = cNode;
            }
            child = circ->pTail->last;
        }
    } else {
            if (!circ->pTail)
                circ->pTail = circ->pHead = circ_buf_node_new(circ);
            child = circ->pTail->last;
    }
    child->metadata = *metadata;
    return child;
}


static inline cdpRecord* circ_buf_top(cdpCircBuf* circ, bool last) {
    return last?  circ->pTail->last:  circ->pHead->first;
}


static inline cdpRecord* circ_buf_find_by_name(cdpCircBuf* circ, cdpNameID nameID) {
    for (cdpCircBufNode* cNode = circ->pHead;  cNode;  cNode = cNode->pNext) {
        for (cdpRecord* record = cNode->first;  record <= cNode->last;  record++) {
            if (record->metadata.nameID == nameID)
                return record;
        }
    }
    return NULL;
}


static inline cdpRecord* circ_buf_find_by_index(cdpCircBuf* circ, size_t index) {
    // ToDo: use from tail to head if index is closer to it.
    for (cdpCircBufNode* cNode = circ->pHead;  cNode;  cNode = cNode->pNext) {
        size_t chunk = cdp_ptr_idx(cNode->first, cNode->last, sizeof(cdpRecord)) + 1;
        if (chunk > index) {
            return &cNode->record[index];
        }
        index -= chunk;
    }
    return NULL;
}


static inline cdpRecord* circ_buf_prev(cdpCircBuf* circ, cdpRecord* record) {
    cdpCircBufNode* cNode = circ_buf_node_from_record(record);
    assert(cNode);    
    if (cNode->first == record)
        return NULL;
    return record - 1;
}


static inline cdpRecord* circ_buf_next(cdpCircBuf* circ, cdpRecord* record) {
    cdpCircBufNode* cNode = circ_buf_node_from_record(record);
    assert(cNode);    
    if (cNode->last == record)
        return NULL;
    return record + 1;
}


static inline cdpRecord* circ_buf_next_by_name(cdpCircBuf* circ, cdpNameID nameID, cdpCircBufNode** prev) {
    for (cdpCircBufNode* cNode = prev? (*prev)->pNext: circ->pHead;  cNode;  cNode = cNode->pNext) {
        for (cdpRecord* record = cNode->first;  record <= cNode->last;  record++) {
            if (record->metadata.nameID == nameID)
                return record;
        }
    }  
    return NULL;
}


static inline bool circ_buf_traverse(cdpCircBuf* circ, cdpRecord* book, cdpRecordTraverse func, void* context) {
    cdpBookEntry entry = {.parent = book};
    cdpCircBufNode* cNode = circ->pHead;
    do {
        entry.next = cNode->first;
        do {
            if (entry.record) {
                if (!func(&entry, 0, context))
                    return false;
                entry.index++;
                entry.prev = entry.record;
            }
            entry.record = entry.next;
            entry.next++;
        } while (entry.next <= cNode->last);
        cNode = cNode->pNext;
    } while (cNode);
    entry.next = NULL;
    return func(&entry, 0, context);
}


static inline void circ_buf_remove_record(cdpCircBuf* circ, cdpRecord* record) {
    if (record == circ->pHead->first) {
        circ->pHead->first++;
        if (circ->pHead->first <= circ->pHead->last) {
            CDP_0(record);
        } else {
            cdpCircBufNode* cNode = circ->pHead;
            circ->pHead = circ->pHead->pNext;
            if (circ->pHead)
                circ->pHead->pPrev = NULL;
            circ_buf_node_del(cNode);
        }
    } else if (record == circ->pTail->last) {
        circ->pTail->last--;
        if (circ->pTail->last >= circ->pTail->first) {
            CDP_0(record);
        } else {
            cdpCircBufNode* cNode = circ->pTail;
            circ->pTail = circ->pTail->pPrev;
            if (circ->pTail)
                circ->pTail->pNext = NULL;
            circ_buf_node_del(cNode);
        }
    } else {
        // Only popping (first or last) is allowed for circular buffers.
        assert(record == circ->pHead->first || record == circ->pTail->last);
    }
}


static inline void circ_buf_del_all_children(cdpCircBuf* circ, unsigned maxDepth) {
    cdpCircBufNode* cNode = circ->pHead, *toDel;
    if (cNode) {
        do {
            for (cdpRecord* record = cNode->first;  record <= cNode->last;  record++) {
                record_delete_storage(record, maxDepth - 1);
            }
            toDel = cNode;
            cNode = cNode->pNext;
            circ_buf_node_del(toDel);
        } while (cNode);
        circ->pHead = circ->pTail = NULL;
    }
}

