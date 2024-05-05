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


typedef struct {
    cdpParentEx parentEx;         // Parent info.
    //                            
    size_t      cHead;            // Head index for the next read.
    size_t      cCapacity;        // Total capacity of the buffer.
    //
    cdpRecord*  record;           // Children Record
} cdpCircBuf;




/*
    Circular buffer implementation
*/

static inline cdpCircBuf* circ_buf_new(int capacity) {
    CDP_NEW(cdpCircBuf, circb);
    circb->cCapacity = capacity;
    circb->record = cdp_malloc0(capacity * sizeof(cdpRecord));
    return circb;
}


static inline void circ_buf_del(cdpCircBuf* circb) {
    cdp_free(circb->record);
    cdp_free(circb);
}


static inline cdpRecord* circ_buf_add(cdpCircBuf* circb, cdpRecord* parent, bool push, cdpRecMeta* metadata) {   
    cdpRecord* child;
    if (circb->parentEx.chdCount) {
        if (push) {
            // Prepend
            if (circb->cCapacity == circb->parentEx.chdCount) {
            } else {
                if (circb->cHead)
                    circb->cHead--;
                else
                    circb->cHead = circb->cCapacity - 1;
                child = &circb->record[circb->cHead];
            }
        } else {
            // Append
            if (circb->cCapacity == circb->parentEx.chdCount) {
                // Overwrite buffer if full
                child = &circb->record[circb->cHead];
                record_delete_storage(child, UINT_MAX);     // FixMe: maxDepth!
                CDP_0(child);
                circb->cHead++;
                if (circb->cHead == circb->cCapacity)
                    circb->cHead = 0;
                circb->parentEx.chdCount--;     // FixMe: no child count increase.
            } else {
                // Increase buffer
                size_t cTail = circb->cHead + circb->parentEx.chdCount;
                if (cTail >= circb->cCapacity)
                    cTail -= circb->cCapacity;
                child = &circb->record[cTail];
            }
        }
    } else {
            circb->cHead = 0;
            child = circb->record;
    }
    child->metadata = *metadata;
    return child;
}


static inline cdpRecord* circ_buf_top(cdpCircBuf* circb, bool last) {
    assert(circb->cCapacity >= circb->parentEx.chdCount);
    return last?  &circb->record[circb->parentEx.chdCount - 1]:  circb->record;
}


static inline cdpRecord* circ_buf_find_by_name(cdpCircBuf* circb, cdpNameID nameID, cdpRecord* book) {
    if (cdp_record_is_dictionary(book) && !circb->parentEx.compare) {
        cdpRecord key = {.metadata.nameID = nameID};
        return circ_buf_search(circb, &key, record_compare_by_name_s, NULL, NULL);
    } else {
        cdpRecord* record = circb->record;
        for (size_t i = 0; i < circb->parentEx.chdCount; i++, record++) {
            if (record->metadata.nameID == nameID)
                return record;
        }
    }
    return NULL;
}


static inline cdpRecord* circ_buf_find_by_index(cdpCircBuf* circb, size_t index) {
    assert(index < circb->parentEx.chdCount);
    return &circb->record[index];
}


static inline cdpRecord* circ_buf_prev(cdpCircBuf* circb, cdpRecord* record) {
    return (record > circb->record)? record - 1: NULL;
}


static inline cdpRecord* circ_buf_next(cdpCircBuf* circb, cdpRecord* record) {
    cdpRecord* last = &circb->record[circb->parentEx.chdCount - 1];
    return (record < last)? record + 1: NULL;
}


static inline cdpRecord* circ_buf_next_by_name(cdpCircBuf* circb, cdpNameID nameID, uintptr_t* prev) {
    cdpRecord* record = circb->record;
    for (size_t i = prev? (*prev + 1): 0;  i < circb->parentEx.chdCount;  i++, record++){
        if (record->metadata.nameID == nameID)
            return record;
    }   
    return NULL;
}

static inline bool circ_buf_traverse(cdpCircBuf* circb, cdpRecord* book, cdpRecordTraverse func, void* context){
    assert(circb && circb->cCapacity >= circb->parentEx.chdCount);
    cdpBookEntry entry = {.record = circb->record, .parent = book, .next = (circb->parentEx.chdCount > 1)? (circb->record + 1): NULL};
    cdpRecord* last = &circb->record[circb->parentEx.chdCount - 1];
    for (;;) {
        if (!func(&entry, 0, context))
            return false;
        entry.index++;
        if (entry.index >= circb->parentEx.chdCount)
            break;
        entry.prev   = entry.record;
        entry.record = entry.next;
        entry.next = (entry.record < last)? (entry.next + 1): NULL;
    }
    return true;
}


static inline void circ_buf_sort(cdpCircBuf* circb, cdpCompare compare, void* context) {
    if (!compare) {
        qsort(circb->record, circb->parentEx.chdCount, sizeof(cdpRecord), (cdpFunc) record_compare_by_name);
    } else {
      #ifdef _GNU_SOURCE
        qsort_r
      #else
        qsort_s
      #endif
        (circb->record, circb->parentEx.chdCount, sizeof(cdpRecord), (cdpFunc) compare, context);
    }
    circ_buf_update_children_parent_ptr(circb->record, &circb->record[circb->parentEx.chdCount - 1]);
}


static inline void circ_buf_remove_record(cdpCircBuf* circb, cdpRecord* record) {
    assert(circb && circb->cCapacity >= circb->parentEx.chdCount);
    cdpRecord* last = &circb->record[circb->parentEx.chdCount - 1];
    if (record < last)
        memmove(record, record + 1, (size_t) cdp_ptr_dif(last, record));
    CDP_0(last);
}


static inline void circ_buf_del_all_children(cdpCircBuf* circb, unsigned maxDepth) {
    cdpRecord* child = circb->record;
    for (size_t n = 0; n < circb->parentEx.chdCount; n++, child++) {
        record_delete_storage(child, maxDepth - 1);
    }
}

