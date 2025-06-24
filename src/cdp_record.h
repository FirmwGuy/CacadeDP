/*
 *  Copyright (c) 2024-2025 Victor M. Barrientos
 *  (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
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
      fully fits inside a quadrant or not.
*/


typedef struct _cdpData       cdpData;
typedef struct _cdpStore      cdpStore;
typedef struct _cdpRecord     cdpRecord;

typedef int (*cdpCompare)(const cdpRecord* restrict, const cdpRecord* restrict, void*);


/*
 *  Domain-Tag (DT) Naming
 */

typedef uint64_t  cdpID;

#define CDP_ID(v)   ((cdpID)(v))

#define CDP_NAME_BITS           58
#define CDP_NAME_MAXVAL         (~(((cdpID)(-1)) << CDP_NAME_BITS))
#define CDP_NAMECONV_BITS       2
#define CDP_AUTOID_BITS         (CDP_NAME_BITS - CDP_NAMECONV_BITS)
#define CDP_AUTOID_MAXVAL       (~(((cdpID)(-1)) << CDP_AUTOID_BITS))
#define CDP_AUTOID_MAX          (CDP_AUTOID_MAXVAL - 1)

typedef struct {
    struct {
        cdpID           _sysbits1:  6,  // Used by other parts of CDP system.
                        domain:     CDP_NAME_BITS;
    };
    struct {
        cdpID           _sysbits2:  6,  // Used by other parts of CDP system.
                        tag:        CDP_NAME_BITS;
    };
} cdpDT;

#define CDP_DT(p)       ((cdpDT*)(p))

static inline int cdp_dt_compare(const cdpDT* restrict key, const cdpDT* restrict dt)
{
    if (key->domain > dt->domain)
        return 1;
    if (key->domain < dt->domain)
        return -1;
    if (key->tag > dt->tag)
        return 1;
    if (key->tag < dt->tag)
        return -1;
    
    return 0;
}


/*
 *  Record Meta
 */

typedef struct {
  union {
    cdpDT       _dt;
    
    struct {
      struct {
        cdpID   type:       2,    // Type of record (dictionary, link, etc).
                hidden:     1,    // Record won't appear on listings (it can only be accessed directly).
                shadowing:  2,    // If record has shadowing records (links pointing to it).
                _unused:    1,

                domain:     CDP_NAME_BITS;
      };
      struct {
        cdpID   _reserved:  6,
                tag:        CDP_NAME_BITS;
      };
    };
  };
} cdpMetarecord;

static_assert(sizeof(cdpMetarecord) == sizeof(cdpDT), "System bits can't exceed 2 pairs of 6 bits!");


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
    CDP_NAMING_ACRONYM,        // Uppercase/numeric text, 9 characters maximum.
    CDP_NAMING_REFERENCE,       // Numerical reference to text record (a pointer in 32bit systems).
    CDP_NAMING_NUMERIC,         // Per-parent numerical ID.

    CDP_NAMING_COUNT
};


#define cdp_id_from_naming(naming)      (((cdpID)((naming) & 3)) << CDP_AUTOID_BITS)
#define CDP_NAMING_MASK                 cdp_id_from_naming(3)

#define cdp_id_to_word(word)            ((word) | cdp_id_from_naming(CDP_NAMING_WORD))
#define cdp_id_to_acronym(acro)         ((acro) | cdp_id_from_naming(CDP_NAMING_ACRONYM))
#define cdp_id_to_reference(ref)        ((ref)  | cdp_id_from_naming(CDP_NAMING_REFERENCE))
#define cdp_id_to_numeric(numb)         ((numb) | cdp_id_from_naming(CDP_NAMING_NUMERIC))

#define cdp_id(name)                    ((name) & CDP_AUTOID_MAXVAL)
#define CDP_ID_SET(name, id)            do{ name = ((name) & CDP_NAMING_MASK) | cdp_id(id); }while(0)

#define CDP_AUTOID_USE                  CDP_AUTOID_MAXVAL
#define CDP_AUTOID                      cdp_id_to_numeric(CDP_AUTOID_USE)

#define cdp_id_is_auto(name)            ((name) == CDP_AUTOID)
#define cdp_id_is_word(name)            ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_WORD))
#define cdp_id_is_acronym(name)         ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_ACRONYM))
#define cdp_id_is_reference(name)       ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_REFERENCE))
#define cdp_id_is_numeric(name)         ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_NUMERIC))

#define cdp_id_valid(id)                (cdp_id(id) && ((id) <= CDP_AUTOID))
#define cdp_id_text_valid(name)         (cdp_id(name) && !cdp_id_is_numeric(name))
#define cdp_id_naming(name)             (((name) >> CDP_AUTOID_BITS) & 3)

#define cdp_dt_valid(dt)                (cdp_id_text_valid((dt)->domain) && cdp_id_valid((dt)->tag))


// Converting C text strings to/from cdpID

/* Acronym character chart (ASCII lower set):
 H \  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
 - -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
2x \  SP  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
3x \  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
4x \  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
5x \  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
*/
#define CDP_ACRON_MAX_CHARS     9

#define CDP_TEXT_TO_ACRONYM_(name)                                             \
    cdpID name(const char *s) {                                                \
        assert(s && *s);                                                       \
                                                                               \
        while (*s == ' ') {                                                    \
            s++;            /* Trim leading spaces. */                         \
        }                                                                      \
        if (!*s)                                                               \
            return 0;                                                          \
                                                                               \
        size_t len = strlen(s);                                                \
        while (len > 0  &&  s[len - 1] == ' ') {                               \
            len--;          /* Trim trailing spaces. */                        \
        }                                                                      \
                                                                               \
        if (len > CDP_ACRON_MAX_CHARS)                                         \
            return 0;       /* Limit to max allowed characters. */             \
                                                                               \
        cdpID coded = 0;                                                       \
        for (size_t n = 0; n < len; n++) {                                     \
            char c = s[n];                                                     \
                                                                               \
            if (c < 0x20  ||  c > 0x5F)                                        \
                return 0;   /* Uncodable characters. */                        \
                                                                               \
            /* Shift and encode each character: */                             \
            coded |= (cdpID)(c - 0x20) << (6 * ((CDP_ACRON_MAX_CHARS-1) - n)); \
        }                                                                      \
                                                                               \
        return cdp_id_to_acronym(coded);                                       \
    }


/* Word character chart (ASCII upper set):
 H \  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
 - -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
6x \ [SP] a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
7x \  p   q   r   s   t   u   v   w   x   y   z  [:] [_] [-] [.] [/]
    Note: characters in square brackets replacing originals.
*/
#define CDP_WORD_MAX_CHARS      11

#define CDP_TEXT_TO_WORD_(name)                                                \
    cdpID name(const char *s) {                                                \
        assert(s && *s);                                                       \
                                                                               \
        while (*s == ' ') {                                                    \
            s++;            /* Trim leading spaces. */                         \
        }                                                                      \
        if (!*s)                                                               \
            return 0;                                                          \
                                                                               \
        size_t len = strlen(s);                                                \
        while (len > 0  &&  s[len - 1] == ' ') {                               \
            len--;          /* Trim trailing spaces. */                        \
        }                                                                      \
        if (len > CDP_WORD_MAX_CHARS)                                          \
            return 0;       /* Limit to max allowed characters. */             \
                                                                               \
        bool hasLowercase = false;                                             \
        cdpID coded = 0;                                                       \
        for (size_t n = 0; n < len; n++) {                                     \
            char c = s[n];                                                     \
                                                                               \
            uint8_t encoded_char;                                              \
            if (c >= 0x61  &&  c <= 0x7A) {                                    \
                encoded_char = c - 0x61 + 1;        /* Map 'a'-'z' to 1-26. */ \
                hasLowercase = true;                                           \
            } else switch (c) {                                                \
              case ' ': encoded_char = 0;   break;  /* Encode space as 0. */   \
              case ':': encoded_char = 27;  break;                             \
              case '_': encoded_char = 28;  break;                             \
              case '-': encoded_char = 29;  break;                             \
              case '.': encoded_char = 30;  break;                             \
              case '/': encoded_char = 31;  break;                             \
                                                                               \
              default:                                                         \
                return 0;   /* Uncodable characters. */                        \
            }                                                                  \
                                                                               \
            /* Shift and encode each character: */                             \
            coded |= (cdpID)encoded_char << (5 * ((CDP_WORD_MAX_CHARS-1) - n));\
        }                                                                      \
        if (!hasLowercase)                                                     \
            return 0;       /* Can't be a pure symbolic word. */               \
                                                                               \
        return cdp_id_to_word(coded);                                          \
    }

static inline CDP_CONST_FUNC CDP_TEXT_TO_ACRONYM_(CDP_ACRO_constant)
static inline CDP_CONST_FUNC CDP_TEXT_TO_WORD_(CDP_WORD_constant)

#define CDP_ACRO(s)     ({static_assert(strlen(s) > 0 && strlen(s) <= 9,  "Acronym IDs must be 9 characters or less!"); CDP_ACRO_constant(s);})
#define CDP_WORD(s)     ({static_assert(strlen(s) > 0 && strlen(s) <= 11, "Word IDs must be 11 characters or less!"); CDP_WORD_constant(s);})

#define CDP_DTS(d, t)   (&(cdpDT){.domain=(d), .tag=(t)})
#define CDP_DTWW(d, t)  CDP_DTS(CDP_WORD(d), CDP_WORD(t))
#define CDP_DTWA(d, t)  CDP_DTS(CDP_WORD(d), CDP_ACRO(t))
#define CDP_DTAA(d, t)  CDP_DTS(CDP_ACRO(d), CDP_ACRO(t))
#define CDP_DTAW(d, t)  CDP_DTS(CDP_ACRO(d), CDP_WORD(t))

cdpID  cdp_text_to_acronym(const char *s);
size_t cdp_acronym_to_text(cdpID acro, char s[10]);

cdpID  cdp_text_to_word(const char *s);
size_t cdp_word_to_text(cdpID coded, char s[12]);


/*
    Record Data
*/

#define CDP_ATTRIBUTE_STRUCT(name, ...)                                        \
    typedef union {                                                            \
        struct {                                                               \
          cdpID                                                                \
            _type:      2,      /* 0: Virtual,      1: Physical,    2: Conceptual,  3: Hybrid/Other. */\
            _active:    1,      /* 0: Static,       1: Active/Alive/Operational. */\
            _interact:  1,      /* 0: Passive,      1: Interactive.     */     \
            _mutable:   1,      /* 0: Immutable,    1: Can change.      */     \
            _relational:1,      /* 0: Isolated,     1: Relational.      */     \
            _complex:   1,      /* 0: Simple,       1: Complex.         */     \
            _infinite:  1,      /* 0: Finite,       1: Infinite.        */     \
            _temporal:  1,      /* 0: Timeless,     1: Time-bound.      */     \
            _mobile:    1,      /* 0: Stationary,   1: Mobile.          */     \
            _natural:   1,      /* 0: Artificial.   1: Natural.         */     \
            _incidental:1,      /* 0: Purposeful,   1: Incidental.      */     \
            _autonomous:1,      /* 0: Dependent,    1: Autonomous.      */     \
            _needenergy:1,      /* 0: No need,      1: Needs energy.    */     \
            ##__VA_ARGS__                                                      \
            ;                                                                  \
        };                                                                     \
        cdpID         _id;                                                     \
    } name;                                                                    \
    static_assert(sizeof(name) == sizeof(cdpID), "Too many attribute flags!")

enum _cdpThingType {
    CDP_THING_TYPE_VIRTUAL,
    CDP_THING_TYPE_PHYSICAL,
    CDP_THING_TYPE_CONCEPTUAL,
    CDP_THING_TYPE_OTHER = 3
};

CDP_ATTRIBUTE_STRUCT(cdpAttribute, _domain_flags: 50);


struct _cdpData {
    union {
      cdpDT             _dt;
      
      struct {
        struct {
          cdpID         datatype:   2,  // Type of data (see _cdpDataType).
                        _unused:    4,

                        domain:     CDP_NAME_BITS;
        };
        struct {
          cdpID         writable:   1,  // If data can be updated.
                        lock:       1,  // Lock on data content.
                        _reserved:  4,
                        
                        tag:        CDP_NAME_BITS;
        };
      };
    };

    cdpAttribute        attribute;      // Data attributes (it depends on domain).

    cdpID               encoding;       // Binary encoding id.
    size_t              size;           // Data size in bytes.
    size_t              capacity;       // Buffer capacity in bytes.

    cdpData*            next;           // Pointer to next data representation (if available).

    uint64_t            hash;           // Hash value of content.
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
        uint8_t         value[2 * sizeof(void*)];  // Data value may start from here.
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


cdpData* cdp_data_new(  cdpDT* dt, cdpID encoding, cdpID attribute,
                        unsigned datatype, bool writable,
                        void** dataloc, void* value, ...  );
void     cdp_data_del(cdpData* data);
void*    cdp_data(const cdpData* data);
#define  cdp_data_valid(d)                                      ((d) && (d)->capacity && cdp_dt_valid(CDP_DT(d)))
#define  cdp_data_new_value(dt, e, a, value, z, capacity)       cdp_data_new(dt, e, CDP_ID(a), CDP_DATATYPE_VALUE, true, NULL, value, z, capacity)


/*
    Record Storage (for children)
*/

typedef struct {
    unsigned        count;      // Number of record pointers.
    unsigned        capacity;   // Capacity of array.
    cdpRecord*      record[];   // Array of records shadowing this one.
} cdpShadow;


struct _cdpStore {
    union {
      cdpDT         _dt;
      
      struct {
        struct {
        cdpID       storage:    3,              // Data structure for children storage (array, linked-list, etc).
                    indexing:   2,              // Indexing (sorting) criteria for children.
                    _unused:    1,

                    domain:     CDP_NAME_BITS;
        };
        struct {
        cdpID       writable:   1,              // If chidren can be added/deleted.
                    lock:       1,              // Lock on children operations.
                    _reserved:  4,

                    tag:        CDP_NAME_BITS;
        };
      };
    };

    cdpAttribute    attribute;  // Structure attributes (it depends on domain). This is used *only* if record has no data (ie, as a pure directory).

    cdpRecord*      owner;      // Record owning this child storage.
    union {
        cdpRecord*  linked;     // A linked shadow record (when children, see in cdpRecord otherwise).
        cdpShadow*  shadow;     // Shadow structure (if record has children).
    };

    cdpStore*       next;       // Next attribute/index/storage (requires hard links?).

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


cdpStore* cdp_store_new(cdpDT* dt, unsigned storage, unsigned indexing, ...);
void      cdp_store_del(cdpStore* store);
void      cdp_store_delete_children(cdpStore* store);
#define   cdp_store_valid(s)      ((s) && cdp_dt_valid(CDP_DT(s)))

static inline bool cdp_store_is_insertable(cdpStore* store)   {assert(cdp_store_valid(store));  return (store->indexing == CDP_INDEX_BY_INSERTION);}
static inline bool cdp_store_is_dictionary(cdpStore* store)   {assert(cdp_store_valid(store));  return (store->indexing == CDP_INDEX_BY_NAME);}
static inline bool cdp_store_is_f_sorted(cdpStore* store)     {assert(cdp_store_valid(store));  return (store->indexing == CDP_INDEX_BY_FUNCTION  ||  store->indexing == CDP_INDEX_BY_HASH);}
static inline bool cdp_store_is_sorted(cdpStore* store)       {assert(cdp_store_valid(store));  return (store->indexing != CDP_INDEX_BY_INSERTION);}
static inline bool cdp_store_is_empty(cdpStore* store)        {assert(cdp_store_valid(store));  return !store->chdCount;}

cdpRecord* cdp_store_add_child(cdpStore* store, uintptr_t context, cdpRecord* child);
cdpRecord* cdp_store_append_child(cdpStore* store, bool prepend, cdpRecord* child);


/*
    Record
*/

typedef struct {
    unsigned        length;
    unsigned        capacity;
    cdpDT           dt[];
} cdpPath;


struct _cdpRecord {
    cdpMetarecord   metarecord; // Meta about this record entry (including name (DT), system bits, etc).
    cdpStore*       parent;     // Parent structure (list, array, etc) where this record is stored in.

    union {
        cdpData*    data;       // Address of cdpData structure.

        cdpRecord*  link;       // Link to another record.
    };

    union {
        cdpStore*   store;      // Address of cdpStore structure.

        cdpRecord*  linked;     // A linked shadow record (if no children, see in cdpStore otherwise).
        cdpShadow*  shadow;     // Structure for multiple linked records (if no children).

        //cdpRecord*  instance;   // Agent instance this record belongs to (if record is a Link).
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

// Initiate records
void cdp_record_initialize(cdpRecord* record, unsigned type, cdpDT* name, cdpData* data, cdpStore* store);
void cdp_record_initialize_clone(cdpRecord* newClone, cdpDT* name, cdpRecord* record);
void cdp_record_finalize(cdpRecord* record);

#define cdp_record_initialize_value(r, name, dt, encoding, attrib, value, size, capacity)               cdp_record_initialize(r, CDP_TYPE_NORMAL, name, cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_VALUE, true, NULL, value, size, capacity), NULL)
#define cdp_record_initialize_data(r, name, dt, encoding, attrib, value, size, capacity, destructor)    cdp_record_initialize(r, CDP_TYPE_NORMAL, name, cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_DATA,  true, NULL, value, size, capacity, destructor), NULL)

#define cdp_record_initialize_list(r, name, dt, storage, ...)                  cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_initialize_dictionary(r, name, dt, storage, ...)            cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_initialize_catalog(r, name, dt, storage, ...)               cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))
#define cdp_record_initialize_spatial(r, name, dt, center, subwide, compare)   cdp_record_initialize(r, CDP_TYPE_NORMAL, name, NULL, cdp_store_new(dt, CDP_STORAGE_OCTREE, CDP_INDEX_BY_FUNCTION, center, subwide, compare))

static inline void  cdp_record_set_tag_id(cdpRecord* record, cdpID id)      {assert(record && cdp_id_valid(id));  CDP_ID_SET(record->metarecord.tag, id);}
static inline void  cdp_record_set_name(cdpRecord* record, cdpDT* name)     {assert(record && cdp_dt_valid(name));  record->metarecord.domain = name->domain; record->metarecord.tag = name->tag;}    // FixMe: mask 'name->tag' before assignation.
//static inline cdpDT cdp_record_get_name(const cdpRecord* record)        {assert(record);  return record->metarecord.name;}
#define cdp_record_get_tag_id(r)    cdp_id(CDP_DT(r)->tag)

#define cdp_record_is_void(r)       (((r)->metarecord.type == CDP_TYPE_VOID) || !cdp_dt_valid(&(r)->metarecord._dt))
#define cdp_record_is_normal(r)     ((r)->metarecord.type == CDP_TYPE_NORMAL)
#define cdp_record_is_flex(r)       ((r)->metarecord.type == CDP_TYPE_FLEX)
#define cdp_record_is_link(r)       ((r)->metarecord.type == CDP_TYPE_LINK)

#define cdp_record_is_shadowed(r)   ((r)->metarecord.shadowing)
#define cdp_record_is_private(r)    ((r)->metarecord.priv)
#define cdp_record_is_system(r)     ((r)->metarecord.system)

static inline bool cdp_record_has_data(const cdpRecord* record)     {assert(cdp_record_is_normal(record));  return record->data;}
static inline bool cdp_record_has_store(const cdpRecord* record)    {assert(cdp_record_is_normal(record));  return record->store;}

static inline void cdp_record_set_data(cdpRecord* record, cdpData* data)      {assert(!cdp_record_has_data(record) && cdp_data_valid(data));   record->data = data;}
static inline void cdp_record_set_store(cdpRecord* record, cdpStore* store)   {assert(!cdp_record_has_store(record) && cdp_store_valid(store));  store->owner = record; record->store = store;}

static inline cdpRecord* cdp_record_parent  (const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->parent)? record->parent->owner: NULL;}
static inline size_t     cdp_record_siblings(const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->parent)? record->parent->chdCount: 0;}
static inline size_t     cdp_record_children(const cdpRecord* record)   {assert(record);  return cdp_record_has_store(record)? record->store->chdCount: 0;}

#define cdp_record_id_is_pending(r)   cdp_id_is_auto((r)->metarecord.tag)
static inline void  cdp_record_set_autoid(const cdpRecord* record, cdpID id)  {assert(cdp_record_has_store(record) && (record->store->autoid < id)  &&  (id <= CDP_AUTOID_MAX)); record->store->autoid = id;}
static inline cdpID cdp_record_get_autoid(const cdpRecord* record)            {assert(cdp_record_has_store(record));  return record->store->autoid;}

#define cdp_record_name_is(record, name)        (0 == cdp_dt_compare(CDP_DT(&(record)->metarecord), CDP_DT(name)))
#define cdp_record_get_name(record)             (&(record)->metarecord._dt)     /* FixMe: filter sysbits out? */


static inline void cdp_record_relink_storage(cdpRecord* record)     {assert(cdp_record_has_store(record));  record->store->owner = record;}     // Re-links record with its own children storage.

static inline void cdp_record_transfer(cdpRecord* src, cdpRecord* dst) {
    assert(!cdp_record_is_void(src) && dst);

    *dst = *src;

    if (!cdp_record_is_link(dst) && dst->store) {
        dst->store->owner = dst;

        if (dst->store->chdCount)
            cdp_record_relink_storage(dst);
    }

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

static inline void cdp_link_initialize(cdpRecord* link, cdpDT* name, cdpRecord* target) {
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
static inline bool cdp_record_is_unset(cdpRecord* record)       {assert(cdp_record_is_normal(record));  return (!record->data && !record->store);}
static inline bool cdp_record_is_floating(cdpRecord* record)    {assert(record);  return (cdp_record_is_void(record)  ||  (!cdp_record_parent(record) && (record != cdp_root())));}


// Appends/prepends or inserts a (copy of) record into another record
cdpRecord* cdp_record_add(cdpRecord* record, uintptr_t context, cdpRecord* child);
cdpRecord* cdp_record_append(cdpRecord* record, bool prepend, cdpRecord* child);

#define cdp_record_add_child(record, type, name, context, data, store)         \
    ({cdpRecord child__={0}; cdp_record_initialize(&child__, type, name, data, store); cdp_record_add(record, context, &child__);})

#define cdp_record_add_empty(record, name, context)                                                             cdp_record_add_child(record, CDP_TYPE_NORMAL, name, (uintptr_t)(context), NULL, NULL)
#define cdp_record_add_value(record, name, context, dt, encoding, attrib, value, size, capacity)                cdp_record_add_child(record, CDP_TYPE_NORMAL, name, (uintptr_t)(context), cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_VALUE, true, NULL, value, size, capacity), NULL)
#define cdp_record_add_data(record, name, context, dt, encoding, attrib, value, size, capacity, destructor)     cdp_record_add_child(record, CDP_TYPE_NORMAL, name, (uintptr_t)(context), cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_DATA,  true, NULL, value, size, capacity, destructor), NULL)

#define cdp_record_add_list(record, name, context, dt, storage, ...)                                        cdp_record_add_child(record, CDP_TYPE_NORMAL, name, (uintptr_t)(context), NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_add_dictionary(record, name, context, dt, storage, ...)                                  cdp_record_add_child(record, CDP_TYPE_NORMAL, name, (uintptr_t)(context), NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_add_catalog(record, name, context, dt, storage, ...)                                     cdp_record_add_child(record, CDP_TYPE_NORMAL, name, (uintptr_t)(context), NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))

#define cdp_record_add_link(record, name, context, source)                                                  cdp_record_add_child(record, CDP_TYPE_LINK, name, (uintptr_t)(context), CDP_P(source), NULL)

#define cdp_dict_add(record, child)                                                                         cdp_record_add(record, 0, child)
#define cdp_dict_add_empty(record, name)                                                                    cdp_record_add_empty(record, name, 0)
#define cdp_dict_add_value(record, name, dt, encoding, attrib, value, size, capacity)                       cdp_record_add_value(record, name, 0, dt, encoding, attrib, value, size, capacity)
#define cdp_dict_add_data(record, name, dt, encoding, attrib, value, size, capacity, destructor)            cdp_record_add_data(record, name, 0, dt, encoding, attrib, value, size, capacity, destructor)
#define cdp_dict_add_list(record, name, dt, storage, ...)                                                   cdp_record_add_list(record, name, 0, dt, storage, ##__VA_ARGS__)
#define cdp_dict_add_dictionary(record, name, dt, storage, ...)                                             cdp_record_add_dictionary(record, name, 0, dt, storage, ##__VA_ARGS__)
#define cdp_dict_add_catalog(record, name, dt, storage, ...)                                                cdp_record_add_catalog(record, name, 0, dt, storage, ##__VA_ARGS__)
#define cdp_dict_add_link(record, name, source)                                                             cdp_record_add_link(record, name, 0, source)

#define cdp_record_append_child(record, type, name, prepend, data, store)      \
    ({cdpRecord child__={0}; cdp_record_initialize(&child__, type, name, data, store); cdp_record_append(record, prepend, &child__);})

#define cdp_record_append_empty(record, name)                                                               cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, NULL, NULL)
#define cdp_record_append_value(record, name, dt, encoding, attrib, value, size, capacity)                  cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_VALUE, true, NULL, value, size, capacity), NULL)
#define cdp_record_append_data(record, name, dt, encoding, attrib, value, size, capacity, destructor)       cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_DATA,  true, NULL, value, size, capacity, destructor), NULL)

#define cdp_record_append_list(record, name, dt, storage, ...)                                              cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_append_dictionary(record, name, dt, storage, ...)                                        cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_append_catalog(record, name, dt, storage, ...)                                           cdp_record_append_child(record, CDP_TYPE_NORMAL, name, false, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))

#define cdp_record_append_link(record, name, source)                                                        cdp_record_append_child(record, CDP_TYPE_LINK, name, false, CDP_P(source), NULL)

#define cdp_record_prepend_empty(record, name)                                                              cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, NULL, NULL)
#define cdp_record_prepend_value(record, name, dt, encoding, attrib, value, size, capacity)                 cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_VALUE, true, NULL, value, size, capacity), NULL)
#define cdp_record_prepend_data(record, name, dt, encoding, attrib, value, size, capacity, destructor)      cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, cdp_data_new(dt, encoding, attrib, CDP_DATATYPE_DATA,  true, NULL, value, size, capacity, destructor), NULL)

#define cdp_record_prepend_list(record, name, dt, storage, ...)                                             cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_INSERTION, ##__VA_ARGS__))
#define cdp_record_prepend_dictionary(record, name, dt, storage, ...)                                       cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_NAME, ##__VA_ARGS__))
#define cdp_record_prepend_catalog(record, name, dt, storage, ...)                                          cdp_record_append_child(record, CDP_TYPE_NORMAL, name, true, NULL, cdp_store_new(dt, storage, CDP_INDEX_BY_FUNCTION, ##__VA_ARGS__))

#define cdp_record_prepend_link(record, name, source)                                                       cdp_record_append_child(record, CDP_TYPE_LINK, name, true, CDP_P(source), NULL)


// Accessing data
void* cdp_record_data(const cdpRecord* record);

void* cdp_record_update(cdpRecord* record, size_t size, size_t capacity, void* value, bool swap);
#define cdp_record_update_value(r, z, v)    cdp_record_update(r, (z), sizeof(*(v)), v, false)
#define cdp_record_update_attribute(r, a)   do{ assert(cdp_record_has_data(r);  (r)->data.attribute.id = CDP_ID(a); }while(0)

static inline void cdp_record_delete_data(cdpRecord* record)        {if (cdp_record_has_data(record))  {cdp_data_del(record->data);   record->data  = NULL;}}
static inline void cdp_record_delete_store(cdpRecord* record)       {if (cdp_record_has_store(record)) {cdp_store_del(record->store); record->store = NULL;}}
static inline void cdp_record_delete_children(cdpRecord* record)    {assert(cdp_record_has_store(record));  cdp_store_delete_children(record->store);}
#define cdp_record_delete(r)    cdp_record_remove(r, NULL)


// Constructs the full path (sequence of ids) for a given record, returning the depth
bool cdp_record_path(const cdpRecord* record, cdpPath** path);


// Accessing branched records
cdpRecord* cdp_record_first(const cdpRecord* record);
cdpRecord* cdp_record_last (const cdpRecord* record);

cdpRecord* cdp_record_find_by_name(const cdpRecord* record, const cdpDT* name);
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context);
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t position);
cdpRecord* cdp_record_find_by_path(const cdpRecord* start, const cdpPath* path);

cdpRecord* cdp_record_prev(const cdpRecord* record, cdpRecord* child);
cdpRecord* cdp_record_next(const cdpRecord* record, cdpRecord* child);

cdpRecord* cdp_record_find_next_by_name(const cdpRecord* record, cdpDT* name, uintptr_t* childIdx);
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


// Initiate and shutdown record system
void cdp_record_system_initiate(void);
void cdp_record_system_shutdown(void);


/*
    TODO:
    - Implement range queries (between a minimum and a maximum key) for records.
    - Implement clone (deep copy) records.
    - Traverse book in internal (stoTech) order.
    - Add indexof for records.
    - Update MAX_DEPTH based on path/traverse operations.
    - Add cdp_record_update_nested_links(old, new).
    - If a record is added with its name explicitly above "auto_id", then that must be updated.
    - Send simultaneous tasks to nested agent records.
*/


#endif
