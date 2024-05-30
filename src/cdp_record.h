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
    within objector cache lines to enhance performance. It supports
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
      a fixed value.

*/


#include "cdp_util.h"


typedef struct _cdpRecord       cdpRecord;
typedef struct _cdpPath         cdpPath;


/*
 * Record Metadata
 */

#define CDP_ATTRIB_PRIVATE      0x01    // Record (with all its children) is private (unlockable).
#define CDP_ATTRIB_FACTUAL      0x02    // Record can't be modified anymore (but it still can be deleted).

#define CDP_ATTRIB_PUB_MASK     (CDP_ATTRIB_PRIVATE | CDP_ATTRIB_FACTUAL)

#define CDP_ATTRIB_SHADOWED     0x04    // Record has shadow records (links pointing to it).
#define CDP_ATTRIB_RESERVED     0x08    // For future use.

#define CDP_ATTRIB_BIT_COUNT    4


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


typedef uint32_t cdpID;

#define CDP_ID_MAXVAL         ((cdpID)(-1))

#define CDP_META_BITS         (CDP_ATTRIB_BIT_COUNT + 4)
#define CDP_TYPE_MAXVAL       (CDP_ID_MAXVAL >> CDP_META_BITS)
#define CDP_TYPE_COUNT_MAX    (CDP_TYPE_MAXVAL >> 1)

// Primal types:
enum _cdpTypePrimal {
    CDP_TYPE_VOID,              // This is the "nothing" type.

    CDP_TYPE_BOOK,
    CDP_TYPE_REGISTER,
    CDP_TYPE_LINK,

    CDP_TYPE_PRIMAL_COUNT
};

// Initial type IDs (for a description see cdp_object.c):
enum _cdpTypeID {
    // Book types
    CDP_TYPE_LIST = CDP_TYPE_PRIMAL_COUNT,
    CDP_TYPE_QUEUE,
    CDP_TYPE_STACK,
    //
    CDP_TYPE_DICTIONARY,

    // Register types
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
    //
    CDP_TYPE_ID,
    CDP_TYPE_NAME_ID,
    CDP_TYPE_UTF8,
    CDP_TYPE_PATCH,
    //
    CDP_TYPE_EXECUTABLE,
    CDP_TYPE_EVENT,

    // Structured types
    CDP_TYPE_TYPE,
    CDP_OBJECT_OBJECT,

    CDP_TYPE_COUNT
};

#define CDP_AUTO_ID           CDP_ID_MAXVAL
#define CDP_AUTO_ID_MAX       (CDP_ID_MAXVAL >> 1)

#define CDP_NAME_FLAG         (~(CDP_ID_MAXVAL >> 1))
#define CDP_TEXT2ID(text)     (CDP_NAME_FLAG | (text))
#define CDP_ID2TEXT(id)       ((id) & (~CDP_NAME_FLAG))
#define CDP_NAME_COUNT_MAX    (CDP_AUTO_ID - 1)

// Initial name IDs:
enum _cdpNameID {
    CDP_NAME_VOID = CDP_NAME_FLAG,
    //
    CDP_NAME_NAME,
    CDP_NAME_VALUE,
    CDP_NAME_SIZE,
    CDP_NAME_DESCRIPTION,
    CDP_NAME_OBJECT,
    CDP_NAME_PRIVATE,
    //CDP_NAME_SERVICE,
    //
    CDP_NAME_ROOT,
    CDP_NAME_TYPE,
    CDP_NAME_SYSTEM,
    CDP_NAME_USER,
    CDP_NAME_PUBLIC,
    CDP_NAME_DATA,
    CDP_NAME_NETWORK,
    CDP_NAME_TEMP,

    CDP_NAME_FLAG_COUNT
};

#define CDP_NAME_COUNT  (CDP_NAME_FLAG_COUNT - CDP_NAME_VOID)


typedef struct {
    cdpID attribute: CDP_ATTRIB_BIT_COUNT,              // Flags for record attributes.
          primal:    2,                                 // Primal type (book, register, link).
          storeTech: 2,                                 // Record storage technique (it depends on the primal type).
          type:      cdp_bitsof(cdpID) - CDP_META_BITS; // Type tag for this record. It includes _cdpTypeID + user defined types.
    cdpID id;                                           // Name/field identifier of this record with respect to the parent record (if negative is a system-wide name ID).
} cdpMetadata;


/*
 * Record Data
 */

typedef struct {
    void*     children;     // Pointer to either cdpArray, cdpList, cdpPackedQ, or cdpRbTree.
    void*     property;     // Book property dictionary (as cdpRbTree).
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
    size_t      count;      // Number of record pointers
    cdpRecord*  record[];   // Dynamic array of (local) links shadowing this one.
} cdpShadow;

typedef int (*cdpCompare)(const cdpRecord* restrict, const cdpRecord* restrict, void*);

typedef struct {
    cdpRecord*  book;       // Parent book owning this child storage.
    cdpShadow*  shadow;     // Pointer to a structure for managing multiple (linked) parents.
    size_t      chdCount;   // Number of child records.
    cdpID       autoID;     // Auto-increment ID for naming contained records.
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

typedef bool (*cdpTraverse)(cdpBookEntry*, unsigned, void*);

typedef bool (*cdpObject)(cdpRecord* instance, cdpID signal, cdpRecord* );


/*
 * Record Operations
 */

bool cdp_record_initialize(cdpRecord* record, unsigned primal, unsigned attrib, cdpID id, uint32_t type, ...);
void cdp_record_finalize(cdpRecord* record, unsigned maxDepth);

#define cdp_record_initialize_list(r, id, chdStorage, ...)        cdp_record_initialize(r, CDP_TYPE_BOOK, 0, id, CDP_TYPE_LIST,       ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_initialize_queue(r, id, chdStorage, ...)       cdp_record_initialize(r, CDP_TYPE_BOOK, 0, id, CDP_TYPE_QUEUE,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_initialize_stack(r, id, chdStorage, ...)       cdp_record_initialize(r, CDP_TYPE_BOOK, 0, id, CDP_TYPE_STACK,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_initialize_dictionary(r, id, chdStorage, ...)  cdp_record_initialize(r, CDP_TYPE_BOOK, 0, id, CDP_TYPE_DICTIONARY, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define CDP_RECORD_SET_ATRIBUTE(r, attrib)     ((r)->metadata.attrib |= (attrib))

// General property check
static inline cdpID cdp_record_attributes(const cdpRecord* record)  {assert(record);  return record->metadata.attribute;}
static inline cdpID cdp_record_primal    (const cdpRecord* record)  {assert(record);  return record->metadata.primal;}
static inline cdpID cdp_record_id        (const cdpRecord* record)  {assert(record);  return record->metadata.id;}
static inline cdpID cdp_record_type      (const cdpRecord* record)  {assert(record);  return record->metadata.type;}

#define cdp_record_is_void(r)       (cdp_record_primal(r) == CDP_TYPE_VOID)
#define cdp_record_is_book(r)       (cdp_record_primal(r) == CDP_TYPE_BOOK)
#define cdp_record_is_register(r)   (cdp_record_primal(r) == CDP_TYPE_REGISTER)
#define cdp_record_is_link(r)       (cdp_record_primal(r) == CDP_TYPE_LINK)

#define cdp_record_is_private(r)    cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_PRIVATE)
#define cdp_record_is_factual(r)    cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_FACTUAL)
#define cdp_record_is_shadowed(r)   cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_SHADOWED)

static inline bool cdp_record_is_named     (const cdpRecord* record)  {assert(record);  if (record->metadata.id & CDP_NAME_FLAG) {assert(CDP_ID2TEXT(record->metadata.id) <= CDP_NAME_COUNT_MAX); return true;} return false;}
static inline bool cdp_record_is_object    (const cdpRecord* record)  {assert(record);  if (record->metadata.type & CDP_OBJECT_FLAG) {assert(CDP_TYPE2OBJ(record->metadata.type) <= CDP_OBJECT_COUNT_MAX); return true;} return false;}
static inline bool cdp_record_is_dictionary(const cdpRecord* record)  {assert(record);  return (cdp_record_is_book(record) && record->metadata.type == CDP_TYPE_DICTIONARY);}

#define cdp_record_id_is_auto(r)    (cdp_record_id(r) < CDP_NAME_FLAG)
#define cdp_record_id_is_pending(r) (cdp_record_id(r) == CDP_AUTO_ID)


// Parent properties
#define CDP_CHD_STORE(children)         ({assert(children);  (cdpChdStore*)(children);})
#define cdp_record_par_store(record)    CDP_CHD_STORE((record)->store)
static inline cdpRecord* cdp_record_parent  (const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->book: NULL;}
static inline size_t     cdp_record_siblings(const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->chdCount: 0;}


// Register properties
static inline bool   cdp_register_is_borrowed(const cdpRecord* reg) {assert(cdp_record_is_register(reg));  return (reg->metadata.storeTech == CDP_STO_REG_BORROWED);}
static inline size_t cdp_register_size(const cdpRecord* reg)        {assert(cdp_record_is_register(reg));  return reg->recData.reg.size;}

// Book properties
static inline size_t cdp_book_children(const cdpRecord* book)       {assert(cdp_record_is_book(book));  return CDP_CHD_STORE(book->recData.book.children)->chdCount;}
static inline bool   cdp_book_is_prependable(const cdpRecord* book) {assert(cdp_record_is_book(book));  return (book->metadata.storeTech != CDP_STO_CHD_RED_BLACK_T);}

static inline cdpID cdp_book_get_auto_id(const cdpRecord* book)           {assert(cdp_record_is_book(book));  return CDP_CHD_STORE(book->recData.book.children)->autoID;}
static inline void  cdp_book_set_auto_id(const cdpRecord* book, cdpID id) {assert(cdp_record_is_book(book));  cdpChdStore* store = CDP_CHD_STORE(book->recData.book.children); assert(store->autoID < id); store->autoID = id;}

cdpRecord* cdp_book_add_property(cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_book_get_property(const cdpRecord* book, cdpID id);


// Appends, inserts or prepends a copy of record into a book.
cdpRecord* cdp_book_add_record(cdpRecord* book, cdpRecord* record, bool prepend);
cdpRecord* cdp_book_sorted_insert(cdpRecord* book, cdpRecord* record, cdpCompare compare, void* context);

#define cdp_book_add(b, primal, attribute, id, type, prepend, ...)  ({cdpRecord r={0}; cdp_record_initialize(&r, primal, attribute, id, type, ##__VA_ARGS__)? cdp_book_add_record(b, &r, prepend): NULL;})
#define cdp_book_add_register(b, attrib, id, type, borrow, data, size)          cdp_book_add(b, CDP_TYPE_REGISTER, attrib, id, type, false, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_book_prepend_register(b, attrib, id, type, borrow, data, size)      cdp_book_add(b, CDP_TYPE_REGISTER, attrib, id, type,  true, ((unsigned)(borrow)), data, ((size_t)(size)))

static inline cdpRecord* cdp_book_add_text(cdpRecord* book, unsigned attrib, cdpID id, bool borrow, const char* text)    {assert(cdp_record_is_book(book) && text && *text);  cdpRecord* reg = cdp_book_add_register(book, attrib, id, CDP_TYPE_UTF8, borrow, text, strlen(text) + 1); reg->recData.reg.size--; return reg;}
#define cdp_book_add_static_text(b, id, text)   cdp_book_add_text(b, CDP_ATTRIB_FACTUAL, id, true, text)

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

    CDP_FUNC_ADD_VAL_(id, cdpID, CDP_TYPE_ID)
    CDP_FUNC_ADD_VAL_(object, cdpObject, CDP_TYPE_EXECUTABLE)


#define cdp_book_add_book(b, id, type, chdStorage, ...)             cdp_book_add(b, CDP_TYPE_BOOK, 0, id, type, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_book(b, type, id, chdStorage, ...)         cdp_book_add(b, CDP_TYPE_BOOK, 0, id, type,  true, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_book_add_list(b, id, chdStorage, ...)                   cdp_book_add_book(b, id, CDP_TYPE_LIST,       ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_queue(b, id, chdStorage, ...)                  cdp_book_add_book(b, id, CDP_TYPE_QUEUE,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_stack(b, id, chdStorage, ...)                  cdp_book_add_book(b, id, CDP_TYPE_STACK,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_dictionary(b, id, chdStorage, ...)             cdp_book_add_book(b, id, CDP_TYPE_DICTIONARY, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_book_prepend_list(b, id, chdStorage, ...)               cdp_book_prepend_book(b, id, CDP_TYPE_LIST,       ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_queue(b, id, chdStorage, ...)              cdp_book_prepend_book(b, id, CDP_TYPE_QUEUE,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_stack(b, id, chdStorage, ...)              cdp_book_prepend_book(b, id, CDP_TYPE_STACK,      ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_dictionary(b, id, chdStorage, ...)         cdp_book_prepend_book(b, id, CDP_TYPE_DICTIONARY, ((unsigned)(chdStorage)), ##__VA_ARGS__)


// Root dictionary.
static inline cdpRecord* cdp_root(void)  {extern cdpRecord CDP_ROOT; assert(CDP_ROOT.recData.book.children);  return &CDP_ROOT;}


// Constructs the full path (sequence of ids) for a given record, returning the depth.
bool cdp_record_path(const cdpRecord* record, cdpPath** path);


// Accessing registers
void* cdp_register_read(const cdpRecord* reg, size_t position, void* data, size_t* size);   // Reads register data from position and puts it on data buffer (atomically).
void* cdp_register_write(cdpRecord* reg, size_t position, const void* data, size_t size);   // Writes the data of a register record at position (atomically and it may reallocate memory).
#define cdp_register_update(reg, data, size)   cdp_register_write(reg, 0, data, size)

#define cdp_register_read_bool(reg)     (*(uint8_t*)cdp_register_read(reg, 0, NULL, NULL))  // ToDo: use recData.reg.data.direct.
#define cdp_register_read_byte(reg)     (*(uint8_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_uint16(reg)   (*(uint16_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_uint32(reg)   (*(uint32_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_uint64(reg)   (*(uint64_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_int16(reg)    (*(int16_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_int32(reg)    (*(int32_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_int64(reg)    (*(int64_t*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_float32(reg)  (*(float*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_float64(reg)  (*(double*)cdp_register_read(reg, 0, NULL, NULL))

#define cdp_register_read_id(reg) (*(cdpID*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_utf8(reg) ((const char*)cdp_register_read(reg, 0, NULL, NULL))
#define cdp_register_read_executable(reg) ((cdpObject)cdp_register_read(reg, 0, NULL, NULL))

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

cdpRecord* cdp_book_find_by_name(const cdpRecord* book, cdpID id);             // Retrieves a child record by its id.
cdpRecord* cdp_book_find_by_key(const cdpRecord* book, cdpRecord* key, cdpCompare compare, void* context);  // Finds a child record based on specified key record.
cdpRecord* cdp_book_find_by_position(const cdpRecord* book, size_t pos);        // Gets the child record at index position from book.
cdpRecord* cdp_book_find_by_path(const cdpRecord* start, const cdpPath* path); // Finds a child record based on a path of ids starting from the root or a given book.

cdpRecord* cdp_book_prev(const cdpRecord* book, cdpRecord* record);     // Retrieves the previous sibling of record (sorted or unsorted).
cdpRecord* cdp_book_next(const cdpRecord* book, cdpRecord* record);     // Retrieves the next sibling of record (sorted or unsorted).

cdpRecord* cdp_book_next_by_name(const cdpRecord* book, cdpID id, uintptr_t* prev);         // Retrieves the first/next (unsorted) child record by its id.
cdpRecord* cdp_book_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev);   // Finds the first/next (unsorted) record based on a path of ids starting from the root or a given book.

bool cdp_book_traverse     (cdpRecord* book, cdpTraverse func, void* context, cdpBookEntry* entry);  // Traverses the children of a book record, applying a function to each.
bool cdp_book_deep_traverse(cdpRecord* book, unsigned maxDepth, cdpTraverse func, cdpTraverse listEnd, void* context, cdpBookEntry* entry);  // Traverses each sub-branch of a book record.

// Converts an unsorted book into a sorted one.
void cdp_book_to_dictionary(cdpRecord* book);


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
    - Traverse book in internal (stoTech) order.
    - Add indexof for records.
    - Use "recData.reg.data.direct" in registers.
    - Put move "depth" from traverse argument to inside entry structure.
    - Add cdp_book_update_nested_links(old, new).
    - CDP_NAME_VOID should never be a valid name for records.
    - If a record is added to a book with its name explicitelly above "auto_id", then it must be updated.
    - Redefine user callback based on typed book ops.
    - Implement cdp_book_insert_at(position).
    - Perhaps ids should be an unsorted tree (instead of a log) and use the deep-traverse index.
    - Fully define the tree (nesting) recursion limit policy.
*/


#endif
