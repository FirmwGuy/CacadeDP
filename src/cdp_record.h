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

#ifndef CDP_RECORD_H
#define CDP_RECORD_H


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
    as either a book, a register, a link, etc.

    * Book: A type of record that contains child records. Records are
    kept in the order they are added. Duplicated content is allowed.

    * Register: A type of record designed to store actual data, either
    directly within the record if small enough or through a pointer to
    larger data sets.

    * Link: A record that points to another record.

    * Metadata and Flags: Each record contains metadata, including
    flags that specify the record's characteristics, an identifier
    indicating the record's role or ID within its parent, and a
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
    mechanisms, determined by the storeTech indicator in its metadata.
    This design enables tailored optimization based on specific needs,
    such as operation frequency, data volume, and access patterns.

    * Storage Techniques: Each storage technique is selected to
    optimize specific aspects of data management, addressing the
    system's goals of flexibility, efficiency, and navigability.

      Doubly Linked List: Provides flexibility for frequent insertions
      and deletions at arbitrary positions with minimal overhead per
      operation.

      Array: Offers fast access and efficient cache utilization for
      densely packed records. Ideal for situations where the number of
      children is relatively static and operations are predominantly at
      the tail end.

      Packed Queue: Strikes a balance between the cache efficiency of
      arrays and the flexibility of linked lists. It's optimized for
      scenarios where operations in head and tail are common.

      Red-Black Tree: Ensures balanced tree structure for ordered data,
      offering logarithmic time complexity for insertions, deletions,
      and lookups. Particularly useful for datasets requiring sorted
      access.

    * Locking System: The lock mechanism is implemented by serializing
    all IOs and keeping a list of currently locked book paths and a
    list of locked register pointers. On each input/output the system
    checks on the lists depending of the specified record.

    * Record Attributes: records may have several attributes.

      Private Records: Records may be private, in which case no locking
      is necessary since they are not meant to be available to other
      than the creator.

      Factual Records: Records is no longer a state (it can't be modified)
      but a fact (it only conveys value).

*/


#include "cdp_util.h"


typedef struct _cdpRecord       cdpRecord;
typedef struct _cdpPath         cdpPath;


/*
 * Record Metadata
 */

#define CDP_ATTRIB_PRIVATE      0x01    // Record and all its children are private (unlockables).
#define CDP_ATTRIB_FACTUAL      0x02    // Record can't be modified anymore.
#define CDP_ATTRIB_NAMED        0x04    // Record ID corresponds to an interned string.
#define CDP_ATTRIB_SHADOWED     0x08    // Record has shadow records (links pointing to it).

enum {
    CDP_STO_CHD_LINKED_LIST,    // Children stored in a doubly linked list.
    CDP_STO_CHD_ARRAY,          // Children stored in an array.
    CDP_STO_CHD_PACKED_QUEUE,   // Children stored in a packed queue (record must be a book).
    CDP_STO_CHD_RED_BLACK_T,    // Children stored in a red-black tree (record must be a dictionary).
    //
    CDP_STO_CHD_COUNT
};

enum {
    CDP_STO_REG_OWNED,          // Register owns its own data.
    CDP_STO_REG_BORROWED,       // Register has borrowed data (only for private records).
    //
    CDP_STO_REG_COUNT
};

enum {
    CDP_STO_LNK_POINTER,        // Link points to an in-memory record.
    CDP_STO_LNK_PATH,           // Link hold the address of an off-memory record.
    //
    CDP_STO_LNK_COUNT
};

// Primal  types:
enum _CDP_TYPE_PRIMAL {
    CDP_TYPE_NONE,              // This is the "nothing" type.
    CDP_TYPE_REGISTER,
    CDP_TYPE_LINK,
    CDP_TYPE_BOOK,

    CDP_TYPE_PRIMAL_COUNT
};

// Initial CascadeDP system type:
enum _CDP_TYPE {
    // Book types
    CDP_TYPE_DICTIONARY = CDP_TYPE_PRIMAL_COUNT,
    CDP_TYPE_CATALOG,
    CDP_TYPE_LIST,
    CDP_TYPE_SET,
    CDP_TYPE_QUEUE,
    //
    CDP_TYPE_CHRONICLE,
    CDP_TYPE_ENCYCLOPEDIA,
    CDP_TYPE_COMPENDIUM,
    CDP_TYPE_COLLECTION,
    CDP_TYPE_LOG,

    // Register types
    CDP_TYPE_PATCH,
    CDP_TYPE_ID,
    CDP_TYPE_NAME,
    CDP_TYPE_UTF8,
    //
    CDP_TYPE_BOOLEAN,
    CDP_TYPE_BYTE,
    CDP_TYPE_UINT16,
    CDP_TYPE_UINT32,
    CDP_TYPE_UINT64,
    CDP_TYPE_INT16,
    CDP_TYPE_INT32,
    CDP_TYPE_INT64,
    CDP_TYPE_FLOAT32,
    CDP_TYPE_FLOAT64,

    // Structured types
    CDP_TYPE_TYPE,

    CDP_TYPE_COUNT
};

typedef uint32_t cdpID;

#define CDP_TYPE_MAX_ID     0xFFFFFF

typedef struct {
    uint32_t  attribute: 4, // Flags for record attributes.
              primal:    2, // Primal type (book, register, link).
              storeTech: 2, // Record storage technique (it depends of the type of record).
              type:     24; // Type tag for the record. It inlcudes _CDP_TYPE + user defined types.
    cdpID     id;           // Name/field identifier in the parent record.
} cdpRecMeta;


/*
 * Record Data
 */

typedef struct {
    void*     children;     // Pointer to either cdpArray, cdpList, cdpPackedQ, or cdpRbTree.
    void*     property;     // Book property dictionary (as cdpRbTree). Reserved for future use.
} cdpVariantBook;

typedef struct {
    union {
        void*     ptr;      // Pointer to large data sets
        uintptr_t direct;   // Direct storage for small data sets
    } data;
    size_t size;            // Data buffer size in bytes
} cdpRegisterData;

typedef struct {
    union {
      cdpRecord* address;   // Memory address of target record.
      cdpPath*   path;      // Full or relative path to target.
    } target;
    bool inRam;             // Target record is in ram.
} cdpLink;

typedef union {
    cdpVariantBook  book;   // Book structure for hierarchical data organization
    cdpRegisterData reg;    // Register structure for actual data storage
    cdpLink         link;   // Link structure for de-referencing.
} cdpRecData;

struct _cdpRecord {
    cdpRecMeta  metadata;   // Metadata including flags and id.
    cdpRecData  recData;    // Data, either a book, a register or a link.
    void*       storage;    // Pointer to the parent's storage structure (list, array, etc.)
};


/*
 * Children Storage Techniques
 */
typedef int (*cdpCompare)(const cdpRecord* restrict, const cdpRecord* restrict, void*);


typedef struct {
    size_t      count;            // Number of record pointers
    cdpRecord*  record[];         // Dynamic array of links shadowing this one.
} cdpShadow;

typedef struct {
    cdpRecord*  book;             // Parent (book or dictionary) owning this child storage.
    cdpShadow*  shadow;           // Pointer to a structure for managing multiple (linked) parents.
    size_t      chdCount;         // Number of child records.
    cdpCompare  compare;          // Compare function for dictionaries.
    void*       context;          // User defined context data for searches.
} cdpParentEx;


/*
 * Other C Data Types
 */

struct _cdpPath {
    unsigned  length;
    unsigned  capacity;
    cdpID id[];
};

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
static inline bool cdp_record_is_book       (cdpRecord* record)  {assert(record);  return (record->metadata.primal == CDP_TYPE_BOOK);}
static inline bool cdp_record_is_register   (cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_REGISTER);}
static inline bool cdp_record_is_link       (cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_LINK);}
static inline bool cdp_record_is_dictionary (cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_DICTIONARY);}
static inline bool cdp_record_is_private    (cdpRecord* record)  {assert(record);  return (record->metadata.attribute & CDP_ATTRIB_PRIVATE) != 0;}
static inline bool cdp_record_has_shadows   (cdpRecord* record)  {assert(record);  return (record->metadata.attribute & CDP_ATTRIB_SHADOWED) != 0;}


// Parent properties
#define CDP_PARENTEX(children)          ({assert(children);  (cdpParentEx*)(children);})
#define cdp_record_parent_ex(record)    CDP_PARENTEX((record)->storage)
static inline cdpRecord* cdp_record_parent  (cdpRecord* record)  {assert(record);  return CDP_EXPECT(record->storage != NULL)? cdp_record_parent_ex(record)->book: NULL;}
static inline size_t     cdp_record_siblings(cdpRecord* record)  {assert(record);  return CDP_EXPECT(record->storage != NULL)? cdp_record_parent_ex(record)->chdCount: 0;}


// Register property check
static inline bool cdp_record_register_is_borrowed(cdpRecord* reg)  {assert(cdp_record_is_register(reg));  return (reg->metadata.storeTech == CDP_STO_REG_BORROWED);}

// Book/Dictionary property check
static inline size_t cdp_record_book_or_dic_children(cdpRecord* book)   {assert(cdp_record_is_book(book));  return CDP_PARENTEX(book->recData.book.children)->chdCount;}
static inline bool   cdp_record_book_pushable       (cdpRecord* book)   {assert(cdp_record_is_book(book));  return (book->metadata.storeTech != CDP_STO_CHD_RED_BLACK_T);}


// Appends/inserts (or pushes) a new record.
cdpRecord* cdp_record_create(cdpRecord* parent, unsigned style, cdpID id, uint32_t type, bool prepend, bool priv, ...);

#define cdp_record_add_register(parent, id, type, borrow, data, size)             cdp_record_create(parent, CDP_REC_STYLE_REGISTER, id, type, false, false, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_record_prepend_register(parent, id, type, borrow, data, size)         cdp_record_create(parent, CDP_REC_STYLE_REGISTER, id, type,  true, false, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_record_add_register_priv(parent, id, type, borrow, data, size)        cdp_record_create(parent, CDP_REC_STYLE_REGISTER, id, type, false,  true, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_record_prepend_register_priv(parent, id, type, borrow, data, size)    cdp_record_create(parent, CDP_REC_STYLE_REGISTER, id, type,  true,  true, ((unsigned)(borrow)), data, ((size_t)(size)))

#define cdp_record_add_book(parent, id, type, chdStorage, ...)                    cdp_record_create(parent, CDP_REC_STYLE_BOOK, id, type, false, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_prepend_book(parent, id, type, chdStorage, ...)                cdp_record_create(parent, CDP_REC_STYLE_BOOK, id, type,  true, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_add_book_priv(parent, id, type, chdStorage, ...)               cdp_record_create(parent, CDP_REC_STYLE_BOOK, id, type, false,  true, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_prepend_book_priv(parent, id, type, chdStorage, ...)           cdp_record_create(parent, CDP_REC_STYLE_BOOK, id, type,  true,  true, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_record_add_dictionary(parent, id, type, chdStorage, cmp, p, ...)          cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, id, type, false, false, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)
#define cdp_record_prepend_dictionary(parent, id, type, chdStorage, cmp, p, ...)      cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, id, type,  true, false, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)
#define cdp_record_add_dictionary_priv(parent, id, type, chdStorage, cmp, p, ...)     cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, id, type, false,  true, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)
#define cdp_record_prepend_dictionary_priv(parent, id, type, chdStorage, cmp, p, ...) cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, id, type,  true,  true, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)


// Root dictionary operations.
static inline cdpRecord* cdp_record_root(void)  {extern cdpRecord ROOT; assert(ROOT.recData.book.children);  return &ROOT;}
#define cdp_record_root_add_book(id, type, chdStorage, ...)           cdp_record_add_book(cdp_record_root(), id, type, chdStorage, ##__VA_ARGS__)
#define cdp_record_root_add_dictionary(id, type, chdStorage, ...)     cdp_record_add_dictionary(cdp_record_root(), id, type, chdStorage, ##__VA_ARGS__)


// Constructs the full path (sequence of ids) for a given record, returning the depth.
bool cdp_record_path(cdpRecord* record, cdpPath** path);


// Accessing registers
void* cdp_record_register_read (cdpRecord* reg, size_t position, void* data, size_t* size);        // Reads register data from position and puts it on data buffer (atomically).
void* cdp_record_register_write(cdpRecord* reg, size_t position, const void* data, size_t size);   // Writes the data of a register record at position (atomically and it may reallocate memory).
#define cdp_record_register_update(reg, data, size)   cdp_record_register_write(reg, 0, data, size)


// Accessing books
cdpRecord* cdp_record_top    (cdpRecord* book, bool last);          // Gets the first (or last) record from book.
cdpRecord* cdp_record_by_name (cdpRecord* book, cdpID id);  // Retrieves a child record by its id.
cdpRecord* cdp_record_by_key  (cdpRecord* book, cdpRecord* key);    // Finds a child record based on specified key.
cdpRecord* cdp_record_by_index(cdpRecord* book, size_t index);      // Gets the child record at index position from book.
cdpRecord* cdp_record_by_path (cdpRecord* start, const cdpPath* path);  // Finds a child record based on a path of ids starting from the root or a given book.

cdpRecord* cdp_record_prev(cdpRecord* book, cdpRecord* record);     // Retrieves the previous sibling of record (sorted or unsorted).
cdpRecord* cdp_record_next(cdpRecord* book, cdpRecord* record);     // Retrieves the next sibling of record (sorted or unsorted).
cdpRecord* cdp_record_next_by_name(cdpRecord* book, cdpID id, uintptr_t* prev);   // Retrieves the first/next (unsorted) child record by its id.
cdpRecord* cdp_record_next_by_path(cdpRecord* start, cdpPath* path, uintptr_t* prev);    // Finds the first/next (unsorted) record based on a path of ids starting from the root or a given book.

bool cdp_record_traverse     (cdpRecord* book, cdpRecordTraverse func, void* context);    // Traverses the children of a book record, applying a function to each.
bool cdp_record_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, cdpRecordTraverse listEnd, void* context);  // Traverses each sub-branch of a book record.

// Converts an unsorted book into a dictionary.
void cdp_record_sort(cdpRecord* book, cdpCompare compare, void* context);


// Removing records
bool cdp_record_remove(cdpRecord* record, unsigned maxDepth);           // Deletes a record and all its children re-organizing sibling storage.
#define cdp_record_remove_register(reg)   cdp_record_remove(reg, 1)
size_t cdp_record_book_reset(cdpRecord* book, unsigned maxDepth);       // Deletes all children of a book or dictionary.


// To manage concurrent access to records safely.
bool cdp_record_lock(cdpRecord* record);
bool cdp_record_unlock(cdpRecord* record);

// Moves a record from one book to another.
bool cdp_record_move(cdpRecord* record, cdpRecord* newParent);

// Creates a duplicate of a record and adds/inserts it (or pushes it) to a book under id, optionally including its children.
cdpRecord* cdp_record_add_copy(const cdpRecord* book, cdpID id, cdpRecord* record, bool includeChildren, bool push);

// Creates a new link to a record and adds/inserts it (or pushes it) to a book under id, updating the source record parent list as necessary.
cdpRecord* cdp_record_add_link(cdpRecord* book, cdpID id, cdpRecord* record, bool push);


// Initiate and shutdown record system.
void cdp_record_system_initiate(void);
void cdp_record_system_shutdown(void);


/*
    TODO:
    - Rename id to ID and use CDP_ATTRIB_NAMED + autoincrement.
    - Perhaps separate dict and catalog, and redefine user callback based main ops.
    - Change "index" to "position" in the API.
    - Add book/dict properties dict to cdpVariantBook.
    - Rename API functions.
    - Traverse should use a user-provided entry struct.
    - Add find by register value function.
    - Add indexof for records;
    - Perhaps ids should be an unsorted tree (instead of a log) and use the deep-traverse index.
*/


#endif
