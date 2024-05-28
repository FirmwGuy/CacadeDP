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

#ifndef CDP_OBJECT_H
#define CDP_OBJECT_H


/*
    Cascade Data Objecting System - Layer 2
    ------------------------------------------

    ### System Overview:
    CascadeDP Layer 1 implemented a record solution, intended to be used
    as the basis of a RAM file system (similar to Plan 9). This Layer 2
    is designed to handle a distributed system, that is, data sharing
    and service management across a distributed network of devices.

    ### Object:
    An object is a function acting as a event handler for a specific
    structured book. The function is run whenever such book is subject
    to an event. The object may propagate instances all across the
    network. In a way, objects are executable functions that "travel"
    along the data they are bound to.

    ### Types:
    The type system uses a factory pattern combined with dynamic type
    specification and validation. This approach enables structured
    creation of book types and instances, ensuring adherence to defined
    types while supporting graph-like relationships with multiple
    parents. The Factory centralizes creation logic, enforces
    structure, and supports dynamic configuration, while a validation
    API ensures consistency and correctness.
    - **Encapsulation**: Centralizes book creation logic.
    - **Consistency**: Enforces structure and metadata for each book type.
    - **Flexibility**: Facilitates adding new book types and dynamic creation.
    - **Separation of Concerns**: Decouples creation from business logic.
    - **Context-Aware Initialization**: Adapts structures based on
    parent relationships and specific requirements.

    ### Directory Structure:
    The book structure is explained using CascadeDP records, hence with each
    entry having an id (represented with text or number), a type (enclosed in
    parentheses), and/or register value (following the ":"). Links are
    represented by the arrow "->".

    #### 1. ** /type/ **
    The `/type/` book stores internal information needed by the
    distributed record system about types. This setup, however, is
    local to each node.
    - **Example Structure**:
      ```
      /type/ (dictionary)
          5/ (type)
              name:"catalog"
              parent/ (dictionary)
                  1 -> /type/3
              description: "Book with records ordered by some user-defined criteria."
          8/ (type)
              name:"log"
              parent/ (dictionary)
                  1 -> /type/7
              description: "Queue that never removes records."
          9/ (type)
              name:"boolean"
              description: "Boolean value."
              value/ (dictionary)
                  1:"false"
                  2:"true"
              size:1 (uint32)
      ```

    #### 2. ** /system/ **
    The `/system/` book is used for storing object connection
    link information. It also contains local object instance data,
    such as input queues and output links.
    - **Example Structure**:
      ```
      /system/ (dictionary)
          object001/ (dictionary)
              555/ (instance)
                  input/ (dictionary)
                      arg/ (queue)
                  output/ (dictionary)
                      result -> /instance/object002/557/input/arg
              556/ (instance)
                  input/ (dictionary)
                      arg/ (queue)
                  output/ (dictionary)
                      result -> /instance/object002/558/input/arg
      ```

    #### 3. ** /user/ **
    This book serves as the personal space for user-specific
    configurations and data. Each user or administrative entity
    interacting with the nodes might have a separate entry here.
    This book may be replicated to other nodes.
    - **Example Structure**:
      ```
      /user/ (dictionary)
          user1/ (dictionary)
          user2/ (dictionary)
      ```

    #### 4. ** ~/private/ **
    This book (inside a user's book) stores persistent records generated
    by objectes and meant to be accessed only by the (owner) network
    user. This book is never replicated.
    - **Example Structure**:
      ```
      /user/ (dictionary)
          user1/ (dictionary)
              private/ (dictionary)
                  system/ (dictionary)
                      object01/ (dictionary)
                          555/ (private_instance)
                             states/ (book)
                          saved-data/ (book)
      ```

    #### 5. ** /public/ **
    The `/public/` book is used for storing public records generated by
    the objectes in the local node. These records are advertised along
    this node when it connects to the network and may be accessed
    (and/or cached/replicated) by other nodes.
    - **Example Structure**:
      ```
      /public/ (dictionary)
          object001/ (dictionary)
              measurements/ (dictionary)
                  car01/ (dictionary)
              shared/ (dictionary)
                  count:123 (uint32)
                  events/ (queue)
      ```

    #### 6. ** /data/ **
    The `/data/` book is a virtual space used for mapping distributed
    public records into a communal coherent structure. This includes
    registers and links as shared resources that might be accessed
    within the network. This is replicated as needed.
    - **Example Structure**:
      ```
      /data/ (dictionary)
          apps/
              object001/ (dictionary)
                  measurements/ (dictionary)
                      car01 -> /network/node001/public/object001/measurements/car01
                      car02 -> /network/node002/public/object001/measurements/car02
                  shared/   -> /network/node001/public/object001/shared/
      ```

    #### 7. ** /data/service/ **
    The `service/` book inside `/data/` contains the object instance
    creation service (OICS) locations for available objects.
    - **Example Structure**:
      ```
      /data/
          service/ (dictionary)
              object001/ (list)
                  node -> /network/node001/system/object001
                  node -> /network/node002/system/object001

      ```

    #### 8. ** /data/type/ **
    The `/type/` book inside `/data/` is a repository that contains
    type/object specific resources (such as executables) necessary for
    the propagation and loading of various objectes across the nodes.
    - **Example Structure**:
      ```
      /data/
          type/ (dictionary)
              object001/ (dictionary)
                  description
                  executable -> /network/node002/public/object001/executable
      ```

    #### 9. ** /network/ **
    This book is central to managing network-specific configurations
    and information about the reachability of each (foreign) connected
    node with respect to the local node.
    - **Example Structure**:
      ```
      /network/ (dictionary)
          node001/ (dictionary)
              protocol (dictionary)
                  address
                  config/
                  status
      ```

    ### Additional Considerations:
    - ** /data/config/ **: Maintains system-wide configuration files that affect
    all nodes and objectes.
    - ** /log/ and /data/log/ **: For comprehensive logging across the system,
    which could include logs from each node, object, and application.
    - ** /temp/ **: Temporary private records needed during execution,
    ensuring that transient data does not consume permanent storage space.

*/


#include "cdp_record.h"


static inline cdpRecord* cdp_record_void(void)  {extern cdpRecord* CDP_VOID; assert(CDP_VOID);  return CDP_VOID;}

enum {
    CDP_BOOLEAN_FALSE,
    CDP_BOOLEAN_TRUE,

    CDP_BOOLEAN_COUNT
};

enum {
    CDP_SIGNAL_STARTUP,
    CDP_SIGNAL_SHUTDOWN,

    CDP_SIGNAL_CONSTRUCT,
    CDP_SIGNAL_DESTRUCT,
    CDP_SIGNAL_REFERENCE,
    CDP_SIGNAL_FREE,

    CDP_SIGNAL_APPEND,
    CDP_SIGNAL_PREPEND,
    CDP_SIGNAL_INSERT,
    CDP_SIGNAL_UPDATE,
    CDP_SIGNAL_REMOVE,

    //CDP_SIGNAL_SORT,
    //CDP_SIGNAL_COPY,
    //CDP_SIGNAL_MOVE,
    //CDP_SIGNAL_PATCH,
    //CDP_SIGNAL_LINK,

    CDP_SIGNAL_COUNT
};


cdpID      cdp_name_id_add(const char* name, bool borrow);
#define    cdp_name_id_add_static(name)   cdp_name_id_add(name, true);
cdpRecord* cdp_name_id_text(cdpID id);

cdpID      cdp_type_add(const char* name, const char* description, size_t baseSize);
cdpRecord* cdp_type(cdpID id);

cdpID      cdp_object_type_add(cdpID nameID, cdpID objectID, char* description, size_t baseSize);
#define    cdp_object_type    cdp_type

void       cdp_object_construct(cdpRecord* object, cdpID nameID, cdpID objectTypeID, cdpID storage, uint32_t base);
void       cdp_object_destruct (cdpRecord* object);
void       cdp_object_reference(cdpRecord* object);
void       cdp_object_free     (cdpRecord* object);

cdpRecord* cdp_object_append   (cdpRecord* object, cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_object_prepend  (cdpRecord* object, cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_object_insert   (cdpRecord* object, cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_object_update   (cdpRecord* object, cdpRecord* book, cdpRecord* record, void* data, size_t size);
cdpRecord* cdp_object_remove   (cdpRecord* object, cdpRecord* book, cdpRecord* record);

//void       cdp_object_sort(cdpRecord* object);

bool       cdp_object_validate  (cdpRecord* object);


bool       cdp_system_step(void);
void       cdp_system_shutdown(void);


#endif
