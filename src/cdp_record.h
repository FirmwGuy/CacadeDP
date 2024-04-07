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
    Cascade Data Processing (CascadeDP) System
    ------------------------------------------

    CascadeDP is designed to represent and manage hierarchical data structures 
    in a distributed execution environment, similar in flexibility to 
    representing complex XML or JSON data models. It facilitates the storage, 
    navigation, and manipulation of records, which can be either data registers 
    (holding actual data values or pointers to data) or books (acting as nodes 
    in the hierarchical structure with potential to have unique or repeatable 
    fields).

    Key Components
    --------------

    * Record: The fundamental unit within the system, capable of acting as 
    either a book (container for other records) or a register (data holder).
    
    * Book: A type of record that contains child records. Books can enforce 
    uniqueness of child fields (similar to a dictionary or a map) or allow 
    repeatable fields (akin to lists or arrays), supporting complex data 
    structures with ordered or unordered elements.
    
    * Register: A type of record designed to store actual data, either directly 
    within the record if small enough or through a pointer to larger data sets.
    
    * Link: A record that points to another record.
    
    * Metadata and Flags: Each record contains metadata, including flags that 
    specify the record's characteristics (e.g., book or register, uniqueness of 
    fields) and an identifier indicating the record's role or "name" within its 
    parent, enabling precise navigation and representation within the 
    hierarchy.

    The system is optimized for efficient storage and access, fitting within 
    processor cache lines to enhance performance. It supports navigating from 
    any record to the root of the database, reconstructing paths within the 
    data hierarchy based on field identifiers in parent records.

    Goals
    -----

    * Flexibility: To accommodate diverse data models, from strictly structured 
    to loosely defined, allowing for dynamic schema changes.
    
    * Efficiency: To ensure data structures are compact, minimizing memory 
    usage and optimizing for cache access patterns.
    
    * Navigability: To allow easy traversal of the hierarchical data structure, 
    facilitating operations like queries, updates, and path reconstruction.

    The system is adept at managing hierarchical data structures through a 
    variety of storage techniques, encapsulated within the cdpVariantBook type. 
    This flexibility allows the system to adapt to different use cases and 
    optimization requirements, particularly focusing on cache efficiency and 
    operation speed for insertions, deletions, and lookups.

    Book and Storage Techniques
    ---------------------------
    
    * cdpVariantBook: Serves as a versatile container within the system, 
    holding child records through diverse storage strategies. Each 
    cdpVariantBook can adopt one of several child storage mechanisms, 
    determined by the childStorage flag in its metadata. This design enables 
    tailored optimization based on specific needs, such as operation frequency, 
    data volume, and access patterns.

    * Storage Techniques: Each storage technique is selected to optimize 
    specific aspects of data management, addressing the system's goals of 
    flexibility, efficiency, and navigability.
    
      Array: Offers fast access and efficient cache utilization for densely 
      packed records. Ideal for situations where the number of children is 
      relatively static and operations are predominantly at the tail end.
      
      Circular Buffer: Enhances the array model with efficient head and tail 
      operations, suitable for queues or streaming data scenarios.
      
      Doubly Linked List: Provides flexibility for frequent insertions and 
      deletions at arbitrary positions with minimal overhead per operation.
      
      Packed List: Strikes a balance between the cache efficiency of arrays and 
      the flexibility of linked lists. It's optimized for scenarios where both 
      random access and dynamic modifications are common.
      
      Red-Black Tree: Ensures balanced tree structure for ordered data, 
      offering logarithmic time complexity for insertions, deletions, and 
      lookups. Particularly useful for datasets requiring sorted access.
      
      B-tree: Similar to Red-Black trees in providing sorted access but 
      optimized for scenarios involving large datasets and minimizing page 
      loads, which is crucial for disk-based storage systems.
    
    * Locking System: The lock mechanism is implemented by serializing all IOs 
    and keeping a list of currently locked book paths and a list of locked 
    register pointers. On each input/output the system checks on the lists 
    depending of the specified record.
    
    * Private Records: Records may be private, in which case no locking is
    necessary since they are never made public.

*/

#ifndef CDP_RECORD_H
#define CDP_RECORD_H


#include "cdp_util.h"


/*
 * Record Metadata
 */
 
#define CDP_FLAG_MULTIPLE_PARENTS   0x01    // Record has more than one parent.
#define CDP_FLAG_PRIVATE            0x02    // Record and all its children are private (unlockable).

enum {
    CDP_REC_STYLE_BOOK,
    CDP_REC_STYLE_REGISTER,
    CDP_REC_STYLE_LINK,
    //
    CDP_REC_STYLE_COUNT
};

enum {
    CDP_BOOK_SORT_NONE,         // Book keeps its fields unsorted.
    CDP_BOOK_SORT_BY_NAME,      // Book sorts its children by their (unique) nameID.
    CDP_BOOK_SORT_BY_DATA,      // Book sorts its children by their own register data (useful for making indexes).
    CDP_BOOK_SORT_BY_KEY,       // Book sorts its children by a register data (key) found on a grand-child nameID.
    //
    CDP_BOOK_SORT_COUNT
};

#define CDP_BOOK_SORTED     CDP_BOOK_SORT_BY_NAME

enum {
    // Unsortable and pushable:
    CDP_CHD_STO_CIRC_BUFFER,    // Children stored in a circular buffer.
    CDP_CHD_STO_LINKED_LIST,    // Children stored in a doubly linked list.
    
    // Sortable and pushable:
    CDP_CHD_STO_ARRAY,          // Children stored in an array.
    CDP_CHD_STO_PACKED_LIST,    // Children stored in a packed list.
    
    // Sorted and non-pushable:
    CDP_CHD_STO_RED_BLACK_T,    // Children stored in a red-black tree (requires sorted books).
    CDP_CHD_STO_B_TREE,         // Children stored in a B-tree (requires sorted books).
    //
    CDP_CHD_STO_COUNT
};

#define CDP_CHD_SORTABLE        CDP_CHD_STO_ARRAY
#define CDP_CHD_NON_PUSHABLE    CDP_CHD_STO_RED_BLACK_T

typedef uint32_t cdpNameID;

typedef struct {
    uint32_t  propFg: 2,        // Flags for record properties (multiple parents, private).
              rStyle: 2,        // Style of the record (book, register or link)
              sorted: 2,        // Children ordering method (sorted, unsorted, etc).
              chdSto: 3,        // Children storage technique (array, circular buffer, linked list, etc.)
              typeID: 23;       // Type identifier for the record.
    cdpNameID nameID;           // Name/field identifier in the parent record.
} cdpRecordMetadata;


/*
 * Record Data
 */
 
typedef struct {
    union {
        void*     ptr;      // Pointer to large data sets
        uintptr_t direct;   // Direct storage for small data sets
    } data;
    size_t size;            // Data buffer size in bytes
} cdpRegisterData;

typedef struct {
    void*  children;        // Pointer to either cdpRecChdArray, cdpRecChdList, cdpRecChdRbTree, etc.
    size_t chdCount;        // Number of child records, for quick access and management
} cdpVariantBook;

typedef union {
    cdpVariantBook  book;   // Book structure for hierarchical data organization
    cdpRegisterData reg;    // Register structure for actual data storage
} cdpRecordData;

typedef struct {
    size_t     count;       // Number of parent pointers
    cdpRecord* coparent[];  // Dynamic array of parent pointers
} cdpParentMultiple;

typedef struct cdpRecordStruct {
    union {
        struct cdpRecordStruct*   single;   // Pointer to a single parent
        struct cdpParentMultiple* multiple; // Pointer to a structure for managing multiple parents
    } parent;                               // Parent pointer(s)
    cdpRecordMetadata metadata;             // Metadata including flags and nameID
    cdpRecordData     recData;              // Data, either a book or register
} cdpRecord;


/*
 * Children Storage Techniques
 */
 
typedef struct {
    size_t capacity;        // Total capacity of the array to manage allocations
    cdpRecord record[];     // Dynamic array of children Record
} cdpRecChdArray;

typedef struct {
    size_t head;            // Head index for the next read.
    size_t capacity;        // Total capacity of the buffer.
    cdpRecord record[];     // Dynamic array of children Record
} cdpRecChdCirBuffer;

typedef struct cdpListNode {
    struct cdpListNode *next;
    struct cdpListNode *prev;
    cdpRecord record;
} cdpListNode;

typedef struct {
    cdpListNode* head;      // Head of the doubly linked list
    cdpListNode* tail;      // Tail of the doubly linked list for quick append
} cdpRecChdList;

typedef struct cdpPackedListNode {
    struct cdpPackedListNode* next; // Pointer to the next node in the list
    size_t count;                   // Number of valid records in this node's pack
    cdpRecord record[];             // Fixed-size array of cdpRecords
} cdpPackedListNode;

typedef struct {
    cdpPackedListNode* head;  // Head of the packed list
    cdpPackedListNode* tail;  // Tail of the packed list for quick append operations
    size_t packSize;          // Capacity of each pack (Total count of records across all packs is in childrenCount)
} cdpRecChdPackedList;

typedef struct cdpRbTreeNode {
    struct cdpRbTreeNode *left, *right, *t_parent;
    bool nodeRed;             // True if node is red.
    cdpRecord record;
} cdpRbTreeNode;

typedef struct {
    cdpRbTreeNode* root;      // Root of the red-black tree
} cdpRecChdRbTree;

#define CDP_BE_TREE_ORDER 5   // Minimum degree (t), adjust based on needs

typedef struct cdpBeTreeNode {
    int leaf;                                                 // Is true when node is leaf.
    int n;                                                    // Current number of keys.
    uintptr_t keys[2 * CDP_BE_TREE_ORDER - 1];                // Array of keys.
    struct cdpBeTreeNode* t_children[2 * CDP_BE_TREE_ORDER];  // Array of child pointers.
    cdpRecord record[2 * CDP_BE_TREE_ORDER - 1];              // Records associated with keys.
} cdpBeTreeNode;

typedef struct {
    cdpBeTreeNode* root;      // Pointer to the root node
} cdpRecChdBeTree;


/*
 * Other C Data Types
 */
typedef struct {
    unsigned  length;
    unsigned  capacity;
    cdpNameID nameID[];
} cdpPath;


typedef struct {
    cdpRecord* record;
    cdpRecord* parent;
    cdpRecord* next;
    cdpRecord* prev;
    size_t     index;
} cdpBookEntry;

typedef bool (*cdpRecordTraverse)(cdpBookEntry*, unsigned, void*);


/*
 * Record Operations
 */

// General property check
static inline bool cdp_record_is_register(cdpRecord* record)  {assert(record);  return (record->metadata.rStyle == CDP_REC_STYLE_REGISTER);}
static inline bool cdp_record_is_link    (cdpRecord* record)  {assert(record);  return (record->metadata.rStyle == CDP_REC_STYLE_LINK);}
static inline bool cdp_record_is_book    (cdpRecord* record)  {assert(record);  return (record->metadata.rStyle == CDP_REC_STYLE_BOOK);}
static inline bool cdp_record_is_private (cdpRecord* record)  {assert(record);  return (record->metadata.propFg & CDP_FLAG_PRIVATE) != 0;}
static inline bool cdp_record_w_coparents(cdpRecord* record)  {assert(record);  return (record->metadata.propFg & CDP_FLAG_MULTIPLE_PARENTS) != 0;}

// Register property check
static inline bool cdp_record_register_borrowed(cdpRecord* reg)  {assert(cdp_record_is_register(reg));  return reg->metadata.chdSto != 0;}


// Book property check
static inline int cdp_record_book_sorting(cdpRecord* book) {
    assert(cdp_record_is_book(book));
    return book->metadata.sorted;
}
#define cdp_record_book_sorted(book)    (cdp_record_book_sorting(book) >= CDP_BOOK_SORTED && (book)->metadata.chdSto >= CDP_CHD_SORTABLE)

static inline bool cdp_record_book_pushable(cdpRecord* book) {
    assert(cdp_record_is_book(book));
    return (book->metadata.chdSto < CDP_CHD_NON_PUSHABLE);
}


// Appends/inserts (or pushes) a new record.
cdpRecord* cdp_record_create(cdpRecord* parent, unsigned style, cdpNameID nameID, uint32_t typeID, bool push, bool priv, ...);
#define cdp_record_add_register(parent, nameID, typeID, data, size)         cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID, false, false, data, size)
#define cdp_record_push_register(parent, nameID, typeID, data, size)        cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID,  true, false, data, size)
#define cdp_record_add_register_priv(parent, nameID, typeID, data, size)    cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID, false,  true, data, size)
#define cdp_record_push_register_priv(parent, nameID, typeID, data, size)   cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID,  true,  true, data, size)


#define cdp_record_add_book(parent, nameID, typeID, chdStorage, chdSort, ...)       cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID, false, false, chdStorage, chdSort, __VA_ARGS__)
#define cdp_record_push_book(parent, nameID, typeID, chdStorage, chdSort, ...)      cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID,  true, false, chdStorage, chdSort, __VA_ARGS__)
#define cdp_record_add_book_priv(parent, nameID, typeID, chdStorage, chdSort, ...)  cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID, false,  true, chdStorage, chdSort, __VA_ARGS__)
#define cdp_record_push_book_priv(parent, nameID, typeID, chdStorage, chdSort, ...) cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID,  true,  true, chdStorage, chdSort, __VA_ARGS__)

// Accessing registers
bool cdp_record_register_read (cdpRecord* reg, size_t position, void* data, size_t* size);        // Reads register data from position and puts it on data buffer (atomically).
bool cdp_record_register_write(cdpRecord* reg, size_t position, const void* data, size_t size);   // Writes the data of a register record at position (atomically and it may reallocate memory).
#define cdp_record_register_update(reg, data, size)   cdp_record_register_write(reg, 0, data, size)

// Constructs the full path (sequence of nameIDs) for a given record, returning the depth.
bool cdp_record_path(cdpRecord* record, cdpPath** path);

// Accessing books
cdpRecord* cdp_record_top     (cdpRecord* book, bool last);                               // Gets the first (or last) record from book.
cdpRecord* cdp_record_by_name (cdpRecord* book, cdpNameID nameID);                        // Retrieves a child record by its nameID.
cdpRecord* cdp_record_by_key  (cdpRecord* book, cdpNameID siblingKeyID, cdpCmp compare, const void* key, size_t kSize);  // Finds a child record based on specified key.
cdpRecord* cdp_record_by_index(cdpRecord* book, size_t index);                            // Gets the child record at index position from book.
cdpRecord* cdp_record_by_path (cdpRecord* start, cdpPath** path);                         // Finds a child record based on a path of nameIDs starting from the root or a given book.

cdpRecord* cdp_record_next_by_name(cdpRecord* book, cdpNameID nameID, uintptr_t* prev);   // Retrieves the first/next (unsorted) child record by its nameID.
cdpRecord* cdp_record_next_by_path(cdpRecord* start, cdpPath** path, uintptr_t* prev);    // Finds the first/next (unsorted) record based on a path of nameIDs starting from the root or a given book.

bool cdp_record_traverse     (cdpRecord* book, cdpRecordTraverse func, void* context);    // Traverses the children of a book record, applying a function to each.
bool cdp_record_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, cdpRecordTraverse listEnd, void* context);  // Traverses each sub-branch of a book record.

// Removing records
bool cdp_record_delete(cdpRecord* record, unsigned maxDepth);                     // Deletes a record and all its children re-organizing sibling storage.
#define cdp_record_delete_register(reg)   cdp_record_delete(reg, 0)

// To manage concurrent access to records safely.
bool cdp_record_lock(cdpRecord* record);
bool cdp_record_unlock(cdpRecord* record);

// Moves a record from one book to another.
bool cdp_record_move(cdpRecord* record, cdpRecord* newParent);

// Creates a duplicate of a record and adds/inserts it (or pushes it) to a book under nameID, optionally including its children.
cdpRecord* cdp_record_add_copy(const cdpRecord* book, cdpNameID nameID, cdpRecord* record, bool includeChildren, bool push);

// Creates a new link to a record and adds/inserts it (or pushes it) to a book under nameID, updating the source record parent list as necessary.
cdpRecord* cdp_record_add_link(cdpRecord* book, cdpNameID nameID, cdpRecord* record, bool push);


CDP_FUNC_CMPi_(cdp_record_compare_by_name, cdpRecord, metadata.nameID)



#endif
