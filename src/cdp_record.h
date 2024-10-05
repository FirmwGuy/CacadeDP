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


/* ### **cdpMetadata: A 64-Bit Attribute Encoding System**

The **cdpMetadata** system is a powerful and scalable **64-bit encoding
framework** designed to universally describe entities, actions, data
structures, and concepts across diverse domains. By providing a flexible
structure that divides its 64 bits into defined segments, the system enables
concise yet richly descriptive metadata that can efficiently encode everything
from natural language elements to complex graphical user interfaces,
programming constructs, and scientific data.

### **Core Structure of cdpMetadata**

The system's 64-bit structure is divided into five core components,
each serving a critical function in the encoding process:

1. **Domain Selector (7 bits)**: Identifies the knowledge domain (up to
128 domains).
2. **Role (3 bits)**: Defines the functional category or type
of entity within its domain (up to 8, with potential for extension).
3. **Entity ID (16 bits)**: Uniquely identifies specific entities or
structures within a domain (64k).
4. **Universal Attributes (6 bits)**: Captures 6 general, cross-domain
characteristics that apply to entities regardless of domain.
5. **Domain-Specific Attributes (32 bits)**: Provides detailed and
context-specific information about entities within a particular domain
using enumerations and flags.1111111111111111

The objective of this fields is not to provide quantities (values) but
rather qualities of the data being represented. Values are intended to be
stored in child records.

---

### **1. Domain Selector (7 bits)**

The **Domain Selector** is a 7-bit segment that identifies the specific
domain of knowledge that an entity or concept belongs to. With **128
possible domains**, this ensures broad coverage across human knowledge,
with the potential for adding more fields in the future.

#### **Examples of Domains**:

| **ID** | **Domain Name**                  | **Description**                                                    |
|:------:|:---------------------------------|:-------------------------------------------------------------------|
| `2`    | Binary Data Domain               | Buffers, memory blocks, data structures, and binary files.         |
| `3`    | GUI and Interface                | Components like buttons, text fields, windows, and sliders in UIs. |
| `7`    | Video Game Engines               | Entities, mechanics, and objects within video game development.    |
| `9`    | Physics                          | Forces, particles, physical fields, and equations of motion.       |
| `20`   | Natural Language                 | Words, grammar, and linguistic structures.                         |
| `21`   | Mathematics                      | Numbers, functions, and equations.                                 |
| `22`   | Programming & Softw. Engineering | Data types, functions, and algorithms in software.                 |

---

### **2. Role (3 bits, extendable)**

The **Role** defines the type or function of the entity within its
domain. This 3-bit field allows for up to **8 primary roles** within
each domain, but can be extended in complex domains by borrowing bits
from the domain-specific attributes. These roles can represent common
linguistic categories (e.g., nouns, verbs) or domain-specific types
like data structures in programming, or UI components in a graphical
interface.

#### **Role Examples** (common across domains):

| **Role** | **Description**                          |
|----------|------------------------------------------|
| `0`      | Entity/Container                         |
| `1`      | Action/Operation                         |
| `2`      | Input/Output Element                     |
| `3`      | Visual Component                         |
| `4`      | State/Condition                          |
| `5`      | Control/Interactive Element              |
| `6`      | Data Structure                           |
| `7`      | Metadata/Annotation                      |

By borrowing bits from **domain-specific attributes**, this can expand
to support specialized sub-roles in domains where greater specificity
is required, such as distinguishing between various kinds of input
fields in a GUI domain or more nuanced data types in a programming
domain.

---

### **3. Entity ID (16 bits)**

The **Entity ID** provides a unique identifier for each entity,
concept, or object within a domain. With **16 bits**, this allows for
**65,536 unique entities** in each domain, providing extensive coverage
for representing words, objects, data structures, or components.

- **In the Natural Language Domain**, an Entity ID could represent a
specific word (e.g., "run").
- **In the GUI Domain**, an Entity ID could represent a specific button
or interactive widget.
- **In the Programming Domain**, it might represent a specific data
type like an integer or an array.

This field allows the system to handle specific objects within the
broader context of their domain and role.

---

### **4. Universal Attributes (6 bits)**

The **Universal Attributes** are a set of 6 bits that apply across all
domains and describe general characteristics of entities. These
attributes are **consistent across domains**, ensuring that any entity
can be evaluated for certain high-level properties.

| **Bit** | **Attribute Name**            | **Description**                                                            |
|--------|-------------------------------|----------------------------------------------------------------------------|
| 0      | **Concrete/Abstract**          | Is the entity a concrete object (0) or an abstract concept (1)?             |
| 1      | **Physical/Virtual/Conceptual**| Is it physical (0), virtual (1), or conceptual (1)?                         |
| 2      | **Active/Static**              | Is the entity operational or alive (1) or static/inactive (0)?              |
| 3      | **Interactive**                | Can the entity be interacted with (1) or is it passive (0)?                 |
| 4      | **Mutable/Immutable**          | Is the entity changeable (1) or immutable (0)?                              |
| 5      | **Relational**                 | Does the entity form part of a relationship or structure with other entities (1)? |

This common set of attributes makes it easier to manage and interpret
entities across domains, such as understanding whether an object is
mutable in both the programming and multimedia domains.

---

### **5. Domain-Specific Attributes (32 bits)**

The **Domain-Specific Attributes** are the most flexible part of the
system, allowing for detailed, domain-specific flags and enumerations.
These **32 bits** provide domain-specific information that is unique to
the field in question and captures essential characteristics that can't
be described through universal attributes alone.

The objective of this 32 bits is to store static information that won't
change during execution lifetime as a form of type description, in
order to avoid type-if-then hell during coding. For example, instead of
asking if a number representation belongs to Float32, Float64 or
Float128 we can just ask if the role is "Float".

- **In the Natural Language Domain**, domain-specific attributes could
describe word forms (e.g., tense, number, case).
- **In the GUI Domain**, these attributes could describe visual
properties (e.g., size, color, alignment).
- **In the Programming Domain**, they could describe data types, memory
allocation, or access.

#### Example Breakdown (for GUI Domain):

| **Bit Range**  | **Attribute**                | **Description**                                                        |
|---------------|------------------------------|------------------------------------------------------------------------|
| 0-3           | **Element Type**              | Identifies the specific type of GUI element (e.g., button, text field). |
| 4-7           | **Size/Style**                | Defines size or style (e.g., large, small, rounded, flat).              |
| 8-11          | **Alignment**                 | Describes horizontal or vertical alignment (e.g., left, center, right). |
| 12-15         | **Layout Type**               | Defines the layout of containers (e.g., grid, flow, vertical).          |
| 16-21         | **Interaction Type**          | Flags for user interaction types (e.g., click, hover, drag/drop).       |
| 22-23         | **Scrollability**             | Describes if the element can scroll or is paged.                        |
| 24-31         | **Visual State/Feedback**     | Visual cues and feedback mechanisms (e.g., highlighted, selected, ripple). |

This structure allows for maximum flexibility while remaining compact
and domain-appropriate.

*/

typedef uint16_t  cdpTag;
typedef uint32_t  cdpAttribute;

#define CDP_DOMAIN_BITS     7
#define CDP_ROLE_BITS       3
//#define CDP_TAG_BITS        cdp_bitsof(cdpTag)
#define CDP_TAG_MAXVAL      ((cdpTag)-1)
//#define CDP_ATTRIB_BITS     cdp_bitsof(cdpAttribute)

typedef struct {
    union {
      cdpAttribute  _head;                          // The header attributes (tag, domain, etc) as a single value.
      struct {
        uint8_t     domain:     CDP_DOMAIN_BITS,    // Domain language selector.

                    abstract:   1;                  // Is a concrete or abstract concept.
        uint8_t     physical:   1,                  // Is a physical, virtual or conceptual idea (depends of abstract flag).
                    alive:      1,                  // Is it alive/operational or is it a static/inactive thing.
                    interactive:1,                  // Can it be interacted with.
                    immutable:  1,                  // Is mutable or immutable.
                    relational: 1,                  // Is it related to (or part of) another thing or is it independent?

                    role:       CDP_ROLE_BITS;      // Role of this data.

        cdpTag      tag;                            // Tag assigned to this record. The lexicon is the same per domain (not per role).
      };
    };
    cdpAttribute    _attribute;                     // Flags/bitfields for domain specific attributes as a single value.
} cdpMetadata;


enum _cdpDomain {
    CDP_DOMAIN_BRANCHE,         // A symbolic domain used to indicate all sorts of purely branched types.
    CDP_DOMAIN_BINARY,
    CDP_DOMAIN_TEXT,
    CDP_DOMAIN_INTERFACE,
    CDP_DOMAIN_MULTIMEDIA,
    CDP_DOMAIN_SIMULATION,
    CDP_DOMAIN_RENDERING,
    CDP_DOMAIN_PHYSICS,
    CDP_DOMAIN_AI,
    //CDP_DOMAIN_USERS,

    CDP_DOMAIN_COUNT
};


#define CDP_ATTRIBUTE_STRUCT(n, s)                                     \
    struct _##n {                                                      \
        s                                                              \
    };                                                                 \
    static_assert(sizeof(cdpAttribute) >= sizeof(_##n));               \
    typedef union {                                                    \
        cdpAttribute  _attribute;                                      \
        struct _##n;                                                   \
    } n;                                                               \
    static_assert(sizeof(cdpAttribute) == sizeof(n))




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

    * Metarecord: Each record contains metadata, including flags that
    specify the record's characteristics, and a name identifier indicating
    the record's role or ID within its parent.

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

typedef uint64_t  cdpID;

#define CDP_ID_BITS             cdp_bitsof(cdpID)
#define CDP_RECD_FLAG_BITS      12
#define CDP_NAME_BITS           (CDP_ID_BITS - CDP_RECD_FLAG_BITS)
#define CDP_NAME_MAXVAL         (~(((cdpID)(-1)) << CDP_NAME_BITS))
#define CDP_NAMECONV_BITS       2
#define CDP_AUTOID_BITS         (CDP_NAME_BITS - CDP_NAMECONV_BITS)
#define CDP_AUTOID_MAXVAL       (~(((cdpID)(-1)) << CDP_AUTOID_BITS))
#define CDP_AUTOID_MAX          (CDP_AUTOID_MAXVAL - 1)

typedef struct {
  union {
    cdpID     _id;
    struct {
      cdpID   storage:    2,    // Data structure for children storage.
              dictionary: 1,    // Children name ids must be unique.

              factual:    1,    // Record can't be modified anymore (but it still can be deleted).
              priv:       1,    // Record (with all its children) is private (unlockable).
              //hidden:     1,    // Data structure for children storage (it depends on the record type).
              //system:     1,    // Record is part of the system and can't be modified or deleted.

              connected:  1,    // Record is connected (it can't skip the signal API).
              baby:       1,    // On receiving any signal this record will first alert its parent.

              recdata:    2,    // Where the data is located.
              withstore:  1,    // Record has child storage (but not necessarily children).
              shadowing:  2,    // If record has shadowing records (links pointing to it).

              name: CDP_NAME_BITS; // Name id of this record instance (the first 2 MSB bits are the naming convention).
    };
  };
} cdpMetarecord;

enum _cdpRecordStorage {
    CDP_STORAGE_LINKED_LIST,    // Children stored in a doubly linked list.
    CDP_STORAGE_ARRAY,          // Children stored in an array.
    CDP_STORAGE_PACKED_QUEUE,   // Children stored in a packed queue (record can't be a dictionary).
    CDP_STORAGE_RED_BLACK_T,    // Children stored in a red-black tree (record must be a dictionary).
    //
    CDP_STORAGE_COUNT
};

enum _cdpRecordData {
    CDP_RECDATA_NONE,           // Record has no data.
    CDP_RECDATA_NEAR,           // Data (small) is inside "_near" field of cdpRecord.
    CDP_RECDATA_DATA,           // Data starts at "_data" field of cdpData.
    CDP_RECDATA_FAR             // Data is in address pointed by "_far" field of cdpData.
};

enum _cdpRecordShadowing {
    CDP_SHADOW_NONE,            // No shadow records.
    CDP_SHADOW_SINGLE,          // Single shadow record.
    CDP_SHADOW_MULTIPLE,        // Multiple shadows.
};


enum _cdpRecordNaming {
    CDP_NAMING_TEXT,            // Cross-domain indexed text id.
    CDP_NAMING_TAG,             // Per-domain indexed text id.
    CDP_NAMING_LOCAL,           // Per-record numerical id.
    CDP_NAMING_GLOBAL,          // Global numerical id (it may be used with a system global autoid).

    CDP_NAMING_COUNT
};

#define cdp_id_from_naming(naming)  (((cdpID)((naming) & 3)) << CDP_AUTOID_BITS)
//#define cdp_id_to_naming(id)        (((id) >> CDP_AUTOID_BITS) & 3)
#define CDP_NAMING_MASK             cdp_id_from_naming(3)

#define cdp_id_to_text(pos)         ((pos) & cdp_id_from_naming(CDP_NAMING_TEXT))
#define cdp_id_to_tag(pos)          ((pos) & cdp_id_from_naming(CDP_NAMING_TAG))
#define cdp_id_local(id)            ((id)  & cdp_id_from_naming(CDP_NAMING_LOCAL))
#define cdp_id_global(id)           ((id)  & cdp_id_from_naming(CDP_NAMING_GLOBAL))

#define cdp_id(name)                ((name) & CDP_AUTOID_MAXVAL)
#define CDP_ID_SET(name, id)        (name = ((name) & CDP_NAMING_MASK) | cdp_id(id))

#define CDP_AUTOID_USE              CDP_AUTOID_MAXVAL
#define CDP_AUTOID_LOCAL            cdp_id_local(CDP_AUTOID_USE)
#define CDP_AUTOID_GLOBAL           cdp_id_global(CDP_AUTOID_USE)

#define cdp_id_name_is_local(name)  ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_LOCAL))
#define cdp_id_name_is_global(name) ((CDP_NAMING_MASK & (name)) == cdp_id_from_naming(CDP_NAMING_GLOBAL))
#define cdp_id_valid(name)          (((name) & ~CDP_NAMING_MASK) <= CDP_AUTOID_USE) /* ToDo: "name" max check, tag check.*/
#define cdp_id_is_auto(name)        (((name) & ~CDP_NAMING_MASK) == CDP_AUTOID_USE)


// Initial text name IDs:
enum _cdpInitialNameID {
    CDP_NAME_VOID,
    CDP_NAME_ROOT,

    CDP_NAME_ID_INITIAL_COUNT
};


typedef struct _cdpRecord   cdpRecord;

typedef union {
    void*       pointer;
    size_t      offset;
    uint8_t     byte[8];
    uint64_t    uint64[1];
    uint32_t    uint32[2];
    uint16_t    uint16[4];
    int64_t     int64[1];
    int32_t     int32[2];
    int16_t     int16[4];
    float       float32[2];
    double      float64[1];
} cdpValue;

typedef struct {
    size_t          size;       // Data size in bytes.
    size_t          capacity;   // Buffer capacity in bytes.
    union {
        struct {
           void*    _far;       // Points to container of data value.
           cdpDel   destructor; // Data container destructor function.
        };
        cdpValue   _data[2];    // Data value may start from here.
    };
} cdpData;

typedef struct {
    unsigned        count;      // Number of record pointers.
    unsigned        max;
    cdpRecord*      record[];   // Dynamic array of records shadowing this one.
} cdpShadow;

struct _cdpRecord {
    cdpMetarecord   metarecord; // Meta about this record entry (including id, etc).

    cdpMetadata     metadata;   // Metadata about what is contained in 'data'.
    union {
        cdpData*    data;       // Address of data buffer.
        cdpValue    _near;      // Data value if it fits in here.
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
    //cdpRecord*      self;       // Next instance of self (circular list ending in this very record).
};

typedef struct {
    cdpID           autoid;     // Auto-increment ID for naming contained records.
    cdpRecord*      owner;      // Parent record owning this child storage.
    union {
        cdpRecord*  linked;     // A linked shadow record (when children, see in cdpRecord otherwise).
        cdpShadow*  shadow;     // Shadow structure (if record has children).
    };
    size_t          chdCount;   // Number of child records.
} cdpChdStore;

typedef struct {
    cdpRecord*      record;
    cdpRecord*      next;
    cdpRecord*      prev;
    cdpRecord*      parent;
    size_t          position;
    unsigned        depth;
} cdpBookEntry;

typedef int  (*cdpCompare)  (const cdpRecord* restrict, const cdpRecord* restrict, void*);
typedef bool (*cdpTraverse) (cdpBookEntry*, void*);
typedef bool (*cdpAgent)    (cdpRecord* task);

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


// Root dictionary.
static inline cdpRecord* cdp_root(void)  {extern cdpRecord CDP_ROOT; assert(CDP_ROOT.children);  return &CDP_ROOT;}


bool cdp_record_initialize( cdpRecord* record, cdpID name,
                            bool dictionary, unsigned storage, size_t basez,
                            cdpID metadata, size_t capacity, size_t size,
                            cdpValue data, cdpDel destructor  );
void cdp_record_initialize_clone(cdpRecord* newClone, cdpID nameID, cdpRecord* record);
void cdp_record_finalize(cdpRecord* record);

#define cdp_record_initialize_value(r, name, metadata, capacity, size, data, destructor)  cdp_record_initialize(r, name, true, CDP_STORAGE_RED_BLACK_T, 0, metadata, capacity, size, data, destructor)
#define cdp_record_initialize_branch(r, name, dictionary, storage, basez)                 cdp_record_initialize(r, name, dictionary, storage, basez, 0, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_initialize_dictionary(r, name, storage, basez)                         cdp_record_initialize(r, name, true, storage, basez, 0, 0, 0, (cdpValue){0}, NULL)

static inline cdpTag cdp_record_domain(const cdpRecord* record) {assert(record);  return record->metadata.domain;}
static inline cdpTag cdp_record_role(const cdpRecord* record)   {assert(record);  return record->metadata.role;}
static inline cdpTag cdp_record_tag(const cdpRecord* record)    {assert(record);  return record->metadata.tag;}

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

#define cdp_record_is_void(r)       (!cdp_record_get_name(r))
#define cdp_record_has_data(r)      ((r)->metarecord.recdata != CDP_RECDATA_NONE)
#define cdp_record_is_dictionary(r) ((r)->metarecord.dictionary)
#define cdp_record_is_insertable(r) ((r)->metarecord.storage != CDP_STORAGE_RED_BLACK_T)

#define cdp_record_is_private(r)    ((r)->metarecord.priv)
#define cdp_record_is_factual(r)    ((r)->metarecord.factual)
#define cdp_record_is_shadowed(r)   ((r)->metarecord.shadowing)
#define cdp_record_is_baby(r)       ((r)->metarecord.baby)
#define cdp_record_is_connected(r)  ((r)->metarecord.connected)

#define cdp_record_id_is_pending(r) cdp_id_is_auto((r)->metarecord.name)
static inline void  cdp_record_set_autoid(const cdpRecord* record, cdpID id)  {assert(cdp_record_with_store(record));  cdpChdStore* store = CDP_CHD_STORE(record->children); assert(store->autoid < id  &&  id <= CDP_AUTOID_MAX); store->autoid = id;}
static inline cdpID cdp_record_get_autoid(const cdpRecord* record)            {assert(cdp_record_with_store(record));  return CDP_CHD_STORE(record->children)->autoid;}

void cdp_record_relink_storage(cdpRecord* record);

static inline void cdp_record_transfer(cdpRecord* src, cdpRecord* dst) {
    assert(!cdp_record_is_void(src) && dst);

    dst = src;

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

#define cdp_record_prepend_record(parent, name, dictionary, storage, basez, metadata, capacity, size, data, destructor, ...)\
    ({cdpRecord r={0}; cdp_record_initialize(&r, name, dictionary, storage, basez, metadata, capacity, size, data, destructor)? cdp_record_add(parent, &r, true ##__VA_ARGS__): NULL;})

#define cdp_record_prepend_value(parent, name, metadata, capacity, size, data, destructor)  cdp_record_prepend_record(parent, name, true, CDP_STORAGE_RED_BLACK_T, 0, metadata, capacity, size, data, destructor)
#define cdp_record_prepend_branch(parent, name, dictionary, storage, basez)                 cdp_record_prepend_record(parent, name, dictionary, storage, basez, 0, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_prepend_dictionary(parent, name, storage, basez)                         cdp_record_prepend_record(parent, name, true, storage, basez, 0, 0, 0, (cdpValue){0}, NULL)

#define cdp_record_add_record(p, n, t, s, b, m, c, z, d, f)                                 cdp_record_prepend_record(p, n, t, s, b, m, c, z, d, f, &&false)

#define cdp_record_add_value(parent, name, metadata, capacity, size, data, destructor)      cdp_record_add_record(parent, name, true, CDP_STORAGE_RED_BLACK_T, 0, metadata, capacity, size, data, destructor)
#define cdp_record_add_branch(parent, name, dictionary, storage, basez)                     cdp_record_add_record(parent, name, dictionary, storage, basez, 0, 0, 0, (cdpValue){0}, NULL)
#define cdp_record_add_dictionary(parent, name, storage, basez)                             cdp_record_add_record(parent, name, true, storage, basez, 0, 0, 0, (cdpValue){0}, NULL)


// Accessing data
cdpValue cdp_record_read(const cdpRecord* record, size_t* capacity, size_t* size, void* data);
cdpValue cdp_record_read_value(const cdpRecord* record);
void* cdp_record_update(cdpRecord* record, size_t capacity, size_t size, cdpValue data, bool swap);
#define cdp_record_update_value(r, v)   cdp_record_update(r, sizeof(cdpValue), sizeof(cdpValue), v, false)

void cdp_record_data_delete(cdpRecord* record);
void cdp_record_data_reset(cdpRecord* record);
void cdp_record_branch_reset(cdpRecord* record);
#define cdp_record_reset(r)    do{ cdp_record_branch_reset(r); cdp_record_data_reset(r);} while(0)
#define cdp_record_delete(r)    do{ cdp_record_branch_reset(r); cdp_record_data_delete(r);} while(0)


// Constructs the full path (sequence of ids) for a given record, returning the depth.
bool cdp_record_path(const cdpRecord* record, cdpPath** path);


// Accessing branched records
cdpRecord* cdp_record_first(const cdpRecord* record);
cdpRecord* cdp_record_last (const cdpRecord* record);

cdpRecord* cdp_record_find_by_name(const cdpRecord* record, cdpID name);
cdpRecord* cdp_record_find_by_key(const cdpRecord* record, cdpRecord* key, cdpCompare compare, void* context);
cdpRecord* cdp_record_find_by_position(const cdpRecord* record, size_t pos);
cdpRecord* cdp_record_find_by_path(const cdpRecord* start, const cdpPath* path);

cdpRecord* cdp_record_prev(const cdpRecord* parent, cdpRecord* record);
cdpRecord* cdp_record_next(const cdpRecord* parent, cdpRecord* record);

cdpRecord* cdp_book_next_by_name(const cdpRecord* record, cdpID id, uintptr_t* prev);
cdpRecord* cdp_book_next_by_path(const cdpRecord* start, cdpPath* path, uintptr_t* prev);

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
