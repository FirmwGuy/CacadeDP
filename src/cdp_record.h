/*
 *  Copyright (c) 2024 Victor M. Barrientos (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
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


static_assert(sizeof(void*) == sizeof(uint64_t), "32 bit is unssoported yet!");


/*
    Cascade Data Processing System - Layer 1
    ------------------------------------------

    CascadeDP Layer 1 is designed to represent and manage hierarchical
    data structures in a distributed execution environment, similar in
    flexibility to representing complex XML or JSON data models. It
    facilitates the storage, navigation, and manipulation of records,
    which can be data values (holding actual information) and/or
    branches of other records (acting as nodes in the hierarchical
    structure with potential to have unique or repeatable fields).

    Key Components
    --------------

    * Record: The fundamental unit within the system, capable of storing data
    and having children records at the same time.

    * Link: A record that points to another record.

    * Metarecord: Each record contains meta, including flags that specify the
    record's characteristics, and a name identifier indicating the record's
    role or ID within its parent.

    * cdpMetadata: This is metadata about the actual data (value) being hold
    by the record.

    The system supports navigating from any record to the root of the database,
    reconstructing paths within the data hierarchy based on field identifiers
    in parent records.

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

    Children Storage Techniques
    ---------------------------

    * cdpRecord->children: Serves as a versatile container within the
    system, holding child records through diverse storage strategies.
    Each storage can adopt one of several mechanisms, determined by
    the structure indicator in its metadata. This design enables
    tailored optimization based on specific needs, such as operation
    frequency, data volume, and access patterns.

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
*/


/*
 * Metadata
 */

typedef uint64_t  cdpID;

#define CDP_NAME_BITS           52
#define CDP_NAME_MAXVAL         (~(((cdpID)(-1)) << CDP_NAME_BITS))
#define CDP_NAMECONV_BITS       2
#define CDP_AUTOID_BITS         (CDP_NAME_BITS - CDP_NAMECONV_BITS)
#define CDP_AUTOID_MAXVAL       (~(((cdpID)(-1)) << CDP_AUTOID_BITS))
#define CDP_AUTOID_MAX          (CDP_AUTOID_MAXVAL - 1)

typedef struct {
  union {
    cdpID     _id;
    struct {
      cdpID   type:       2,    // Type of record (dictionary, link, etc).
              dictionary: 1,    // If children name ids must be unique (1) or they may be repeated (0).
              storage:    2,    // Data structure for children storage.
              withstore:  1,    // Record has child storage (but not necessarily children).

              shadowing:  2,    // If record has shadowing records (links pointing to it).
              task:       1,    // Record belongs to a task (which needs to be activated after I/O).

              system:     1,    // Record can't be modified or deleted (the parent can though).
              hidden:     1,    // Data structure for children storage (it depends on the record type).
              priv:       1,    // Record (with all its children) is private (unlockable).

              name:       CDP_NAME_BITS;    // Name id of this record instance (including naming convention and domain).
    };
  };
} cdpMetarecord;

enum _cdpRecordType {
    CDP_TYPE_VOID,              // Void (uninitialized) record.
    CDP_TYPE_NORMAL,            // Normal (initialized) record.
    CDP_TYPE_LINK,              // Link to another record.
    CDP_TYPE_AGENT,             // Agent function.
    //
    CDP_TYPE_COUNT
};

enum _cdpRecordStorage {
    CDP_STORAGE_LINKED_LIST,    // Children stored in a doubly linked list.
    CDP_STORAGE_ARRAY,          // Children stored in an array.
    CDP_STORAGE_PACKED_QUEUE,   // Children stored in a packed queue (record can't be a dictionary).
    CDP_STORAGE_RED_BLACK_T,    // Children stored in a red-black tree (record must be a dictionary).
    //
    CDP_STORAGE_COUNT
};

enum _cdpRecordShadowing {
    CDP_SHADOW_NONE,            // No shadow records.
    CDP_SHADOW_SINGLE,          // Single shadow record.
    CDP_SHADOW_MULTIPLE,        // Multiple shadows.
};

enum _cdpRecordNaming {
    // This is also used for Domain/Tag naming (do not move!).
    CDP_NAMING_WORD,            // Lowercase text value (10 chars max).
    CDP_NAMING_ACRONYSM,        // Uppercase/numeric text (8 chars max).

    // The following are only used for record naming.
    CDP_NAMING_REFERENCE,       // Numerical reference to text record (this should be a pointer in 32bit systems).
    CDP_NAMING_NUMERIC,         // Per-parent numerical (auto) id.

    CDP_NAMING_COUNT
};

#define cdp_id_from_naming(naming)      (((cdpID)((naming) & 3)) << CDP_AUTOID_BITS)
#define CDP_NAMING_MASK                 cdp_id_from_naming(3)

#define cdp_id_to_word(word)            ((word) | cdp_id_from_naming(CDP_NAMING_WORD))
#define cdp_id_to_acronysm(acro)        ((acro) | cdp_id_from_naming(CDP_NAMING_ACRONYSM))
#define cdp_id_to_reference(ref)        ((ref)  | cdp_id_from_naming(CDP_NAMING_REFERENCE))
#define cdp_id_to_numeric(numb)         ((numb) | cdp_id_from_naming(CDP_NAMING_NUMERIC))

#define cdp_id(name)                    ((name) & CDP_AUTOID_MAXVAL)
#define CDP_ID_SET(name, id)            do{ name = ((name) & CDP_NAMING_MASK) | cdp_id(id); }while(0)

#define CDP_AUTOID_USE                  CDP_AUTOID_MAXVAL
#define CDP_AUTOID                      cdp_id_to_numeric(CDP_AUTOID_USE)

#define cdp_id_naming(name)             (((name) >> CDP_AUTOID_BITS) & 3)
#define cdp_id_name_is_word(name)       ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_WORD))
#define cdp_id_name_is_acronysm(name)   ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_ACRONYSM))
#define cdp_id_name_is_reference(name)  ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_REFERENCE))
#define cdp_id_name_is_numeric(name)    ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_NUMERIC))

#define cdp_id_is_auto(name)            (((name) & ~CDP_NAMING_MASK) == CDP_AUTOID_USE)
#define cdp_id_valid(name)              (((name) & ~CDP_NAMING_MASK) <= CDP_AUTOID_USE)


typedef struct _cdpData     cdpData;
typedef struct _cdpRecord   cdpRecord;
typedef struct _cdpChdStore cdpChdStore;
typedef bool (*cdpAgent)(cdpRecord* task);

typedef union {
    cdpRecord*  link;
    cdpAgent    agent;
    void*       pointer;
    size_t      size;
    uint8_t     byte;
    uint8_t     _byte[8];
    uint64_t    uint64;
    uint32_t    uint32;
    uint32_t    _uint32[2];
    uint16_t    uint16;
    uint16_t    _uint16[4];
    int64_t     int64;
    int32_t     int32;
    int32_t     _int32[2];
    int16_t     int16;
    int16_t     _int16[4];
    float       float32;
    float       _float32[2];
    double      float64;
} cdpValue;

#define CDP_TAG_BITS        61
#define CDP_TAG_VAL_BITS    (CDP_TAG_BITS - 1)
#define CDP_TAG_MAXVAL      (~(((cdpID)(-1)) << CDP_TAG_VAL_BITS))

typedef struct _cdpData {
    struct {
        cdpID           location:   2,  // Where the data is located (see _cdpDataLocation).
                        _unused:    1,
                        domain:     CDP_TAG_BITS;   // Data domain.
    };
    struct {
        cdpID           _reserved:  3,
                        tag:        CDP_TAG_BITS;   // Data tag.
    };
    cdpValue            attribute;      // Data attributes (it depends on domain).
    size_t              size;           // Data size in bytes.
    size_t              capacity;       // Buffer capacity in bytes.
    cdpData*            next;           // Pointer to next data representation (if available).
    union {
        struct {
            void*       data;           // Points to container of data value.
            cdpDel      destructor;     // Data container destructor function.
        };
        struct {
            cdpRecord*  handle;         // Resource record id (used with external libraries).
            cdpRecord*  library;        // Library where the resource is located.
        };
        cdpValue        value[(2 * sizeof(void*)) / sizeof(cdpValue)];  // Data value may start from here.
    };
};

enum _cdpDataLocation {
    CDP_DATALOC_VALUE,          // Data starts at "value" field of cdpData.
    CDP_DATALOC_DATA,           // Data is in address pointed by "data" field of cdpData.
    CDP_DATALOC_HANDLE,         // Data is just a handle to an opaque (library internal) resource.
    //
    CDP_DATALOC_COUNT
};


typedef struct {
    unsigned        count;      // Number of record pointers.
    unsigned        max;
    cdpRecord*      record[];   // Dynamic array of records shadowing this one.
} cdpShadow;

struct _cdpRecord {
    cdpMetarecord   metarecord; // Meta about this record entry (including id, etc).

    union {
        cdpData*    data;       // Address of data buffer.
        cdpAgent    agent;      // Address of an agent function.
        cdpRecord*  link;       // Link to another record.
    };

    union {
        void*       store;      // Parent storage structure (List, Array, etc) where this record is in.
        size_t      basez;      // Base size for arrays and packed-lists.
    };

    union {
        void*       children;   // Pointer to child storage structure.

        cdpRecord*  linked;     // A linked shadow record (if no children, see in cdpChdStore otherwise).
        cdpShadow*  shadow;     // Structure for multiple linked records (if no children).
    };
};

typedef struct _cdpChdStore {
    struct {
        cdpID       _unused:    3,
                    domain:     CDP_TAG_BITS;   // Data domain.
    };
    struct {
        cdpID       _reserved:  3,
                    tag:        CDP_TAG_BITS;   // Data tag.
    };
    cdpChdStore*    next;

    cdpRecord*      owner;          // Parent record owning this child storage.
    union {
        cdpRecord*  linked;         // A linked shadow record (when children, see in cdpRecord otherwise).
        cdpShadow*  shadow;         // Shadow structure (if record has children).
    };
    cdpID           autoid;         // Auto-increment ID for naming contained records.
    size_t          chdCount;       // Number of child records.
};


#define cdp_tag_from_naming(naming)     (((cdpID)((naming) & 1)) << CDP_TAG_VAL_BITS)
#define CDP_TAG_NAMING_MASK             cdp_id_from_naming(1)

#define cdp_tag_to_word(word)           ((word) | cdp_tag_from_naming(CDP_NAMING_WORD))
#define cdp_tag_to_acronysm(acro)       ((acro) | cdp_tag_from_naming(CDP_NAMING_ACRONYSM))

#define cdp_tag(tag)                    ((tag) & CDP_TAG_MAXVAL)
#define CDP_TAG_SET(tag, id)            do{ tag = ((tag) & CDP_TAG_NAMING_MASK) | cdp_tag(id); }while(0)

#define cdp_tag_naming(tag)             (((tag) >> CDP_TAG_VAL_BITS) & 1)
#define cdp_tag_name_is_word(tag)       ((CDP_TAG_NAMING_MASK & (tag)) == cdp_tag_from_naming(CDP_NAMING_WORD))
#define cdp_tag_name_is_acronysm(tag)   ((CDP_TAG_NAMING_MASK & (tag)) == cdp_tag_from_naming(CDP_NAMING_ACRONYSM))

#define cdp_tag_valid(tag)              (((tag) & ~CDP_TAG_NAMING_MASK) <= CDP_TAG_MAXVAL)


typedef struct {
    cdpRecord*      record;
    cdpRecord*      next;
    cdpRecord*      prev;
    cdpRecord*      parent;
    size_t          position;
    unsigned        depth;
} cdpBookEntry;

typedef int  (*cdpCompare)(const cdpRecord* restrict, const cdpRecord* restrict, void*);
typedef bool (*cdpTraverse)(cdpBookEntry*, void*);

typedef struct {
    unsigned        length;
    unsigned        capacity;
    cdpID           id[];
} cdpPath;


/*
 * Record Operations
 */

// Initiate and shutdown record system.
void cdp_record_system_initiate(void);
void cdp_record_system_shutdown(void);


bool cdp_record_initialize( cdpRecord* record, cdpID name, unsigned type,
                            bool dictionary, unsigned storage, size_t basez,
                            cdpMetadata metadata, size_t capacity, size_t size,
                            cdpValue data, cdpDel destructor  );
void cdp_record_initialize_clone(cdpRecord* newClone, cdpID nameID, cdpRecord* record);
void cdp_record_finalize(cdpRecord* record);

#define cdp_record_initialize_value(r, name, metadata, capacity, size, data, destructor)    cdp_record_initialize(r, name, CDP_TYPE_NORMAL, true,  CDP_STORAGE_RED_BLACK_T, 0, metadata, capacity, size, data, destructor)
#define cdp_record_initialize_data(r, name, metadata, capacity, size, data, destructor)     cdp_record_initialize(r, name, CDP_TYPE_NORMAL, true,  CDP_STORAGE_RED_BLACK_T, 0, metadata, cdp_max(capacity, sizeof((cdpData){}._data)), size, data, destructor)
#define cdp_record_initialize_branch(r, name, storage, basez)                               cdp_record_initialize(r, name, CDP_TYPE_NORMAL, false, storage, basez, (cdpMetadata){0}, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_initialize_dictionary(r, name, storage, basez)                           cdp_record_initialize(r, name, CDP_TYPE_NORMAL, true,  storage, basez, (cdpMetadata){0}, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_initialize_link(r, name, source)                                         cdp_record_initialize(r, name, CDP_TYPE_LINK,   true,  CDP_STORAGE_RED_BLACK_T, 0, (cdpMetadata){0}, 0, 0, (cdpValue)(source), NULL)
#define cdp_record_initialize_agent(r, name, agent)                                         cdp_record_initialize(r, name, CDP_TYPE_AGENT,  true,  CDP_STORAGE_RED_BLACK_T, 0, (cdpMetadata){0}, 0, 0, (cdpValue)(agent), NULL)

static inline void  cdp_record_set_id(cdpRecord* record, cdpID id)      {assert(record && cdp_id_valid(id));  CDP_ID_SET(record->metarecord.name, id);}
static inline void  cdp_record_set_name(cdpRecord* record, cdpID name)  {assert(record && cdp_id_valid(name));  record->metarecord.name = name;}    // FixMe: mask 'name' before assignation.
static inline cdpID cdp_record_get_name(const cdpRecord* record)        {assert(record);  return record->metarecord.name;}
#define cdp_record_get_id(r)  cdp_id(cdp_record_get_name(r))

#define cdp_record_with_store(record)   ((record)->metarecord.withstore == 1)
#define CDP_CHD_STORE(children)         ({assert(children);  (cdpChdStore*)(children);})
#define cdp_record_par_store(record)    CDP_CHD_STORE((record)->store)
static inline cdpRecord* cdp_record_parent  (const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->owner: NULL;}
static inline size_t     cdp_record_siblings(const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->chdCount: 0;}
static inline size_t     cdp_record_children(const cdpRecord* record)   {assert(record);  return cdp_record_with_store(record)? CDP_CHD_STORE(record->children)->chdCount: 0;}

#define cdp_record_is_dictionary(r) ((r)->metarecord.dictionary)

#define cdp_record_is_void(r)       (!(r)->metarecord.type)
#define cdp_record_is_normal(r)     ((r)->metarecord.type == CDP_TYPE_NORMAL)
#define cdp_record_is_link(r)       ((r)->metarecord.type == CDP_TYPE_LINK)
#define cdp_record_is_agent(r)      ((r)->metarecord.type == CDP_TYPE_AGENT)

#define cdp_record_is_insertable(r) ((r)->metarecord.storage != CDP_STORAGE_RED_BLACK_T)

#define cdp_record_is_shadowed(r)   ((r)->metarecord.shadowing)
#define cdp_record_is_private(r)    ((r)->metarecord.priv)
#define cdp_record_is_system(r)     ((r)->metarecord.system)

static inline bool cdp_record_has_data(cdpRecord* record)   {assert(cdp_record_is_normal(record));  return record->data;}

#define cdp_record_id_is_pending(r) cdp_id_is_auto((r)->metarecord.name)
static inline void  cdp_record_set_autoid(const cdpRecord* record, cdpID id)  {assert(cdp_record_with_store(record));  cdpChdStore* store = CDP_CHD_STORE(record->children); assert(store->autoid < id  &&  id <= CDP_AUTOID_MAX); store->autoid = id;}
static inline cdpID cdp_record_get_autoid(const cdpRecord* record)            {assert(cdp_record_with_store(record));  return CDP_CHD_STORE(record->children)->autoid;}

void cdp_record_relink_storage(cdpRecord* record);

static inline void cdp_record_transfer(cdpRecord* src, cdpRecord* dst) {
    assert(!cdp_record_is_void(src) && dst);

    *dst = *src;

    if (cdp_record_children(dst))
        cdp_record_relink_storage(dst);

    // ToDo: relink self list.
}

static inline void cdp_record_replace(cdpRecord* oldr, cdpRecord* newr) {
    cdp_record_finalize(oldr);
    cdp_record_transfer(newr, oldr);
}


// Appends, inserts or prepends a (copy of) record into another record.
cdpRecord* cdp_record_add(cdpRecord* parent, cdpRecord* record, bool prepend);
cdpRecord* cdp_record_sorted_insert(cdpRecord* parent, cdpRecord* record, cdpCompare compare, void* context);

#define cdp_record_add_record(parent, name, type, dictionary, storage, basez, metadata, capacity, size, data, destructor)\
    ({cdpRecord r={0}; cdp_record_initialize(&r, name, type, dictionary, storage, basez, metadata, capacity, size, data, destructor)? cdp_record_add(parent, &r, false): NULL;})

#define cdp_record_add_data(parent, name, metadata, capacity, size, data, destructor)       cdp_record_add_record(parent, name, CDP_TYPE_NORMAL, true,  CDP_STORAGE_RED_BLACK_T, 0, metadata, capacity, size, data, destructor)
#define cdp_record_add_value(parent, name, metadata, v)                                     cdp_record_add_record(parent, name, CDP_TYPE_NORMAL, true,  CDP_STORAGE_RED_BLACK_T, 0, metadata, sizeof(cdpValue), 0, v, NULL)
#define cdp_record_add_branch(parent, name, storage, basez)                                 cdp_record_add_record(parent, name, CDP_TYPE_NORMAL, false, storage, basez, (cdpMetadata){0}, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_add_dictionary(parent, name, storage, basez)                             cdp_record_add_record(parent, name, CDP_TYPE_NORMAL, true,  storage, basez, (cdpMetadata){0}, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_add_link(parent, name, source)                                           cdp_record_add_record(parent, name, CDP_TYPE_LINK,   true,  CDP_STORAGE_RED_BLACK_T, 0, (cdpMetadata){0}, 0, 0, (cdpValue)(source), NULL)
#define cdp_record_add_agent(parent, name, agent)                                           cdp_record_add_record(parent, name, CDP_TYPE_AGENT,  true,  CDP_STORAGE_RED_BLACK_T, 0, (cdpMetadata){0}, 0, 0, (cdpValue)(agent), NULL)

#define cdp_record_prepend_record(parent, name, type, dictionary, storage, basez, metadata, capacity, size, data, destructor)\
    ({cdpRecord r={0}; cdp_record_initialize(&r, name, type, dictionary, storage, basez, metadata, capacity, size, data, destructor)? cdp_record_add(parent, &r, true): NULL;})

#define cdp_record_prepend_data(parent, name, metadata, capacity, size, data, destructor)   cdp_record_prepend_record(parent, name, CDP_TYPE_NORMAL, true, CDP_STORAGE_RED_BLACK_T, 0, metadata, capacity, size, data, destructor)
#define cdp_record_prepend_value(parent, name, metadata, v)                                 cdp_record_prepend_record(parent, name, CDP_TYPE_NORMAL, true, CDP_STORAGE_RED_BLACK_T, 0, metadata, sizeof(cdpValue), 0, v, NULL)
#define cdp_record_prepend_branch(parent, name, storage, basez)                             cdp_record_prepend_record(parent, name, CDP_TYPE_NORMAL, false, storage, basez, (cdpMetadata){0}, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_prepend_dictionary(parent, name, storage, basez)                         cdp_record_prepend_record(parent, name, CDP_TYPE_NORMAL, true,  storage, basez, (cdpMetadata){0}, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_prepend_link(parent, name, source)                                       cdp_record_prepend_record(parent, name, CDP_TYPE_LINK,   true,  CDP_STORAGE_RED_BLACK_T, 0, (cdpMetadata){0}, 0, 0, (cdpValue)(source), NULL)
#define cdp_record_prepend_agent(parent, name, agent)                                       cdp_record_prepend_record(parent, name, CDP_TYPE_AGENT,  true,  CDP_STORAGE_RED_BLACK_T, 0, (cdpMetadata){0}, 0, 0, (cdpValue)(agent), NULL)


// Accessing data
void*    cdp_record_read(const cdpRecord* record, size_t* capacity, size_t* size, void* data);
cdpValue cdp_record_read_value(const cdpRecord* record);
#define cdp_record_data(r)              cdp_record_read(r, NULL, NULL, NULL)
#define cdp_record_data_capactiy(r)     ({size_t c; cdp_record_read(r, &c, NULL, NULL); c;})
#define cdp_record_data_size(r)         ({size_t z; cdp_record_read(r, NULL, &z, NULL); z;})

void* cdp_record_update(cdpRecord* record, size_t capacity, size_t size, cdpValue data, bool swap);
#define cdp_record_update_value(r, v)   cdp_record_update(r, sizeof(cdpValue), sizeof(cdpValue), v, false)

void cdp_record_data_delete(cdpRecord* record);
void cdp_record_data_reset(cdpRecord* record);
void cdp_record_branch_reset(cdpRecord* record);
#define cdp_record_reset(r)     do{ cdp_record_branch_reset(r); cdp_record_data_reset(r);} while(0)
#define cdp_record_delete(r)    cdp_record_remove(r, NULL)


// Constructs the full path (sequence of ids) for a given record, returning the depth.
bool cdp_record_path(const cdpRecord* record, cdpPath** path);

// Root dictionary.
static inline cdpRecord* cdp_root(void)  {extern cdpRecord CDP_ROOT; assert(!cdp_record_is_void(&CDP_ROOT));  return &CDP_ROOT;}


// Accessing branched records
cdpRecord* cdp_record_first(const cdpRecord* record);
cdpRecord* cdp_record_last (const cdpRecord* record);

cdpRecord* cdp_record_find_by_name(const cdpRecord* record, cdpID name);
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context);
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t pos);
cdpRecord* cdp_record_find_by_path(const cdpRecord* start, const cdpPath* path);

cdpRecord* cdp_record_prev(const cdpRecord* parent, cdpRecord* record);
cdpRecord* cdp_record_next(const cdpRecord* parent, cdpRecord* record);

cdpRecord* cdp_record_find_next_by_name(const cdpRecord* record, cdpID id, uintptr_t* childIdx);
cdpRecord* cdp_record_find_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev);

bool cdp_record_traverse     (cdpRecord* record, cdpTraverse func, void* context, cdpBookEntry* entry);
bool cdp_record_deep_traverse(cdpRecord* record, cdpTraverse func, cdpTraverse listEnd, void* context, cdpBookEntry* entry);


// Removing records
bool cdp_record_child_take(cdpRecord* record, cdpRecord* target);
bool cdp_record_child_pop(cdpRecord* record, cdpRecord* target);
void cdp_record_remove(cdpRecord* record, cdpRecord* target);


// Converts an unsorted record into a sorted one.
void cdp_record_to_dictionary(cdpRecord* record);


/*
    TODO:
    - Change name from 'id' to 'name' in cdpPath.
    - Implement clone (deep copy) records.
    - Traverse book in internal (stoTech) order.
    - Add indexof for records.
    - Implement cdp_book_insert_at(position).
    - Update MAX_DEPTH based on path/traverse operations.
    - Add cdp_record_update_nested_links(old, new).
    - Any storage tech may be a dictionary, but only if the name matches the insertion/deletion sequence.
    - If a record is added with its name explicitly above "auto_id", then that must be updated.
    - CDP_TAG_VOID should never be a valid name for records.
    - Send simultaneous tasks to nested agent records.
*/


#endif
