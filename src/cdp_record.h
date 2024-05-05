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


#include "cdp_util.h"


typedef struct _cdpRecord       cdpRecord;
typedef struct _cdpPath         cdpPath;


/*
 * Record Metadata
 */
 
#define CDP_FLAG_PRIVATE    0x01    // Record and all its children are private (unlockables).
#define CDP_FLAG_SHADOWED   0x02    // Record has shadow records (links pointing to it).
#define CDP_FLAG_RESERVED   0x04    // This flag is for future use.

enum {
    CDP_REC_STYLE_BOOK,
    CDP_REC_STYLE_DICTIONARY,
    CDP_REC_STYLE_REGISTER,
    CDP_REC_STYLE_LINK,
    //
    CDP_REC_STYLE_COUNT
};

enum {
    CDP_STO_CHD_LINKED_LIST,    // Children stored in a doubly linked list.
    CDP_STO_CHD_CIRC_BUFFER,    // Children stored in a circular buffer.
    CDP_STO_CHD_ARRAY,          // Children stored in an array.
    CDP_STO_CHD_PACKED_LIST,    // Children stored in a packed list.
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

typedef uint32_t cdpNameID;

typedef struct {
    uint32_t  reStyle: 2,       // Style of the record (book, register or link)
              proFlag: 3,       // Flags for record properties (multiple parents, private, sorted).
              stoTech: 3,       // Record storage technique (it depends of the style of record).
              typeID: 24;       // Type identifier for the record.
    cdpNameID nameID;           // Name/field identifier in the parent record.
} cdpRecMeta;


/*
 * Record Data
 */

typedef struct {
    void*     children;     // Pointer to cdpArray, cdpList, cdpRbTree, etc.
    void*     reserved;     // Reserved for future use.
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
    cdpRecMeta  metadata;   // Metadata including flags and nameID.
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
    cdpNameID nameID[];
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
static inline bool cdp_record_is_book       (cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_BOOK);}
static inline bool cdp_record_is_dictionary (cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_DICTIONARY);}
static inline bool cdp_record_is_book_or_dic(cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_BOOK || record->metadata.reStyle == CDP_REC_STYLE_DICTIONARY);}
static inline bool cdp_record_is_register   (cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_REGISTER);}
static inline bool cdp_record_is_link       (cdpRecord* record)  {assert(record);  return (record->metadata.reStyle == CDP_REC_STYLE_LINK);}
static inline bool cdp_record_is_private    (cdpRecord* record)  {assert(record);  return (record->metadata.proFlag & CDP_FLAG_PRIVATE) != 0;}
static inline bool cdp_record_has_shadows   (cdpRecord* record)  {assert(record);  return (record->metadata.proFlag & CDP_FLAG_SHADOWED) != 0;}


// Parent properties
#define CDP_PARENTEX(children)          ({assert(children);  (cdpParentEx*)(children);})
#define cdp_record_parent_ex(record)    CDP_PARENTEX((record)->storage)
static inline cdpRecord* cdp_record_parent  (cdpRecord* record)  {assert(record);  return CDP_EXPECT(record->storage != NULL)? cdp_record_parent_ex(record)->book: NULL;}
static inline size_t     cdp_record_siblings(cdpRecord* record)  {assert(record);  return CDP_EXPECT(record->storage != NULL)? cdp_record_parent_ex(record)->chdCount: 0;}


// Register property check
static inline bool cdp_record_register_is_borrowed(cdpRecord* reg)  {assert(cdp_record_is_register(reg));  return (reg->metadata.stoTech == CDP_STO_REG_BORROWED);}

// Book/Dictionary property check
static inline size_t cdp_record_book_or_dic_children(cdpRecord* book)   {assert(cdp_record_is_book_or_dic(book));  return CDP_PARENTEX(book->recData.book.children)->chdCount;}
static inline bool   cdp_record_book_pushable       (cdpRecord* book)   {assert(cdp_record_is_book(book));  return (book->metadata.stoTech != CDP_STO_CHD_RED_BLACK_T);}


// Appends/inserts (or pushes) a new record.
cdpRecord* cdp_record_create(cdpRecord* parent, unsigned style, cdpNameID nameID, uint32_t typeID, bool push, bool priv, ...);

#define cdp_record_add_register(parent, nameID, typeID, borrow, data, size)         cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID, false, false, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_record_push_register(parent, nameID, typeID, borrow, data, size)        cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID,  true, false, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_record_add_register_priv(parent, nameID, typeID, borrow, data, size)    cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID, false,  true, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_record_push_register_priv(parent, nameID, typeID, borrow, data, size)   cdp_record_create(parent, CDP_REC_STYLE_REGISTER, nameID, typeID,  true,  true, ((unsigned)(borrow)), data, ((size_t)(size)))

#define cdp_record_add_book(parent, nameID, typeID, chdStorage, ...)                cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID, false, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_push_book(parent, nameID, typeID, chdStorage, ...)               cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID,  true, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_add_book_priv(parent, nameID, typeID, chdStorage, ...)           cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID, false,  true, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_push_book_priv(parent, nameID, typeID, chdStorage, ...)          cdp_record_create(parent, CDP_REC_STYLE_BOOK, nameID, typeID,  true,  true, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_record_add_dictionary(parent, nameID, typeID, chdStorage, cmp, p, ...)        cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, nameID, typeID, false, false, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)
#define cdp_record_push_dictionary(parent, nameID, typeID, chdStorage, cmp, p, ...)       cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, nameID, typeID,  true, false, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)
#define cdp_record_add_dictionary_priv(parent, nameID, typeID, chdStorage, cmp, p, ...)   cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, nameID, typeID, false,  true, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)
#define cdp_record_push_dictionary_priv(parent, nameID, typeID, chdStorage, cmp, p, ...)  cdp_record_create(parent, CDP_REC_STYLE_DICTIONARY, nameID, typeID,  true,  true, ((unsigned)(chdStorage)), cmp, p, ##__VA_ARGS__)


// Root dictionary operations.
static inline cdpRecord* cdp_record_root(void)  {extern cdpRecord ROOT; assert(ROOT.recData.book.children);  return &ROOT;}
#define cdp_record_root_add_book(nameID, typeID, chdStorage, ...)           cdp_record_add_book(cdp_record_root(), nameID, typeID, chdStorage, ##__VA_ARGS__)
#define cdp_record_root_add_dictionary(nameID, typeID, chdStorage, ...)     cdp_record_add_dictionary(cdp_record_root(), nameID, typeID, chdStorage, ##__VA_ARGS__)


// Constructs the full path (sequence of nameIDs) for a given record, returning the depth.
bool cdp_record_path(cdpRecord* record, cdpPath** path);


// Accessing registers
void* cdp_record_register_read (cdpRecord* reg, size_t position, void* data, size_t* size);        // Reads register data from position and puts it on data buffer (atomically).
void* cdp_record_register_write(cdpRecord* reg, size_t position, const void* data, size_t size);   // Writes the data of a register record at position (atomically and it may reallocate memory).
#define cdp_record_register_update(reg, data, size)   cdp_record_register_write(reg, 0, data, size)


// Accessing books
cdpRecord* cdp_record_top    (cdpRecord* book, bool last);          // Gets the first (or last) record from book.
cdpRecord* cdp_record_by_name (cdpRecord* book, cdpNameID nameID);  // Retrieves a child record by its nameID.
cdpRecord* cdp_record_by_key  (cdpRecord* book, cdpRecord* key);    // Finds a child record based on specified key.
cdpRecord* cdp_record_by_index(cdpRecord* book, size_t index);      // Gets the child record at index position from book.
cdpRecord* cdp_record_by_path (cdpRecord* start, const cdpPath* path);  // Finds a child record based on a path of nameIDs starting from the root or a given book.

cdpRecord* cdp_record_prev(cdpRecord* book, cdpRecord* record);     // Retrieves the previous sibling of record (sorted or unsorted).
cdpRecord* cdp_record_next(cdpRecord* book, cdpRecord* record);     // Retrieves the next sibling of record (sorted or unsorted).
cdpRecord* cdp_record_next_by_name(cdpRecord* book, cdpNameID nameID, uintptr_t* prev);   // Retrieves the first/next (unsorted) child record by its nameID.
cdpRecord* cdp_record_next_by_path(cdpRecord* start, cdpPath* path, uintptr_t* prev);    // Finds the first/next (unsorted) record based on a path of nameIDs starting from the root or a given book.

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

// Creates a duplicate of a record and adds/inserts it (or pushes it) to a book under nameID, optionally including its children.
cdpRecord* cdp_record_add_copy(const cdpRecord* book, cdpNameID nameID, cdpRecord* record, bool includeChildren, bool push);

// Creates a new link to a record and adds/inserts it (or pushes it) to a book under nameID, updating the source record parent list as necessary.
cdpRecord* cdp_record_add_link(cdpRecord* book, cdpNameID nameID, cdpRecord* record, bool push);


// Initiate and shutdown record system.
void cdp_record_system_initiate(void);
void cdp_record_system_shutdown(void);


#endif
