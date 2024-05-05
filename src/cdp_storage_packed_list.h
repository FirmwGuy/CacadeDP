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


typedef struct _cdpPackListNode cdpPackListNode;

struct _cdpPackListNode {
    cdpPackListNode*  pNext;      // Pointer to the next node in the list
    size_t            count;      // Number of valid records in this node's pack
    //
    cdpRecord         record[];   // Fixed-size array of cdpRecords
};

typedef struct {
    cdpParentEx       parentEx;   // Parent info.
    //
    cdpPackListNode*  pHead;      // Head of the packed list
    cdpPackListNode*  pTail;      // Tail of the packed list for quick append operations
    size_t            packSize;   // Capacity of each pack (Total count of records across all packs is in chdCount)
} cdpPackList;




/*
    Packed list implementation
*/


