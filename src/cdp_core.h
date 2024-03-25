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

#ifndef CDP_CORE_H
#define CDP_CORE_H

/*
    Cascade Data Processing (CascadeDP) System
    ------------------------------------------

    CascadeDP is designed to represent and manage hierarchical data structures in a distributed execution environment, similar in flexibility to representing complex XML or JSON data models. It facilitates the storage, navigation, and manipulation of records, which can be either data registers (holding actual data values or pointers to data) or books (acting as nodes in the hierarchical structure with potential to have unique or repeatable fields).

    Key Components
    --------------

    * Record: The fundamental unit within the system, capable of acting as either a book (container for other records) or a register (data holder).
    * Book: A type of record that contains child records. Books can enforce uniqueness of child fields (similar to a dictionary or a map) or allow repeatable fields (akin to lists or arrays), supporting complex data structures with ordered or unordered elements.
    * Register: A type of record designed to store actual data, either directly within the record if small enough or through a pointer to larger data sets.
    * Metadata and Flags: Each record contains metadata, including flags that specify the record's characteristics (e.g., book or register, uniqueness of fields) and an identifier indicating the record's role or "name" within its parent, enabling precise navigation and representation within the hierarchy.

    The system is optimized for efficient storage and access, fitting within processor cache lines to enhance performance. It supports navigating from any record to the root of the database, reconstructing paths within the data hierarchy based on unique field identifiers in parent records.

    Goals
    -----

    * Flexibility: To accommodate diverse data models, from strictly structured to loosely defined, allowing for dynamic schema changes.
    * Efficiency: To ensure data structures are compact, minimizing memory usage and optimizing for cache access patterns.
    * Navigability: To allow easy traversal of the hierarchical data structure, facilitating operations like queries, updates, and path reconstruction.

    The system is adept at managing hierarchical data structures through a variety of storage techniques, encapsulated within the cdpVariantBook type. This flexibility allows the system to adapt to different use cases and optimization requirements, particularly focusing on cache efficiency and operation speed for insertions, deletions, and lookups.

    Book and Storage Techniques
    ---------------------------
    
    * cdpVariantBook: Serves as a versatile container within the system, holding child records through diverse storage strategies. Each cdpVariantBook can adopt one of several child storage mechanisms, determined by the childStorage flag in its metadata. This design enables tailored optimization based on specific needs, such as operation frequency, data volume, and access patterns.

    * Storage Techniques:
    
      Array: Offers fast access and efficient cache utilization for densely packed records. Ideal for situations where the number of children is relatively static and operations are predominantly at the tail end.
      Circular Buffer: Enhances the array model with efficient head and tail operations, suitable for queues or streaming data scenarios.
      Doubly Linked List: Provides flexibility for frequent insertions and deletions at arbitrary positions with minimal overhead per operation.
      Packed List: Strikes a balance between the cache efficiency of arrays and the flexibility of linked lists. It's optimized for scenarios where both random access and dynamic modifications are common.
      Red-Black Tree: Ensures balanced tree structure for ordered data, offering logarithmic time complexity for insertions, deletions, and lookups. Particularly useful for datasets requiring sorted access.
      B-tree: Similar to Red-Black trees in providing sorted access but optimized for scenarios involving large datasets and minimizing page loads, which is crucial for disk-based storage systems.
      
    Each storage technique is selected to optimize specific aspects of data management, addressing the system's goals of flexibility, efficiency, and navigability.

*/



#include <stddef.h>
#include <stdint.h>


// Record metadata:
#define CDP_FLAG_LINK                   0x01  // Record is a register with data pointing to another record
#define CDP_FLAG_MULTIPLE_PARENTS       0x02  // Record has more than one parent.
#define CDP_FLAG_TYPE_BOOK              0x04  // Record is a book.
#define CDP_FLAG_BOOK_UNIQUE_FIELDS     0x08  // Book enforces unique child fields.
#define CDP_FLAG_BOOK_RB_TREE_N_RED     0x80  // Node color flag for red-black tree children.

enum {
    CDP_BOOK_CHD_ARRAY,         // Children stored in an array.
    CDP_BOOK_CHD_CIR_BUF,       // Children stored in circular buffer.
    CDP_BOOK_CHD_L_LIST,        // Children stored in a doubly linked list.
    CDP_BOOK_CHD_PACK_LIST,     // Children stored in a packed list.
    CDP_BOOK_CHD_RB_TREE,       // Children stored in a red-black tree (requires unique fields).
    CDP_BOOK_CHD_BE_TREE        // Children stored in a B-tree (requires unique fields).
};

typedef uint32_t cdpNameID;

typedef struct {
    uint32_t flags: 5,          // Flags for record properties (e.g., type, data storage method).
             childStorage: 3,   // Child storage technique (array, circular buffer, linked list, etc.)
             typeID: 24;        // Unique type identifier for the record.
    cdpNameID nameID;           // Unique name/field identifier in the parent record.
} cdpRecordMetadata;


// Record definitions:
typedef struct {
    union {
        void* ptr;          // Pointer to large data sets
        uintptr_t direct;   // Direct storage for small data sets
    } data;
    size_t size;            // Data buffer size in bytes
} cdpRegisterData;

typedef struct {
    void* data;             // Pointer to either cdpChildrenArray, cdpChildrenList, cdpChildrenRbTree, etc.
    size_t childrenCount;   // Number of child records, for quick access and management
} cdpVariantBook;

typedef union {
    cdpVariantBook book;    // Book structure for hierarchical data organization
    cdpRegisterData reg;    // Register structure for actual data storage
} cdpRecordData;

typedef struct {
    size_t count;           // Number of parent pointers
    cdpRecord* parents[];   // Dynamic array of parent pointers
} cdpParentArrayHeader;

typedef struct cdpRecordStruct {
    union {
        struct cdpRecordStruct* singleParent;            // Pointer to a single parent
        struct cdpParentArrayHeader* multipleParents; // Pointer to a structure for managing multiple parents
    } parent;                         // Parent pointer(s)
    cdpRecordMetadata metadata;       // Metadata including flags and nameID
    cdpRecordData data;               // Data, either a book or register
} cdpRecord;


// Children storage techniques:
typedef struct {
    size_t capacity;        // Total capacity of the array to manage allocations
    cdpRecord record[];     // Dynamic array of children Record
} cdpChildrenArray;

typedef struct {
    size_t head;            // Head index for the next read.
    size_t tail;            // Tail index for the next write.
    size_t capacity;        // Total capacity of the buffer.
    cdpRecord record[];     // Dynamic array of children Record
} cdpChildrenCirBuffer;

typedef struct cdpListNode {
    struct cdpListNode *prev, *next;
    cdpRecord record;
} cdpListNode;

typedef struct {
    cdpListNode* head;      // Head of the doubly linked list
    cdpListNode* tail;      // Tail of the doubly linked list for quick append
} cdpChildrenList;

typedef struct cdpPackedListNode {
    size_t count;                   // Number of valid records in this node's pack
    struct cdpPackedListNode* next; // Pointer to the next node in the list
    cdpRecord record[];             // Fixed-size array of cdpRecords
} cdpPackedListNode;

typedef struct {
    cdpPackedListNode* head;  // Head of the packed list
    cdpPackedListNode* tail;  // Tail of the packed list for quick append operations
    size_t packSize;          // Capacity of each pack (Total count of records across all packs is in childrenCount)
} cdpChildrenPackedList;

typedef struct cdpRbTreeNode {
    struct cdpRbTreeNode *parent, *left, *right;
    cdpRecord record;         // Node color is stored in the record's metadata flags, using CDP_FLAG_BOOK_RB_TREE_N_RED
} cdpRbTreeNode;

typedef struct {
    cdpRbTreeNode* root;      // Root of the red-black tree
} cdpChildrenRbTree;

#define CDP_BE_TREE_ORDER 5       // Minimum degree (t), adjust based on needs

typedef struct cdpBeTreeNode {
    int leaf;                                               // Is true when node is leaf. Otherwise false
    int n;                                                  // Current number of keys
    cdpNameID keys[2 * CDP_BE_TREE_ORDER - 1];              // Array of keys
    struct cdpBeTreeNode* children[2 * CDP_BE_TREE_ORDER];  // Array of child pointers
    cdpRecord record[2 * CDP_BE_TREE_ORDER - 1];            // Records associated with keys
} cdpBeTreeNode;

typedef struct {
    cdpBeTreeNode* root;      // Pointer to the root node
} cdpChildrenBeTree;





// Adds a new record to a specified parent book.
cdpRecord* cdp_record_create(cdpRecord* parent, cdpRecordData data, cdpNameID nameID, uint32_t typeID, unsigned childStorage);

// Finds a record based on a path of nameIDs starting from the root or a given record.
cdpRecord* cdp_record_find_by_path(cdpRecord* start, cdpNameID* path, size_t path_length);

// Retrieves a direct child record by its nameID within a book.
cdpRecord* cdp_record(cdpRecord* book, cdpNameID nameID);

// To manage concurrent access to records safely.
bool cdp_record_lock(cdpRecord* record);
bool cdp_record_unlock(cdpRecord* record);

// Moves a record from one book to another.
bool cdp_record_move(cdpRecord* record, cdpRecord* newParent);

// Creates a duplicate of a record, optionally including its children if it's a book.
cdpRecord* cdp_record_copy(const cdpRecord* record, cdpRecord* targetParent, bool includeChildren);

// Creates a link from one record to another (target), updating the target parent list as necessary.
bool cdp_record_link(cdpRecord* record, cdpRecord* target);

// Updates the data of a register record.
bool cdp_record_update_register(cdpRecord* reg, const void* data, size_t size);

// Deletes a record and optionally all its children if it's a book.
bool cdp_record_delete(cdpRecord* record, bool delete_children);

// Traverses the children of a book record, applying a function to each.
int cdp_record_traverse(cdpRecord* book, int (*func)(cdpRecord*, void*), void* context);

// Constructs the full path (sequence of nameIDs) for a given record.
int cdp_record_path(cdpRecord* record, cdpNameID** path, size_t* path_length);

// Returns non-zero if the record is a book; otherwise, it's a register.
bool cdp_record_is_book(cdpRecord* record);


#endif
