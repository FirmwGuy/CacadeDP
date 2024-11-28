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


static_assert(sizeof(void*) == sizeof(uint64_t), "32 bit is unsupported yet!");


/*
    Cascade Data Processing System - Layer 1
    ------------------------------------------

    CascadeDP Layer 1 is designed to represent and manage hierarchical
    data structures in a distributed execution environment, similar in
    flexibility to representing complex XML or JSON data models. It
    facilitates the storage, navigation, and manipulation of records,
    which can point to data values (holding actual information) and
    to branches of other records (acting as nodes in the hierarchical
    structure).

    Key Components
    --------------

    * Record: The fundamental unit within the system, capable of storing data
    and having children records at the same time.

    * Link: A record that points to another record.

    * Agent: A record with the address of a callable agent function (able
    to be activated on record events).

    * Metarecord: Each record contains meta, including flags that specify the
    record's characteristics, and a name identifier indicating the record's
    role or ID within its parent.

    * Data: This is where the actual data (value) being hold by the record is
    located. It has its own metadata.

    * Store: Storage for children records according to different indexing
    techniques.

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

    The system is adept at managing hierarchical data structures through a
    variety of storage techniques, encapsulated within each store. This
    flexibility allows the system to adapt to different use cases and
    optimization requirements, particularly focusing on cache efficiency and
    operation speed for insertions, deletions, and lookups.

    Children Storage Techniques
    ---------------------------

    * Store Metadata: Stores serve as a versatile container within the system,
    holding child records through diverse storage strategies. Each storage can
    adopt one of several mechanisms, determined by the structure indicator in
    its metadata. This design enables tailored optimization based on specific
    needs, such as operation frequency, data volume, and access patterns.

    * Storage Types: Each storage type is selected to optimize specific aspects
    of data management, addressing the system's goals of flexibility,
    efficiency, and navigability.

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

      Octree: Used for (3D) spatial indexing according to contained data.
      It only needs a comparation function able to determine if the record
      fully fits in between a quadrant or not.
*/


typedef union  _cdpValue      cdpValue;
typedef struct _cdpData       cdpData;
typedef struct _cdpStore      cdpStore;
typedef struct _cdpRecord     cdpRecord;
typedef struct _cdpAgentList  cdpAgentList;

typedef int   (*cdpCompare)(const cdpRecord* restrict, const cdpRecord* restrict, void*);
typedef void* (*cdpAgent)  (cdpRecord* client, cdpRecord* subject, unsigned verb, cdpRecord* object, cdpValue value);


/*
 *  Record Metadata
 */

typedef uint64_t  cdpID;

#define CDP_NAME_BITS           58
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

              hidden:     1,    // Record won't appaear on listings (it can only be accesed directly).
              shadowing:  2,    // If record has shadowing records (links pointing to it).
              task:       1,    // Record belongs to a task (which needs to be activated after I/O).

              name:       CDP_NAME_BITS;    // Name id of this record instance (including naming convention).
    };
  };
} cdpMetarecord;

enum _cdpRecordType {
    CDP_TYPE_VOID,              // A void (uninitialized) record.
    CDP_TYPE_NORMAL,            // Regular record.
    CDP_TYPE_FLEX,              // A single record that can automatically expand to a list.
    CDP_TYPE_LINK,              // Link to another record.
    //
    CDP_TYPE_COUNT
};

enum _cdpRecordShadowing {
    CDP_SHADOW_NONE,            // No shadow records.
    CDP_SHADOW_SINGLE,          // Single shadow record.
    CDP_SHADOW_MULTIPLE,        // Multiple shadows.
};

enum _cdpRecordNaming {
    CDP_NAMING_WORD,            // Lowercase text value, 11 chars max (it must be the first in this enum!).
    CDP_NAMING_ACRONYSM,        // Uppercase/numeric text, 9 characters maximum.
    CDP_NAMING_REFERENCE,       // Numerical reference to text record (a pointer in 32bit systems).
    CDP_NAMING_NUMERIC,         // Per-parent numerical ID.

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

#define cdp_id_is_auto(name)            ((name) == CDP_AUTOID)
#define cdp_id_is_word(name)            ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_WORD))
#define cdp_id_is_acronysm(name)        ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_ACRONYSM))
#define cdp_id_is_reference(name)       ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_REFERENCE))
#define cdp_id_is_numeric(name)         ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_NUMERIC))

#define cdp_id_valid(id)                (cdp_id(id) && ((id) <= CDP_AUTOID))
#define cdp_id_text_valid(name)         (cdp_id(name) && !cdp_id_is_numeric(name))
#define cdp_id_naming(name)             (((name) >> CDP_AUTOID_BITS) & 3)


cdpID  cdp_text_to_acronysm(const char *s);
cdpID  cdp_text_to_word(const char *s);
size_t cdp_acronysm_to_text(cdpID acro, char s[10]);
size_t cdp_word_to_text(cdpID coded, char s[12]);

#define CDP_ID                  UINT64_C

#define CDP_WORD_ROOT           CDP_ID(0x007C000000000000)      /* "/"           */
#define CDP_WORD_LIST           CDP_ID(0x003133A000000000)      /* "list"        */
#define CDP_WORD_DICTIONARY     CDP_ID(0x001123A25EE0CB20)      /* "dictionary"  */
#define CDP_WORD_CATALOG        CDP_ID(0x000C340B1E700000)      /* "catalog"     */

#define CDP_ACRON_CDP           CDP_ID(0x0123930000000000)      /* "CDP"         */


struct _cdpAgentList {
    cdpAgentList*   next;
    cdpAgent        agent;
    struct {
        cdpID       _unused:    6,
                    domain:     CDP_NAME_BITS;
        cdpID       _reserved:  6,
                    tag:        CDP_NAME_BITS;
    };
};

static inline cdpAgentList* cdp_agent_list_new(cdpID domain, cdpID tag, cdpAgent agent) {
    assert(cdp_id_text_valid(domain) && cdp_id_text_valid(tag) && agent);

    CDP_NEW(cdpAgentList, list);
    list->domain = domain;
    list->tag    = tag;
    list->agent  = agent;

    return list;
}

#define cdp_agent_list_del    cdp_free


/*
    Record Data
*/

union _cdpValue {
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

    size_t      size;
    void*       pointer;

    cdpID       id;
    cdpRecord*  record;
};

#define CDP_V(v)    ((cdpValue)(v))


struct _cdpData {
    struct {
        cdpID           datatype:   2,  // Type of data (see _cdpDataType).
                        _unused:    4,
                        domain:     CDP_NAME_BITS;  // Data domain.
    };
    struct {
        cdpID           writable:   1,  // If data can be updated.
                        lock:       1,  // Lock on data content.
                        _reserved:  4,
                        tag:        CDP_NAME_BITS;  // Data tag.
    };

    cdpID               attribute;      // Data attributes (it depends on domain).
    size_t              size;           // Data size in bytes.
    size_t              capacity;       // Buffer capacity in bytes.

    cdpData*            next;           // Pointer to next data representation (if available).
    cdpAgentList*       agent;          // List of agents to be run on data modification.

    cdpValue            hash;           // Hash value of content.
    union {
        struct {
            void*       data;           // Points to container of data value.
            cdpDel      destructor;     // Data container destruction function.
        };
        struct {
          union {
            cdpRecord*  handle;         // Resource record id (used with external libraries).
            cdpRecord*  stream;         // Data window to streamed content.
          };
          cdpRecord*    library;        // Library where the resource is located.
        };
        cdpValue        value[(2 * sizeof(void*)) / sizeof(cdpValue)];  // Data value may start from here.
    };
};

enum _cdpDataType {
    CDP_DATATYPE_VALUE,         // Data starts at "value" field of cdpData.
    CDP_DATATYPE_DATA,          // Data is in address pointed by "data" field.
    CDP_DATATYPE_HANDLE,        // Data is just a handle to an opaque (library internal) resource.
    CDP_DATATYPE_STREAM,        // Data is a window to a larger (library internal) stream.
    //
    CDP_DATATYPE_COUNT
};


cdpData* cdp_data_new(  cdpID domain, cdpID tag,
                        cdpID attribute, unsigned datatype, bool writable,
                        void** dataloc, cdpValue value, ...  );
void     cdp_data_del(cdpData* data);
void*    cdp_data(const cdpData* data);
#define  cdp_data_valid(d)                      ((d) && (d)->capacity && cdp_id_text_valid((d)->domain) && cdp_id_text_valid((d)->tag))
#define  cdp_data_new_value(d, t, a, value)     cdp_data_new(d, t, a, CDP_DATATYPE_VALUE, true, NULL, CDP_V(value))

static inline void cdp_data_add_agent(cdpData* data, cdpID domain, cdpID tag, cdpAgent agent) {
    assert(cdp_data_valid(data));

    cdpAgentList* list = cdp_agent_list_new(domain, tag, agent);
    if (data->agent)
        list->next = data->agent;
    data->agent = list;
}


/*
    Record Storage (for children)
*/

typedef struct {
    unsigned        count;      // Number of record pointers.
    unsigned        capacity;   // Capacity of array.
    cdpRecord*      record[];   // Array of records shadowing this one.
} cdpShadow;


struct _cdpStore {
    struct {
        cdpID       storage:    3,              // Data structure for children storage (array, linked-list, etc).
                    indexing:   2,              // Indexing (sorting) criteria for children.
                    _unused:    1,
                    domain:     CDP_NAME_BITS;  // Data domain.
    };
    struct {
        cdpID       writable:   1,              // If chidren can be added/deleted.
                    lock:       1,              // Lock on children operations.
                    _reserved:  4,
                    tag:        CDP_NAME_BITS;  // Data tag.
    };

    cdpRecord*      owner;      // Record owning this child storage.
    union {
        cdpRecord*  linked;     // A linked shadow record (when children, see in cdpRecord otherwise).
        cdpShadow*  shadow;     // Shadow structure (if record has children).
    };

    //cdpStore*       next;       // Next index/storage (requires hard links).
    cdpAgentList*   agent;      // List of agents to be run on child updates.

    size_t          chdCount;   // Number of child records.
    cdpCompare      compare;    // Compare function for indexing children.
    cdpID           autoid;     // Auto-increment ID for inserting new child records.

    // The specific storage structure will follow after this...
};

enum _cdpRecordStorage {
    CDP_STORAGE_LINKED_LIST,    // Children stored in a doubly linked list.
    CDP_STORAGE_ARRAY,          // Children stored in an array.
    CDP_STORAGE_PACKED_QUEUE,   // Children stored in a packed queue.
    CDP_STORAGE_RED_BLACK_T,    // Children stored in a red-black tree.
    CDP_STORAGE_OCTREE,         // Children stored in an octree spatial index.
    //
    CDP_STORAGE_COUNT
};

enum _cdpRecordIndexing {
    CDP_INDEX_BY_INSERTION,      // Children indexed by their insertion order (the default).
    CDP_INDEX_BY_NAME,           // Children indexed by their unique name (a dicionary).
    CDP_INDEX_BY_FUNCTION,       // Children indexed by a custom comparation function.
    CDP_INDEX_BY_HASH,           // Children indexed by data hash value (first) and then by a comparation function (second).
    //
    CDP_INDEX_COUNT
};


cdpStore* cdp_store_new(cdpID domain, cdpID tag, unsigned storage, unsigned indexing, ...);
void      cdp_store_del(cdpStore* store);
void      cdp_store_delete_children(cdpStore* store);
#define   cdp_store_valid(s)      ((s) && cdp_id_text_valid((s)->domain) && cdp_id_text_valid((s)->tag))

static inline bool cdp_store_is_insertable(cdpStore* store)   {assert(cdp_store_valid(store));  return (store->indexing == CDP_INDEX_BY_INSERTION);}
static inline bool cdp_store_is_dictionary(cdpStore* store)   {assert(cdp_store_valid(store));  return (store->indexing == CDP_INDEX_BY_NAME);}
static inline bool cdp_store_is_f_sorted(cdpStore* store)     {assert(cdp_store_valid(store));  return (store->indexing == CDP_INDEX_BY_FUNCTION  ||  store->indexing == CDP_INDEX_BY_HASH);}
static inline bool cdp_store_is_sorted(cdpStore* store)       {assert(cdp_store_valid(store));  return (store->indexing != CDP_INDEX_BY_INSERTION);}
static inline bool cdp_store_is_empty(cdpStore* store)        {assert(cdp_store_valid(store));  return !store->chdCount;}

static inline void cdp_store_add_agent(cdpStore* store, cdpID domain, cdpID tag, cdpAgent agent) {
    assert(cdp_store_valid(store));
    cdpAgentList* list = cdp_agent_list_new(domain, tag, agent);
    if (store->agent)
        list->next = store->agent;
    store->agent = list;
}


/*
    Record
*/

typedef struct {
    unsigned        length;
    unsigned        capacity;
    cdpID           id[];
} cdpPath;


struct _cdpRecord {
    cdpMetarecord   metarecord; // Meta about this record entry (including id, etc).
    cdpStore*       parent;     // Parent structure (list, array, etc) where this record is stored in.

    union {
        cdpData*    data;       // Address of cdpData structure.

        cdpRecord*  link;       // Link to another record.
    };

    union {
        cdpStore*   store;      // Address of cdpStore structure.

        cdpRecord*  linked;     // A linked shadow record (if no children, see in cdpStore otherwise).
        cdpShadow*  shadow;     // Structure for multiple linked records (if no children).

        cdpPath*    target;     // Path to linked target (if record is a Link).
    };
};


typedef struct {
    cdpRecord*      record;
    cdpRecord*      next;
    cdpRecord*      prev;
    cdpRecord*      parent;
    size_t          position;
    unsigned        depth;
} cdpEntry;

typedef bool (*cdpTraverse)(cdpEntry*, void*);


/*
 * Record Operations
 */

enum _cdpAction {
    CDP_ACTION_GET_INLET,
    CDP_ACTION_CONNECT,
    CDP_ACTION_UNPLUG,
    //
    CDP_ACTION_DATA_NEW,
    CDP_ACTION_DATA_ATTRIBUTE,
    CDP_ACTION_DATA_UPDATE,
    CDP_ACTION_DATA_DELETE,
    //
    CDP_ACTION_STORE_NEW,
    CDP_ACTION_STORE_ADD_ITEM,
    CDP_ACTION_STORE_REMOVE_ITEM,
    CDP_ACTION_STORE_DELETE,

    CDP_ACTION_COUNT
};


// Initiate and shutdown record system
void cdp_record_system_initiate(void);
void cdp_record_system_shutdown(void);


// Initiate records
void cdp_record_initialize(cdpRecord* record, unsigned type, cdpID name, cdpData* data, cdpStore* store);
void cdp_record_initialize_clone(cdpRecord* newClone, cdpID nameID, cdpRecord* record);
void cdp_record_finalize(cdpRecord* record);

#define cdp_record_initialize_value(r, name, domain, tag, attrib, value, size, capacity)              cdp_record_initialize(r, CDP_TYPE_NORMAL, name, cdp_data_new(domain, tag, attrib, CDP_DATATYPE_VALUE, true, NULL, CDP_V(value), size, capacity), NULL)
#define cdp_record_initialize_data(r, name, domain, tag, attrib, data, size, capacity, destructor)    cdp_record_initialize(r, CDP_TYPE_NORMAL, name, cdp_data_new(domain, tag, attrib, CDP_DATATYPE_DATA,  true, NULL, CDP_V(data),  size, capacity, destructor), NULL)

#define cdp_record_initialize_list(r, name, domain, tag, storage, ...)                  cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_initialize_dictionary(r, name, domain, tag, storage, ...)            cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_initialize_catalog(r, name, domain, tag, storage, ...)               cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))
#define cdp_record_initialize_spatial(r, name, domain, tag, center, subwide, compare)   cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(domain, tag, CDP_STORAGE_OCTREE, CDP_INDEX_BY_FUNCTION, center, subwide, compare))

static inline void  cdp_record_set_id(cdpRecord* record, cdpID id)      {assert(record && cdp_id_valid(id));  CDP_ID_SET(record->metarecord.name, id);}
static inline void  cdp_record_set_name(cdpRecord* record, cdpID name)  {assert(record && cdp_id_text_valid(name));  record->metarecord.name = name;}    // FixMe: mask 'name' before assignation.
static inline cdpID cdp_record_get_name(const cdpRecord* record)        {assert(record);  return record->metarecord.name;}
#define cdp_record_get_id(r)    cdp_id(cdp_record_get_name(r))

#define cdp_record_is_void(r)       (((r)->metarecord.type == CDP_TYPE_VOID) || !cdp_id_valid((r)->metarecord.name))
#define cdp_record_is_normal(r)     ((r)->metarecord.type == CDP_TYPE_NORMAL)
#define cdp_record_is_flex(r)       ((r)->metarecord.type == CDP_TYPE_FLEX)
#define cdp_record_is_link(r)       ((r)->metarecord.type == CDP_TYPE_LINK)

#define cdp_record_is_shadowed(r)   ((r)->metarecord.shadowing)
#define cdp_record_is_private(r)    ((r)->metarecord.priv)
#define cdp_record_is_system(r)     ((r)->metarecord.system)

static inline bool cdp_record_has_data(const cdpRecord* record)     {assert(cdp_record_is_normal(record));  return record->data;}
static inline bool cdp_record_has_store(const cdpRecord* record)    {assert(cdp_record_is_normal(record));  return record->store;}

static inline void cdp_record_set_data(cdpRecord* record, cdpData* data)      {assert(!cdp_record_has_data(record) && cdp_data_valid(data));   record->data = data;}
static inline void cdp_record_set_store(cdpRecord* record, cdpStore* store)   {assert(!cdp_record_has_store(record));  record->store = store;}

static inline cdpRecord* cdp_record_parent  (const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->parent)? record->parent->owner: NULL;}
static inline size_t     cdp_record_siblings(const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->parent)? record->parent->chdCount: 0;}
static inline size_t     cdp_record_children(const cdpRecord* record)   {assert(record);  return cdp_record_has_store(record)? record->store->chdCount: 0;}

#define cdp_record_id_is_pending(r)   cdp_id_is_auto((r)->metarecord.name)
static inline void  cdp_record_set_autoid(const cdpRecord* record, cdpID id)  {assert(cdp_record_has_store(record) && (record->store->autoid < id)  &&  (id <= CDP_AUTOID_MAX)); record->store->autoid = id;}
static inline cdpID cdp_record_get_autoid(const cdpRecord* record)            {assert(cdp_record_has_store(record));  return record->store->autoid;}


static inline void cdp_record_relink_storage(cdpRecord* record)     {assert(cdp_record_has_store(record));  record->store->owner = record;}     // Re-link record with its own children storage.

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


// Root dictionary
static inline cdpRecord* cdp_root(void)  {extern cdpRecord CDP_ROOT; assert(!cdp_record_is_void(&CDP_ROOT));  return &CDP_ROOT;}


// Links
static inline void cdp_link_set(cdpRecord* link, cdpRecord* target) {
    assert(link && cdp_record_is_link(link));
    assert(target && !cdp_record_is_void(target) && cdp_record_parent(target));     // Links to "root" aren't allowed for now.
    link->link = target;
    // ToDo: remove link from target's shadows.
}

static inline void cdp_link_initialize(cdpRecord* link, cdpID name, cdpRecord* target) {
    assert(link);
    cdp_record_initialize(link, CDP_TYPE_LINK, name, NULL, NULL);
    if (target)
        cdp_link_set(link, target);
}

static inline cdpRecord* cdp_link_pull(cdpRecord* link) {
    assert(link);
    while (cdp_record_is_link(link)) {
        link = link->link;
    }
    return link;
}

static inline bool cdp_record_is_insertable(cdpRecord* record)  {assert(cdp_record_has_store(record));  return record->store? cdp_store_is_insertable(record->store): false;}
static inline bool cdp_record_is_dictionary(cdpRecord* record)  {assert(cdp_record_is_normal(record));  return record->store? cdp_store_is_dictionary(record->store): false;}
static inline bool cdp_record_is_f_sorted(cdpRecord* record)    {assert(cdp_record_is_normal(record));  return record->store? cdp_store_is_f_sorted(record->store): false;}
static inline bool cdp_record_is_sorted(cdpRecord* record)      {assert(cdp_record_is_normal(record));  return record->store? cdp_store_is_sorted(record->store): false;}

static inline bool cdp_record_is_empty(cdpRecord* record)       {assert(cdp_record_is_normal(record));  return (!record->data && !cdp_record_children(record));}
static inline bool cdp_record_is_floating(cdpRecord* record)    {assert(cdp_record_is_normal(record));  return (cdp_record_is_void(record)  ||  (!cdp_record_parent(record) && (record != cdp_root())));}


// Appends/prepends or inserts a (copy of) record into another record
cdpRecord* cdp_record_add(cdpRecord* record, cdpValue context, cdpRecord* child);
cdpRecord* cdp_record_append(cdpRecord* record, bool prepend, cdpRecord* child);

#define cdp_record_add_child(record, type, name, context, data, store)         \
    ({cdpRecord child__={0}; cdp_record_initialize(&child__, type, name, data, store); cdp_record_add(record, context, &child__);})

#define cdp_record_add_value(record, name, context, domain, tag, attrib, value, size, capacity)             cdp_record_add_child(record, CDP_TYPE_NORMAL, name, CDP_V(context), cdp_data_new(domain, tag, attrib, CDP_DATATYPE_VALUE, true, NULL, CDP_V(value), size, capacity), NULL)
#define cdp_record_add_data(record, name, context, domain, tag, attrib, data, size, capacity, destructor)   cdp_record_add_child(record, CDP_TYPE_NORMAL, name, CDP_V(context), cdp_data_new(domain, tag, attrib, CDP_DATATYPE_DATA, true, NULL, CDP_V(data), size, capacity, destructor), NULL)

#define cdp_record_add_list(record, name, context, domain, tag, storage, ...)                               cdp_record_add_child(record, CDP_TYPE_NORMAL, name, CDP_V(context), NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_add_dictionary(record, name, context, domain, tag, storage, ...)                         cdp_record_add_child(record, CDP_TYPE_NORMAL, name, CDP_V(context), NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_add_catalog(record, name, context, domain, tag, storage, ...)                            cdp_record_add_child(record, CDP_TYPE_NORMAL, name, CDP_V(context), NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))

#define cdp_record_add_link(record, name, context, source)                                                  cdp_record_add_child(record, CDP_TYPE_LINK,  name, CDP_V(context), CDP_P(source), NULL)

#define cdp_dict_add(r, c)                      cdp_record_add(r, CDP_V(0), c)
#define cdp_dict_add_value(r, n, ...)           cdp_record_add_value(r, n, 0, __VA_ARGS__)
#define cdp_dict_add_data(r, n, ...)            cdp_record_add_data(r, n, 0, __VA_ARGS__)
#define cdp_dict_add_list(r, n, ...)            cdp_record_add_list(r, n, 0, __VA_ARGS__)
#define cdp_dict_add_dictionary(r, n, ...)      cdp_record_add_dictionary(r, n, 0, __VA_ARGS__)
#define cdp_dict_add_catalog(r, n, ...)         cdp_record_add_catalog(r, n, 0, __VA_ARGS__)
#define cdp_dict_add_link(r, n, ...)            cdp_record_add_link(r, n, 0, __VA_ARGS__)

#define cdp_record_append_child(record, type, name, prepend, data, store)      \
    ({cdpRecord child__={0}; cdp_record_initialize(&child__, type, name, data, store); cdp_record_append(record, prepend, &child__);})

#define cdp_record_append_value(record, name, domain, tag, attrib, value, size, capacity)                   cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, cdp_data_new(domain, tag, attrib, CDP_DATATYPE_VALUE, true, NULL, CDP_V(value), size, capacity), NULL)
#define cdp_record_append_data(record, name, domain, tag, attrib, data, size, capacity, destructor)         cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, cdp_data_new(domain, tag, attrib, CDP_DATATYPE_DATA, true, NULL, CDP_V(data), size, capacity, destructor), NULL)

#define cdp_record_append_list(record, name, domain, tag, storage, ...)                                     cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_append_dictionary(record, name, domain, tag, storage, ...)                               cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_append_catalog(record, name, domain, tag, storage, ...)                                  cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))

#define cdp_record_append_link(record, name, source)                                                        cdp_record_append_child(record, CDP_TYPE_LINK,  name, false, CDP_P(source), NULL)

#define cdp_record_prepend_value(record, name, domain, tag, attrib, value, size, capacity)                  cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, cdp_data_new(domain, tag, attrib, CDP_DATATYPE_VALUE, true, NULL, CDP_V(value), size, capacity), NULL)
#define cdp_record_prepend_data(record, name, domain, tag, attrib, data, size, capacity, destructor)        cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, cdp_data_new(domain, tag, attrib, CDP_DATATYPE_DATA, true, NULL, CDP_V(data), size, capacity, destructor), NULL)

#define cdp_record_prepend_list(record, name, domain, tag, storage, ...)                                    cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_prepend_dictionary(record, name, domain, tag, storage, ...)                              cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_prepend_catalog(record, name, domain, tag, storage, ...)                                 cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, NULL, cdp_store_new(domain, tag, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))

#define cdp_record_prepend_link(record, name, source)                                                       cdp_record_append_child(record, CDP_TYPE_LINK,  name, true, CDP_P(source), NULL)


// Accessing data
void*    cdp_record_data(const cdpRecord* record);
#define  cdp_record_value(r)       (*(cdpValue*)cdp_record_data(r))

void* cdp_record_update(cdpRecord* record, size_t size, size_t capacity, cdpValue value, bool swap);
#define cdp_record_update_value(r, z, v)    cdp_record_update(r, (z), sizeof(cdpValue), CDP_V(v), false)
#define cdp_record_update_attribute(r, a)   do{ assert(cdp_record_has_data(r);  (r)->data.attribute = (a); }while(0)

static inline void cdp_record_delete_data(cdpRecord* record)        {if (cdp_record_has_data(record))  {cdp_data_del(record->data);   record->data  = NULL;}}
static inline void cdp_record_delete_store(cdpRecord* record)       {if (cdp_record_has_store(record)) {cdp_store_del(record->store); record->store = NULL;}}
static inline void cdp_record_delete_children(cdpRecord* record)    {assert(cdp_record_has_store(record));  cdp_store_delete_children(record->store);}
#define cdp_record_delete(r)    cdp_record_remove(r, NULL)


// Constructs the full path (sequence of ids) for a given record, returning the depth
bool cdp_record_path(const cdpRecord* record, cdpPath** path);


// Accessing branched records
cdpRecord* cdp_record_first(const cdpRecord* record);
cdpRecord* cdp_record_last (const cdpRecord* record);

cdpRecord* cdp_record_find_by_name(const cdpRecord* record, cdpID name);
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context);
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t position);
cdpRecord* cdp_record_find_by_path(const cdpRecord* start, const cdpPath* path);

cdpRecord* cdp_record_prev(const cdpRecord* record, cdpRecord* child);
cdpRecord* cdp_record_next(const cdpRecord* record, cdpRecord* child);

cdpRecord* cdp_record_find_next_by_name(const cdpRecord* record, cdpID id, uintptr_t* childIdx);
cdpRecord* cdp_record_find_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev);

bool cdp_record_traverse     (cdpRecord* record, cdpTraverse func, void* context, cdpEntry* entry);
bool cdp_record_deep_traverse(cdpRecord* record, cdpTraverse func, cdpTraverse listEnd, void* context, cdpEntry* entry);


// Converts an unsorted record into a sorted one
void cdp_record_to_dictionary(cdpRecord* record);
void cdp_record_sort(cdpRecord* record, cdpCompare compare, void* context);


// Removing records
bool cdp_record_child_take(cdpRecord* record, cdpRecord* target);
bool cdp_record_child_pop(cdpRecord* record, cdpRecord* target);
void cdp_record_remove(cdpRecord* record, cdpRecord* target);


// Converting C text strings to/from cdpID
cdpID  cdp_text_to_acronysm(const char *s);
size_t cdp_acronysm_to_text(cdpID acro, char s[10]);

cdpID  cdp_text_to_word(const char *s);
size_t cdp_word_to_text(cdpID coded, char s[12]);


/*
    TODO:
    - Implement clone (deep copy) records.
    - Traverse book in internal (stoTech) order.
    - Add indexof for records.
    - Update MAX_DEPTH based on path/traverse operations.
    - Add cdp_record_update_nested_links(old, new).
    - If a record is added with its name explicitly above "auto_id", then that must be updated.
    - Send simultaneous tasks to nested agent records.
*/


#endif
