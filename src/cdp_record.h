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

      Collector Records: Records that can only grow in size/members, but
      they never lose data.

      Factual Records: Records that are no longer a state but a fact,
      that means that they can't be modified anymore so they only convey
      value.

*/


#include "cdp_util.h"


typedef struct _cdpRecord       cdpRecord;
typedef struct _cdpPath         cdpPath;


/*
 * Record Metadata
 */

#define CDP_ATTRIB_PRIVATE      0x01    // Record (with all its children) is private (unlockable).
#define CDP_ATTRIB_COLLECTOR    0x02    // Record can only grow (but never lose data).
#define CDP_ATTRIB_FACTUAL      0x04    // Record can't be modified anymore (but it still can be deleted).

#define CDP_ATTRIB_PUB_MASK     (CDP_ATTRIB_PRIVATE | CDP_ATTRIB_COLLECTOR | CDP_ATTRIB_FACTUAL)

#define CDP_ATTRIB_SHADOWED     0x08    // Record has shadow records (links pointing to it).
#define CDP_ATTRIB_RESERVED     0x10    // This flag is reserved for future use.

#define CDP_ATTRIB_BIT_COUNT    5


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


// Primal types:
enum _CDP_TYPE_PRIMAL {
    CDP_TYPE_NONE,              // This is the "nothing" type.

    CDP_TYPE_BOOK,
    CDP_TYPE_REGISTER,
    CDP_TYPE_LINK,

    CDP_TYPE_PRIMAL_COUNT
};

// Initial type ID:
enum _CDP_TYPE {
    // Book types
    CDP_TYPE_LIST = CDP_TYPE_PRIMAL_COUNT,
    CDP_TYPE_QUEUE,
    CDP_TYPE_STACK,
    //
    CDP_TYPE_DICTIONARY,
    CDP_TYPE_CATALOG,

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

#define CDP_TYPE_MAX_ID     (((uint32_t)(-1)) >> (CDP_ATTRIB_BIT_COUNT + 4))


// Initial name ID:
#define CDP_NAME_NAME           -1
#define CDP_NAME_VALUE          -2
#define CDP_NAME_SIZE           -3
#define CDP_NAME_DESCRIPTION    -4

#define CDP_NAME_ROOT           -5
#define CDP_NAME_TYPE           -6
#define CDP_NAME_SYSTEM         -7
#define CDP_NAME_USER           -8
#define CDP_NAME_PRIVATE        -9
#define CDP_NAME_PUBLIC         -10
#define CDP_NAME_DATA           -11
#define CDP_NAME_SERVICE        -12
#define CDP_NAME_PROCESS        -13
#define CDP_NAME_NETWORK        -14
#define CDP_NAME_TEMP           -15

#define CDP_NAME_COUNT          15


typedef int32_t cdpID;

typedef struct {
    uint32_t  attribute: CDP_ATTRIB_BIT_COUNT,              // Flags for record attributes.
              primal:    2,                                 // Primal type (book, register, link).
              storeTech: 2,                                 // Record storage technique (it depends on the primal type).
              type:      32 - (CDP_ATTRIB_BIT_COUNT + 4);   // Type tag for this record. It includes _CDP_TYPE + user defined types.
    cdpID     id;                                           // Name/field identifier of this record with respect to the parent record (if negative is a system-wide name ID).
} cdpMetadata;


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
    cdpMetadata metadata;   // Metadata including attributes, type and id.
    cdpRecData  recData;    // Data, either a book, a register or a link.
    void*       store;      // Pointer to the parent's storage structure (List, Array, Queue, RB-Tree).
};


/*
 * Children Storage Structs
 */

typedef struct {
    size_t      count;            // Number of record pointers
    cdpRecord*  record[];         // Dynamic array of (local) links shadowing this one.
} cdpShadow;

typedef int (*cdpCompare)(const cdpRecord* restrict, const cdpRecord* restrict, void*);

typedef struct {
    cdpCompare  compare;          // Compare function.
    void*       context;          // User defined context data for searches.
} cdpSorter;

typedef struct {
    cdpRecord*  book;             // Parent book owning this child storage.
    cdpShadow*  shadow;           // Pointer to a structure for managing multiple (linked) parents.
    size_t      chdCount;         // Number of child records.
    cdpSorter*  sorter;           // Used for sorting catalogs.
} cdpChdStore;


/*
 * Other C Data Types
 */

struct _cdpPath {
    unsigned    length;
    unsigned    capacity;
    cdpID       id[];
};

typedef struct {
    cdpRecord*  record;
    cdpRecord*  parent;
    cdpRecord*  next;
    cdpRecord*  prev;
    size_t      position;
} cdpBookEntry;

typedef bool (*cdpRecordTraverse)(cdpBookEntry*, unsigned, void*);


/*
 * Record Operations
 */

bool cdp_record_initialize(cdpRecord* record, unsigned primal, unsigned attrib, cdpID id, uint32_t type, ...);
void cdp_record_finalize(cdpRecord* record, unsigned maxDepth);

// General property check
static inline bool cdp_record_is_none       (const cdpRecord* record)  {assert(record);  return (record->metadata.primal == CDP_TYPE_NONE);}
static inline bool cdp_record_is_book       (const cdpRecord* record)  {assert(record);  return (record->metadata.primal == CDP_TYPE_BOOK);}
static inline bool cdp_record_is_register   (const cdpRecord* record)  {assert(record);  return (record->metadata.primal == CDP_TYPE_REGISTER);}
static inline bool cdp_record_is_link       (const cdpRecord* record)  {assert(record);  return (record->metadata.primal == CDP_TYPE_LINK);}
static inline bool cdp_record_is_named      (const cdpRecord* record)  {assert(record);  return (record->metadata.id < 0);}
static inline bool cdp_record_is_private    (const cdpRecord* record)  {assert(record);  return cdp_is_set(record->metadata.attribute, CDP_ATTRIB_PRIVATE);}
static inline bool cdp_record_is_collector  (const cdpRecord* record)  {assert(record);  return cdp_is_set(record->metadata.attribute, CDP_ATTRIB_COLLECTOR);}
static inline bool cdp_record_is_factual    (const cdpRecord* record)  {assert(record);  return cdp_is_set(record->metadata.attribute, CDP_ATTRIB_FACTUAL);}
static inline bool cdp_record_is_shadowed   (const cdpRecord* record)  {assert(record);  return cdp_is_set(record->metadata.attribute, CDP_ATTRIB_SHADOWED);}
static inline bool cdp_record_is_dictionary (const cdpRecord* record)  {assert(record);  return (cdp_record_is_book(record) && record->metadata.type == CDP_TYPE_DICTIONARY);}
static inline bool cdp_record_is_catalog    (const cdpRecord* record)  {assert(record);  return (cdp_record_is_book(record) && record->metadata.type == CDP_TYPE_CATALOG);}
static inline bool cdp_record_is_dict_or_cat(const cdpRecord* record)  {assert(record);  return (cdp_record_is_book(record) && (record->metadata.type == CDP_TYPE_DICTIONARY || record->metadata.type == CDP_TYPE_CATALOG));}


// Parent properties
#define CDP_CHD_STORE(children)         ({assert(children);  (cdpChdStore*)(children);})
#define cdp_record_par_store(record)    CDP_CHD_STORE((record)->store)
static inline cdpRecord* cdp_record_parent  (const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->book: NULL;}
static inline size_t     cdp_record_siblings(const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->chdCount: 0;}


// Register property check
static inline bool cdp_register_is_borrowed(const cdpRecord* reg)   {assert(cdp_record_is_register(reg));  return (reg->metadata.storeTech == CDP_STO_REG_BORROWED);}

// Book property check
static inline size_t cdp_book_children(const cdpRecord* book)       {assert(cdp_record_is_book(book));  return CDP_CHD_STORE(book->recData.book.children)->chdCount;}
static inline bool   cdp_book_is_prependable(const cdpRecord* book) {assert(cdp_record_is_book(book));  return (book->metadata.storeTech != CDP_STO_CHD_RED_BLACK_T);}


// Appends, inserts or prepends a copy of record into a book.
cdpRecord* cdp_book_add_record(cdpRecord* book, cdpRecord* record, bool prepend);
#define cdp_book_add(b, primal, attribute, id, type, prepend, ...)  ({cdpRecord r={0}; cdp_record_initialize(&r, primal, attribute, id, type, ##__VA_ARGS__)? cdp_book_add_record(b, &r, prepend): NULL;})
#define cdp_catalog_add(cat, record)    cdp_book_add_record(cat, record, false)

#define cdp_book_add_register(b, attrib, id, type, borrow, data, size)          cdp_book_add(b, CDP_TYPE_REGISTER, attrib, id, type, false, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_book_prepend_register(b, attrib, id, type, borrow, data, size)      cdp_book_add(b, CDP_TYPE_REGISTER, attrib, id, type,  true, ((unsigned)(borrow)), data, ((size_t)(size)))

static inline cdpRecord* cdp_book_add_text(cdpRecord* book, cdpID id, const char* text)  {assert(cdp_record_is_book(book) && text && *text);  return cdp_book_add_register(book, 0, id, CDP_TYPE_UTF8, false, text, strlen(text));}
#define cdp_book_add_static_text(b, id, text)   cdp_book_add_register(b, CDP_ATTRIB_FACTUAL, id, CDP_TYPE_UTF8, true, text, strlen(text));

#define CDP_FUNC_ADD_VAL_(func, ctype, rtype)                                  \
    static inline cdpRecord* cdp_book_add_##func(cdpRecord* book, cdpID id, ctype value)    {assert(cdp_record_is_book(book));  return cdp_book_add_register(book, 0, id, rtype, false, &value, sizeof(value));}\
    static inline cdpRecord* cdp_book_prepend_##func(cdpRecord* book, cdpID id, ctype value){assert(cdp_book_is_prependable(book));  return cdp_book_prepend_register(book, 0, id, rtype, false, &value, sizeof(value));}

    CDP_FUNC_ADD_VAL_(boolean, uint8_t,  CDP_TYPE_BOOLEAN)
    CDP_FUNC_ADD_VAL_(byte,    uint8_t,  CDP_TYPE_BYTE)
    CDP_FUNC_ADD_VAL_(uint16,  uint16_t, CDP_TYPE_UINT16)
    CDP_FUNC_ADD_VAL_(uint32,  uint32_t, CDP_TYPE_UINT32)
    CDP_FUNC_ADD_VAL_(uint64,  uint64_t, CDP_TYPE_UINT64)
    CDP_FUNC_ADD_VAL_(int16,   int16_t,  CDP_TYPE_INT16)
    CDP_FUNC_ADD_VAL_(int32,   int32_t,  CDP_TYPE_INT32)
    CDP_FUNC_ADD_VAL_(int64,   int64_t,  CDP_TYPE_INT64)
    CDP_FUNC_ADD_VAL_(float32, float,    CDP_TYPE_FLOAT32)
    CDP_FUNC_ADD_VAL_(float64, double,   CDP_TYPE_FLOAT64)


#define cdp_book_add_book(b, id, type, chdStorage, ...)             cdp_book_add(b, CDP_TYPE_BOOK, 0, id, type, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_book(b, type, id, chdStorage, ...)         cdp_book_add(b, CDP_TYPE_BOOK, 0, id, type,  true, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_book_add_list(b, id, chdStorage, ...)                   cdp_book_add_book(b, id, CDP_TYPE_LIST,       ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_set(b, id, chdStorage, ...)                    cdp_book_add_book(b, id, CDP_TYPE_SET,        ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_queue(b, id, chdStorage, ...)                  cdp_book_add_book(b, id, CDP_TYPE_QUEUE,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_dictionary(b, id, chdStorage, ...)             cdp_book_add_book(b, id, CDP_TYPE_DICTIONARY, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_catalog(b, id, chdStorage, cmp, p, ...)        cdp_book_add_book(b, id, CDP_TYPE_CATALOG,    ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)
#define cdp_book_prepend_list(b, id, chdStorage, ...)               cdp_book_prepend_book(b, id, CDP_TYPE_LIST,       ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_set(b, id, chdStorage, ...)                cdp_book_prepend_book(b, id, CDP_TYPE_SET,        ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_queue(b, id, chdStorage, ...)              cdp_book_prepend_book(b, id, CDP_TYPE_QUEUE,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_dictionary(b, id, chdStorage, ...)         cdp_book_prepend_book(b, id, CDP_TYPE_DICTIONARY, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_catalog(b, id, chdStorage, cmp, p, ...)    cdp_book_prepend_book(b, id, CDP_TYPE_CATALOG,    ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)


// Root dictionary.
static inline cdpRecord* cdp_root(void)  {extern cdpRecord ROOT; assert(ROOT.recData.book.children);  return &ROOT;}


// Constructs the full path (sequence of ids) for a given record, returning the depth.
bool cdp_record_path(const cdpRecord* record, cdpPath** path);


// Accessing registers
void* cdp_register_read(const cdpRecord* reg, size_t position, void* data, size_t* size);   // Reads register data from position and puts it on data buffer (atomically).
void* cdp_register_write(cdpRecord* reg, size_t position, const void* data, size_t size);   // Writes the data of a register record at position (atomically and it may reallocate memory).
#define cdp_register_update(reg, data, size)   cdp_register_write(reg, 0, data, size)

#define cdp_register_read_bool(reg)     (*(uint8_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_byte(reg)     (*(uint8_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_uint16(reg)   (*(uint16_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_uint32(reg)   (*(uint32_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_uint64(reg)   (*(uint64_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_int16(reg)    (*(int16_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_int32(reg)    (*(int32_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_int64(reg)    (*(int64_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_float32(reg)  (*(float*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_float64(reg)  (*(double*)cdp_register_read(reg, 0, NULL, NULL))

#define cdp_register_update_bool(reg, v)    cdp_register_update(reg, &(v), sizeof(uint8_t))
#define cdp_register_update_byte(reg, v)    cdp_register_update(reg, &(v), sizeof(uint8_t))
#define cdp_register_update_uint16(reg, v)  cdp_register_update(reg, &(v), sizeof(uint16_t))
#define cdp_register_update_uint32(reg, v)  cdp_register_update(reg, &(v), sizeof(uint32_t))
#define cdp_register_update_uint64(reg, v)  cdp_register_update(reg, &(v), sizeof(uint64_t))
#define cdp_register_update_int16(reg, v)   cdp_register_update(reg, &(v), sizeof(int16_t))
#define cdp_register_update_int32(reg, v)   cdp_register_update(reg, &(v), sizeof(int32_t))
#define cdp_register_update_int64(reg, v)   cdp_register_update(reg, &(v), sizeof(int64_t))
#define cdp_register_update_float32(reg, v) cdp_register_update(reg, &(v), sizeof(float))
#define cdp_register_update_float64(reg, v) cdp_register_update(reg, &(v), sizeof(double))


// Accessing books
cdpRecord* cdp_book_first(const cdpRecord* book);   // Gets the first record from book.
cdpRecord* cdp_book_last (const cdpRecord* book);   // Gets the last record from book.

cdpRecord* cdp_book_find_by_name (const cdpRecord* book, cdpID id);             // Retrieves a child record by its id.
cdpRecord* cdp_book_find_by_key  (const cdpRecord* book, cdpRecord* key);       // Finds a child record based on specified key record.
cdpRecord* cdp_book_find_by_position(const cdpRecord* book, size_t pos);        // Gets the child record at index position from book.
cdpRecord* cdp_book_find_by_path (const cdpRecord* start, const cdpPath* path); // Finds a child record based on a path of ids starting from the root or a given book.

cdpRecord* cdp_book_prev(const cdpRecord* book, cdpRecord* record);     // Retrieves the previous sibling of record (sorted or unsorted).
cdpRecord* cdp_book_next(const cdpRecord* book, cdpRecord* record);     // Retrieves the next sibling of record (sorted or unsorted).

cdpRecord* cdp_book_next_by_name(const cdpRecord* book, cdpID id, uintptr_t* prev);         // Retrieves the first/next (unsorted) child record by its id.
cdpRecord* cdp_book_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev);   // Finds the first/next (unsorted) record based on a path of ids starting from the root or a given book.

bool cdp_book_traverse     (cdpRecord* book, cdpRecordTraverse func, void* context, cdpBookEntry* entry);  // Traverses the children of a book record, applying a function to each.
bool cdp_book_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpRecordTraverse func, cdpRecordTraverse listEnd, void* context, cdpBookEntry* entry);  // Traverses each sub-branch of a book record.

// Converts an unsorted book into a sorted one.
void cdp_book_to_dictionary(cdpRecord* book);
void cdp_book_to_catalog(cdpRecord* book, cdpCompare compare, void* context);


// Removing records
bool cdp_record_remove(cdpRecord* record, unsigned maxDepth);   // Deletes a record and all its children re-organizing sibling storage.
#define cdp_register_remove(reg)   cdp_record_remove(reg, 1)
#define cdp_book_remove(book)      cdp_record_remove(book, 64)  /* FixMe: compute maxDepth */
size_t cdp_book_reset(cdpRecord* book, unsigned maxDepth);      // Deletes all children of a book or dictionary.


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
    - Implement auto-increment in books.
    - Add indexof for records;
    - Redefine user callback based on typed book ops.
    - Add book properties dict to cdpVariantBook.
    - Perhaps ids should be an unsorted tree (instead of a log) and use the deep-traverse index.
    - Fully define the tree (nesting) recursion limit policy.
*/


#endif
