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

    * Record: The fundamental unit within the system, capable of storing data
    and having children records at the same time.

    * Link: A record that points to another record.

    * Metadata and Flags: Each record contains metadata, including
    flags that specify the record's characteristics, and identifier
    indicating the record's role or ID within its parent.

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

    * cdpVariantBook: Serves as a versatile container within the
    system, holding child records through diverse storage strategies.
    Each cdpVariantBook can adopt one of several child storage
    mechanisms, determined by the structure indicator in its metadata.
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
    all IOs and keeping a list of currently locked record paths and a
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


/* ### **cdpMetadata: A Comprehensive 64-Bit Encoding System for
Cross-Domain Knowledge Representation**

The **cdpMetadata** system is a powerful and scalable **64-bit encoding
framework** designed to universally describe entities, actions, data
structures, and concepts across diverse domains of human knowledge. By
providing a flexible structure that divides its 64 bits into defined
segments, the system enables concise yet richly descriptive metadata
that can efficiently encode everything from natural language elements
to complex graphical user interfaces, programming constructs, and
scientific data.

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
      cdpAttribute  _name;                          // The header attributes (tag, domain, etc) as a single value.
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
    CDP_DOMAIN_RECORD,          // A symbolic domain used to indicate a pure parent record without actual data.
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
 * Record Structures
 */

typedef uint64_t  cdpID;

#defome CDP_ID_BITS             cdp_bitsof(cdpID)
#define CDP_RECD_FLAG_BITS      12
#define CDP_NAME_BITS           (CDP_ID_BITS - CDP_RECD_FLAG_BITS)
#define CDP_NAME_MAX            (~(((cdpID)(-1)) << CDP_NAME_BITS))
#define CDP_NAMECONV_BITS       2
#define CDP_AUTO_ID_BITS        (CDP_NAME_BITS - CDP_NAMECONV_BITS)
#define CDP_AUTO_ID_MAXVAL      (~(((cdpID)(-1)) << CDP_AUTO_ID_BITS))
#define CDP_AUTO_ID_MAX         (CDP_AUTO_ID_MAXVAL - 1)

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
              withstore:  1,    // Record has child storage (but not necessarely children).
              shadow:     2,    // If record has shadowing records (links pointing to it).

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
}

enum _cdpRecordShadowing {
    CDP_SHADOW_NONE,            // No shadow records.
    CDP_SHADOW_SINGLE,          // Single shadow record.
    CDP_SHADOW_MULTIPLE,        // Multiple shadows.
};


enum _cdpRecordNaming {
    CDP_NAMING_TEXT,            // Cross-domain indexed text id.
    CDP_NAMING_TAG,             // Per-domain indexed text id.
    CDP_NAMING_LOCAL,           // Per-record numerical id.
    CDP_NAMING_GLOBAL,          // Global numerical id (it may be used with a system global autoID).

    CDP_NAMING_COUNT
}

#define cdp_id_from_naming(naming)  (((cdpID)((naming) & 3)) << CDP_AUTO_ID_BITS)
#define cdp_id_to_naming(id)        (((id) >> CDP_AUTO_ID_BITS) & 3)
#define cdp_id_to_text(textid)      ((textid) & cdp_id_from_naming(CDP_NAMING_TEXT))
#define cdp_id_to_tag(tagid)        ((tagid) & cdp_id_from_naming(CDP_NAMING_TAG))
#define cdp_id(nameid)              ((nameid) & CDP_AUTO_ID_MAXVAL)

#define cdp_id_local(id)            ((id) & cdp_id_from_naming(CDP_NAMING_LOCAL))
#define cdp_id_global(id)           ((id) & cdp_id_from_naming(CDP_NAMING_GLOBAL))

#define CDP_AUTO_ID_USE             CDP_AUTO_ID_MAXVAL
#define CDP_AUTO_ID_LOCAL           cdp_id_local(CDP_AUTO_ID_USE)
#define CDP_AUTO_ID_GLOBAL          cdp_id_global(CDP_AUTO_ID_USE)

#define cdp_id_name_is_local(name)  ((cdp_id_from_naming(3) & (name)) == cdp_id_from_naming(CDP_NAMING_LOCAL))
#define cdp_id_name_is_global(name) ((cdp_id_from_naming(3) & (name)) == cdp_id_from_naming(CDP_NAMING_GLOBAL))
#define cdp_id_valid(nameid)        (cdp_id(nameid) < CDP_AUTO_ID_MAX) /* ToDo: "name" max check, tag check.*/
#define cdp_id_is_auto(nameid)      ((nameid) & CDP_AUTO_ID_MAXVAL)


// Initial text name IDs:
enum _cdpInitialNameID {
    CDP_NAME_VOID,
    CDP_NAME_ROOT,

    CDP_NAME_ID_INITIAL_COUNT
};


typedef struct _cdpRecord   cdpRecord;

static_assert(sizeof(void*) == sizeof(uint64_t), "32 bit is unssoported yet!");

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
    cdpRecord*      self;       // Next instance of self (circular list ending in this very record).
};

typedef struct {
    cdpID           autoID;     // Auto-increment ID for naming contained records.
    cdpRecord*      owner;      // Parent record owning this child storage.
    union {
        cdpRecord*  linked;     // A linked shadow record (when children, see in cdpRecord otherwise).
        cdpShadow*  shadow;     // Shadow structure (if record has children).
    };
    size_t          chdCount;   // Number of child records.
} cdpChdStore;

typedef struct {
    cdpRecord*      record;
    cdpRecord*      parent;
    cdpRecord*      next;
    cdpRecord*      prev;
    size_t          position;
    unsigned        depth;
} cdpBookEntry;

typedef int (*cdpCompare)(const cdpRecord* restrict, const cdpRecord* restrict, void*);

typedef bool (*cdpTraverse)(cdpBookEntry*, void*);

typedef bool (*cdpAgent)(cdpRecord* task);

typedef struct {
    unsigned        count;
    unsigned        max;
    cdpID           id[];
} cdpPath;


/*
 * Record Operations
 */

bool cdp_record_initialize( cdpRecord* record, cdpID name,
                            bool dictionary, unsigned storage, size_t basez,
                            cdpID metadata, size_t size, size_t capacity,
                            cdpValue data, cdpDel destructor);
void cdp_record_finalize(cdpRecord* record);

#define cdp_record_initialize_register(r, id, tag, borrow, data, size)  cdp_record_initialize(r, CDP_ROLE_REGISTER, 0, id, tag, borrow, data, size)

#define cdp_record_initialize_list(r, id, chdStorage, ...)        cdp_record_initialize(r, CDP_ROLE_BOOK, 0, id, CDP_TAG_LIST,  ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_initialize_queue(r, id, chdStorage, ...)       cdp_record_initialize(r, CDP_ROLE_BOOK, 0, id, CDP_TAG_QUEUE, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_record_initialize_stack(r, id, chdStorage, ...)       cdp_record_initialize(r, CDP_ROLE_BOOK, 0, id, CDP_TAG_STACK, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_record_initialize_dictionary(r, id, tag, chdStorage, ...)  cdp_record_initialize(r, CDP_ROLE_BOOK, CDP_ATTRIB_DICTIONARY, id, tag? tag: CDP_TAG_DICTIONARY, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define CDP_RECORD_SET_ATTRIB(r, a)   ((r)->metadata.attribute |= (a))

// General property check
static inline cdpID cdp_record_attributes(const cdpRecord* record)  {assert(record);  return record->metadata.attribute;}
static inline cdpID cdp_record_type      (const cdpRecord* record)  {assert(record);  return record->metadata.type;}
static inline cdpID cdp_record_tag       (const cdpRecord* record)  {assert(record);  return record->metadata.tag;}

static inline cdpID cdp_record_get_id(const cdpRecord* record)      {assert(record);  return record->metarecord.name;}
static inline void  cdp_record_set_id(cdpRecord* record, cdpID name)  {assert(record && name < CDP_NAME_MAX);  record->metarecord.name = name;}

#define CDP_CHD_STORE(children)         ({assert(children);  (cdpChdStore*)(children);})
#define cdp_record_par_store(record)    CDP_CHD_STORE((record)->store)
static inline cdpRecord* cdp_record_parent  (const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->book: NULL;}
static inline size_t     cdp_record_siblings(const cdpRecord* record)   {assert(record);  return CDP_EXPECT_PTR(record->store)? cdp_record_par_store(record)->chdCount: 0;}

static inline size_t cdp_record_children(const cdpRecord* record)       {assert(record);  if (record->metarecord.withstore) return ((cdpChdStore*)record->store)->chdCount; else return 0;}

#define cdp_record_is_void(r)       (cdp_record_type(r) == CDP_ROLE_VOID)
#define cdp_record_has_data(r)      ((r)->metarecord.recdata != CDP_RECDATA_NONE)
#define cdp_record_is_link(r)       (cdp_record_type(r) == CDP_ROLE_LINK)

#define cdp_record_is_private(r)    cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_PRIVATE)
#define cdp_record_is_factual(r)    cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_FACTUAL)
#define cdp_record_is_shadowed(r)   cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_SHADOWED)
#define cdp_record_is_baby(r)       cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_BABY)
#define cdp_record_is_connected(r)  cdp_is_set(cdp_record_attributes(r), CDP_ATTRIB_CONNECTED)
#define cdp_record_is_dictionary(r) ((r)->metarecord.dictionary)
#define cdp_record_is_insertable(r) ((r)->metarecord.storage != CDP_STORAGE_RED_BLACK_T)

static inline bool cdp_record_is_named(const cdpRecord* record)  {assert(record);  if (cdp_id_is_named(record->metadata.id)) {assert(record->metadata.id <= CDP_NAME_MAXVAL); return true;} return false;}

#define cdp_record_id_is_auto(r)    (cdp_record_get_id(r) < CDP_NAMEID_FLAG)
#define cdp_record_id_is_pending(r) (cdp_record_get_id(r) == CDP_AUTO_ID)


// Link properties
static inline void* cdp_link_data(const cdpRecord* link)   {assert(cdp_record_is_link(link));  return link->recData.link.target.address;}

#define CDP_LINK_RESOLVE(r)  do{ if (cdp_record_is_link(r)) (r) = cdp_link_data(r); }while(0)

static inline void cdp_record_initialize_link(cdpRecord* newLink, cdpID nameID, cdpRecord* record) {
    assert(newLink && !cdp_record_is_void(record));
    CDP_LINK_RESOLVE(record);
    cdp_record_initialize(newLink, CDP_ROLE_LINK, 0, nameID, cdp_record_tag(record), record);
    newLink->metarecord.storage = CDP_STO_LNK_POINTER;
}

static inline void cdp_record_initialize_shadow(cdpRecord* newShadow, cdpID nameID, cdpRecord* record) {
    assert(newShadow && !cdp_record_is_void(record));
    CDP_LINK_RESOLVE(record);
    cdp_record_initialize(newShadow, CDP_ROLE_LINK, 0, nameID, CDP_TAG_LINK, record);
    newShadow->metarecord.storage = CDP_STO_LNK_POINTER;
    CDP_RECORD_SET_ATTRIB(record, CDP_ATTRIB_SHADOWED);
    // ToDo: actually shadow the record.
}

void cdp_record_initialize_clone(cdpRecord* newClone, cdpID nameID, cdpRecord* record);

static inline void cdp_record_initialize_agent(cdpRecord* newAgent, cdpTag tag, cdpAgent agent) {
    assert(newAgent && agent);
    cdp_record_initialize(newAgent, CDP_ROLE_LINK, 0, tag, CDP_TAG_LINK, agent);
    newAgent->metarecord.storage = CDP_STO_LNK_AGENT;
}


void cdp_record_relink_storage(cdpRecord* book);

static inline void cdp_record_transfer(cdpRecord* src, cdpRecord* dst) {
    assert(!cdp_record_is_void(src) && dst);
    dst->metadata = src->metadata;
    dst->recData  = src->recData;
    dst->store    = NULL;
    if (cdp_record_children(dst))
        cdp_record_relink_storage(dst);
}

static inline void cdp_record_replace(cdpRecord* original, cdpRecord* newRecord) {
    assert(original && !cdp_record_is_void(newRecord));
    cdp_record_finalize(original);
    original->metadata = newRecord->metadata;
    original->recData  = newRecord->recData;
    if (cdp_record_children(original))
        cdp_record_relink_storage(original);
}


// Register properties
static inline bool   cdp_register_is_borrowed(const cdpRecord* reg) {assert(cdp_record_has_data(reg));  return (reg->metarecord.storage == CDP_STO_REG_BORROWED);}
static inline void*  cdp_register_data(const cdpRecord* reg)        {assert(cdp_record_has_data(reg));  return reg->recData.reg.data.ptr;}
static inline size_t cdp_register_size(const cdpRecord* reg)        {assert(cdp_record_has_data(reg));  return reg->recData.reg.size;}


static inline cdpID cdp_book_get_auto_id(const cdpRecord* book)           {assert(cdp_record_children(book));  return CDP_CHD_STORE(book->children)->autoID;}
static inline void  cdp_book_set_auto_id(const cdpRecord* book, cdpID id) {assert(cdp_record_children(book));  cdpChdStore* store = CDP_CHD_STORE(book->children); assert(store->autoID < id  &&  id <= CDP_AUTO_ID_MAXVAL); store->autoID = id;}

cdpRecord* cdp_book_add_property(cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_book_get_property(const cdpRecord* book, cdpID id);


// Appends, inserts or prepends a copy of record into another record.
cdpRecord* cdp_record_add(cdpRecord* parent, cdpRecord* record, bool prepend);
cdpRecord* cdp_record_sorted_insert(cdpRecord* parent, cdpRecord* record, cdpCompare compare, void* context);

#define cdp_book_add(b, type, attribute, id, tag, prepend, ...)  ({cdpRecord r={0}; cdp_record_initialize(&r, type, attribute, id, tag, ##__VA_ARGS__)? cdp_record_add(b, &r, prepend): NULL;})
#define cdp_book_add_register(b, attrib, id, tag, borrow, data, size)          cdp_book_add(b, CDP_ROLE_REGISTER, attrib, id, tag, false, ((unsigned)(borrow)), data, ((size_t)(size)))
#define cdp_book_prepend_register(b, attrib, id, tag, borrow, data, size)      cdp_book_add(b, CDP_ROLE_REGISTER, attrib, id, tag,  true, ((unsigned)(borrow)), data, ((size_t)(size)))

static inline cdpRecord* cdp_book_add_text(cdpRecord* book, unsigned attrib, cdpID id, bool borrow, const char* text)    {assert(cdp_record_children(book) && text && *text);  cdpRecord* reg = cdp_book_add_register(book, attrib, id, CDP_TAG_UTF8, borrow, text, strlen(text) + 1); reg->recData.reg.size--; return reg;}
#define cdp_book_add_static_text(b, id, text)   cdp_book_add_text(b, CDP_ATTRIB_FACTUAL, id, true, text)

#define CDP_FUNC_ADD_VAL_(func, ctype, tag)                                    \
    static inline cdpRecord* cdp_book_add_##func(cdpRecord* book, cdpID id, ctype value)    {assert(cdp_record_children(book));  return cdp_book_add_register(book, 0, id, tag, false, &value, sizeof(value));}\
    static inline cdpRecord* cdp_book_prepend_##func(cdpRecord* book, cdpID id, ctype value){assert(cdp_record_is_insertable(book));  return cdp_book_prepend_register(book, 0, id, tag, false, &value, sizeof(value));}

    CDP_FUNC_ADD_VAL_(byte,    uint8_t,  CDP_TAG_BYTE)
    CDP_FUNC_ADD_VAL_(uint16,  uint16_t, CDP_TAG_UINT16)
    CDP_FUNC_ADD_VAL_(uint32,  uint32_t, CDP_TAG_UINT32)
    CDP_FUNC_ADD_VAL_(uint64,  uint64_t, CDP_TAG_UINT64)
    CDP_FUNC_ADD_VAL_(int16,   int16_t,  CDP_TAG_INT16)
    CDP_FUNC_ADD_VAL_(int32,   int32_t,  CDP_TAG_INT32)
    CDP_FUNC_ADD_VAL_(int64,   int64_t,  CDP_TAG_INT64)
    CDP_FUNC_ADD_VAL_(float32, float,    CDP_TAG_FLOAT32)
    CDP_FUNC_ADD_VAL_(float64, double,   CDP_TAG_FLOAT64)

    CDP_FUNC_ADD_VAL_(id, cdpID, CDP_TAG_ID)
    CDP_FUNC_ADD_VAL_(bool, uint8_t,  CDP_TAG_BOOLEAN)


#define cdp_book_add_book(b, id, tag, chdStorage, ...)            cdp_book_add(b, CDP_ROLE_BOOK, 0, id, tag, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_book(b, id, tag, chdStorage, ...)        cdp_book_add(b, CDP_ROLE_BOOK, 0, id, tag,  true, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_book_add_list(b, id, chdStorage, ...)                 cdp_book_add_book(b, id, CDP_TAG_LIST,  ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_queue(b, id, chdStorage, ...)                cdp_book_add_book(b, id, CDP_TAG_QUEUE, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_add_stack(b, id, chdStorage, ...)                cdp_book_add_book(b, id, CDP_TAG_STACK, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_book_add_dictionary(b, id, tag, chdStorage, ...)      cdp_book_add(b, CDP_ROLE_BOOK, CDP_ATTRIB_DICTIONARY, id, tag? tag: CDP_TAG_DICTIONARY, false, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_book_prepend_list(b, id, chdStorage, ...)             cdp_book_prepend_book(b, id, CDP_TAG_LIST,  ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_queue(b, id, chdStorage, ...)            cdp_book_prepend_book(b, id, CDP_TAG_QUEUE, ((unsigned)(chdStorage)), ##__VA_ARGS__)
#define cdp_book_prepend_stack(b, id, chdStorage, ...)            cdp_book_prepend_book(b, id, CDP_TAG_STACK, ((unsigned)(chdStorage)), ##__VA_ARGS__)

#define cdp_book_prepend_dictionary(b, id, tag, chdStorage, ...)  cdp_book_add(b, CDP_ROLE_BOOK, CDP_ATTRIB_DICTIONARY, id, tag? tag: CDP_TAG_DICTIONARY, true, ((unsigned)(chdStorage)), ##__VA_ARGS__)


// Root dictionary.
static inline cdpRecord* cdp_root(void)  {extern cdpRecord CDP_ROOT; assert(CDP_ROOT.children);  return &CDP_ROOT;}


// Constructs the full path (sequence of ids) for a given record, returning the depth.
bool cdp_record_path(const cdpRecord* record, cdpPath** path);


// Accessing registers
void* cdp_record_read(const cdpRecord* reg, size_t position, void* data, size_t* size);   // Reads register data from position and puts it on data buffer (atomically).
void* cdp_register_write(cdpRecord* reg, size_t position, const void* data, size_t size);   // Writes the data of a register record at position (atomically and it may reallocate memory).
#define cdp_register_update(reg, data, size)   cdp_register_write(reg, 0, data, size)

#define cdp_record_read_byte(reg)     (*(uint8_t*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_uint16(reg)   (*(uint16_t*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_uint32(reg)   (*(uint32_t*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_uint64(reg)   (*(uint64_t*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_int16(reg)    (*(int16_t*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_int32(reg)    (*(int32_t*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_int64(reg)    (*(int64_t*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_float32(reg)  (*(float*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_float64(reg)  (*(double*)cdp_record_read(reg, 0, NULL, NULL))

#define cdp_record_read_id(reg)       (*(cdpID*)cdp_record_read(reg, 0, NULL, NULL))
#define cdp_record_read_bool(reg)     (*(uint8_t*)cdp_record_read(reg, 0, NULL, NULL))  // ToDo: use recData.reg.data.direct.
#define cdp_record_read_utf8(reg)     ((const char*)cdp_record_read(reg, 0, NULL, NULL))

#define cdp_register_update_byte(reg, v)    cdp_register_update(reg, &(v), sizeof(uint8_t))
#define cdp_register_update_uint16(reg, v)  cdp_register_update(reg, &(v), sizeof(uint16_t))
#define cdp_register_update_uint32(reg, v)  cdp_register_update(reg, &(v), sizeof(uint32_t))
#define cdp_register_update_uint64(reg, v)  cdp_register_update(reg, &(v), sizeof(uint64_t))
#define cdp_register_update_int16(reg, v)   cdp_register_update(reg, &(v), sizeof(int16_t))
#define cdp_register_update_int32(reg, v)   cdp_register_update(reg, &(v), sizeof(int32_t))
#define cdp_register_update_int64(reg, v)   cdp_register_update(reg, &(v), sizeof(int64_t))
#define cdp_register_update_float32(reg, v) cdp_register_update(reg, &(v), sizeof(float))
#define cdp_register_update_float64(reg, v) cdp_register_update(reg, &(v), sizeof(double))

#define cdp_register_update_bool(reg, v)    cdp_register_update(reg, &(v), sizeof(uint8_t))

static inline void cdp_register_reset(cdpRecord* reg)   {memset(cdp_register_data(reg), 0, cdp_register_size(reg));}


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
bool cdp_book_deep_traverse(cdpRecord* book, cdpTraverse func, cdpTraverse listEnd, void* context, cdpBookEntry* entry);  // Traverses each sub-branch of a book record.


// Removing records
bool cdp_book_take(cdpRecord* book, cdpRecord* target);     // Removes last record.
bool cdp_book_pop(cdpRecord* book, cdpRecord* target);      // Removes first record.

void cdp_record_remove(cdpRecord* record, cdpRecord* target);   // Deletes a record and all its children re-organizing sibling storage.
#define cdp_register_remove(reg, tgt)   cdp_record_remove(reg, tgt)
#define cdp_register_delete(reg)        cdp_register_remove(reg, NULL)
#define cdp_book_remove(book, tgt)      cdp_record_remove(book, tgt)
#define cdp_book_delete(book)           cdp_book_remove(book, NULL)
void cdp_book_reset(cdpRecord* book);     // Deletes all children of a book or dictionary.


static inline cdpRecord* cdp_book_add_link(cdpRecord* book, cdpID nameID, cdpRecord* record) {
    assert(cdp_record_children(book) && !cdp_record_is_void(record));
    CDP_LINK_RESOLVE(record);
    cdpRecord link = {0};
    cdp_record_initialize_link(&link, nameID, record);
    return cdp_record_add(book, &link, false);
}

static inline cdpRecord* cdp_book_add_shadow(cdpRecord* book, cdpID nameID, cdpRecord* record) {
    assert(cdp_record_children(book) && !cdp_record_is_void(record));
    CDP_LINK_RESOLVE(record);
    cdpRecord shadow = {0};
    cdp_record_initialize_shadow(&shadow, nameID, record);
    return cdp_record_add(book, &shadow, false);
}

static inline cdpRecord* cdp_book_add_clone(cdpRecord* book, cdpID nameID, cdpRecord* record) {
    assert(cdp_record_children(book) && !cdp_record_is_void(record));
    cdpRecord clone;
    cdp_record_initialize_clone(&clone, nameID, record);
    return cdp_record_add(book, &clone, false);
}

static inline cdpRecord* cdp_book_move_to(cdpRecord* book, cdpID nameID, cdpRecord* record) {
    assert(cdp_record_children(book) && !cdp_record_is_void(record));
    cdpRecord moved;
    cdp_record_remove(record, &moved);
    cdp_record_set_id(&moved, nameID);
    return cdp_record_add(book, &moved, false);
}

static inline cdpRecord* cdp_book_add_agent(cdpRecord* book, cdpTag tag, cdpAgent agent) {
    assert(cdp_record_children(book) && agent);
    cdpRecord newAgent = {0};
    cdp_record_initialize_agent(&newAction, tag, action);
    return cdp_record_add(book, &newAction, false);
}


// Accessing dictionary
static inline void* cdp_dict_get_data(cdpRecord* dict, cdpID nameID)    {assert(cdp_record_is_dictionary(dict));  cdpRecord* reg = cdp_book_find_by_name(dict, nameID);  return cdp_register_data(reg);}
#define cdp_dict_get_byte(d, id)        (*(uint8_t*)cdp_dict_get_data(d, id))
#define cdp_dict_get_uint16(d, id)      (*(uint16_t*)cdp_dict_get_data(d, id))
#define cdp_dict_get_uint32(d, id)      (*(uint32_t*)cdp_dict_get_data(d, id))
#define cdp_dict_get_uint64(d, id)      (*(uint64_t*)cdp_dict_get_data(d, id))
#define cdp_dict_get_int16(d, id)       (*(int16_t*)cdp_dict_get_data(d, id))
#define cdp_dict_get_int32(d, id)       (*(int32_t*)cdp_dict_get_data(d, id))
#define cdp_dict_get_int64(d, id)       (*(int64_t*)cdp_dict_get_data(d, id))
#define cdp_dict_get_float32(d, id)     (*(float*)cdp_dict_get_data(d, id))
#define cdp_dict_get_float64(d, id)     (*(double*)cdp_dict_get_data(d, id))

#define cdp_dict_get_id(d, id)      (*(cdpID*)cdp_dict_get_data(d, id))
#define cdp_dict_get_bool(d, id)    (*(bool*)cdp_dict_get_data(d, id))

static inline void* cdp_dict_get_link(cdpRecord* dict, cdpID nameID)    {assert(cdp_record_is_dictionary(dict));  cdpRecord* link = cdp_book_find_by_name(dict, nameID);  return link? cdp_link_data(link): NULL;}
#define cdp_dict_get_agent(dict, nameID)   ((cdpAgent)cdp_dict_get_link(dict, nameID))


// Converts an unsorted book into a sorted one.
void cdp_book_to_dictionary(cdpRecord* book);


// To manage concurrent access to records safely.
bool cdp_record_lock(cdpRecord* record);
bool cdp_record_unlock(cdpRecord* record);


// Initiate and shutdown record system.
void cdp_record_system_initiate(void);
void cdp_record_system_shutdown(void);


/*
    TODO:
    - Send simultaneous book tasks to nested agent books.
    - Fully define new roles of dictionaries and *_find_by_name() family functions.
    - Use "recData.reg.data.direct" in registers.
    - Imlement clone (deep copy) records.
    - Traverse book in internal (stoTech) order.
    - Add indexof for records.
    - Implement cdp_book_insert_at(position).
    - Update MAX_DEPTH based on path/traverse operations.
    - Add cdp_book_update_nested_links(old, new).
    - CDP_TAG_VOID should never be a valid name for records.
    - Any book may be a dictionary, but only if the name matches the insertion/deletion sequence.
    - If a record is added to a book with its name explicitly above "auto_id", then that must be updated.
*/


#endif
