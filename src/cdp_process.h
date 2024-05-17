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

#ifndef CDP_PROCESS_H
#define CDP_PROCESS_H


/*
    Cascade Data Processing System - Layer 2
    ------------------------------------------

    ### System Overview:
    CascadeDP Layer 1 implemented a record system, intended to be used
    as the basis of a RAM file system (similar to Plan 9). This Layer 2
    is designed to handle a distributed system, that is, data sharing
    and service management across a distributed network of processes.

    ### Process
    A process is a data processor unit, with inputs and outputs
    connected to other processes. The process is created on demand by
    the Process Instance Creation Service (or PICS), a unit which only
    task is to instatiate a local process by request. On the local node
    each process allowed to run is advertised (along its capabilities
    and data), allowing other processes to connect to them all across
    the network.

    ### Directory Structure:
    The book structure is explained using CascadeDP records, hece
    with each entry having a nameID (represented with text), a type
    (enclosed in parentheses), and an index (represented here inside
    brackets). Registers may have values (following the ":"), while links
    are represented by "->". UTF8 text content is represented by quoted
    text (its type is implied). The root of the tree is the dictionary "/".

    #### 1. **/type/**
    The `/type/` book stores internal information needed by the
    (distributed) record system about types. This setup, however, is
    local to each node.
    - **Example Structure**:
      ```
      /type/ (log)
          [9] type/ (type)
              name:"catalog"
              description: "Book with records ordered by some user-defined criteria."
          [12] type/ (type)
              name:"log"
              description: "Queue that never removes records."
          [17] boolean (type)
              name:"boolean"
              description: "Boolean value."
              value/ (set)
                  [0] value:"false"
                  [1] value:"true"
              size:1 (uint32)

      ```

    #### 2. **/system/**
    The `/system/` book is used for storing records related to each
    (connected) process instance, such as input queues and output
    linkage.
    - **Example Structure**:
      ```
      /system/ (dictionary)
          process001/ (catalog)
              instance/ (dictionary)
                  id:555 (uint32)
                  input/ (dictionary)
                      arg/ (queue)
                  output/ (dictionary)
                      result -> /instance/process002/instance(id=557)/input/arg
              instance/ (dictionary)
                  id:556 (uint32)
                  input/ (dictionary)
                      arg/ (queue)
                  output/ (dictionary)
                      result -> /instance/process002/instance(id=558)/input/arg
      ```

    #### 3. **/user/**
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

    #### 4. **~/private/**
    This book (inside a user's book) stores peristent records generated
    by processes and meant to be accessed only by the (owner) network
    user. This book is never replicated.
    - **Example Structure**:
      ```
      /user/ (dictionary)
          user1/ (dictionary)
              private/ (dictionary)
                  system/ (dictionary)
                      process01/ (catalog)
                          instance/ (dictionary)
                             id:555 (uint32)
                             states/ (book)
                          saved-data/ (book)
      ```

    #### 5. **/public/**
    The `/public/` book is used for storing public records generated by
    the processes in the local node. This records are advertised along
    this node when it connects to the network and may be accessed
    (and/or cached/replicated) by other nodes.
    - **Example Structure**:
      ```
      /public/ (dictionary)
          process001/ (dictionary)
              measurements/ (dictionary)
                  car01/ (dictionary)
              shared/ (dictionary)
                  count:123 (uint32)
                  events/ (queue)
      ```

    #### 6. **/data/**
    The `/data/` book is a virtual space used for mapping public
    records into a communal coherent structure. This includes registers
    and links as shared resources that might be accessed within the
    network.
    - **Example Structure**:
      ```
      /data/ (dictionary)
          process001/ (dictionary)
              measurements/ (dictionary)
                  car01 -> /network/node001/public/process001/measurements/car01
                  car02 -> /network/node002/public/process001/measurements/car02
              shared/   -> /network/node001/public/process001/shared/
      ```

    #### 7. **/service/**
    The `/service/` book contains the process instance creation service
    (PICS) locations for available processes. This book is advertised
    and replicated as needded in the whole network.
    - **Example Structure**:
      ```
      /service/ (dictionary)
          process001/ (dictionary)
              node -> /network/node001/service/
              node -> /network/node002/service/

      ```

    #### 8. **/process/**
    The `/process/` book is a repository that contains process
    description and specific resources (such as executables) necessary
    for the loading of various processes in the nodes. This book also
    contains private entry points for local node process loading. This
    book is advertised and replicated as needded in the whole network.
    - **Example Structure**:
      ```
      /process/ (dictionary)
          process001/ (dictionary)
              input/ (dictionary)
              output/ (dictionary)
              description
              executable/
              entry/( dictionary)
      ```

    #### 9. **/network/**
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
    - **/config/**: Maintains system-wide configuration files that affect
    all nodes and processes.
    - **/log/**: For comprehensive logging across the system, which
    could include logs from each node, process, and application.
    - **/temp/**: Temporary private records needed during execution,
    ensuring that transient data does not consume permanent storage space.

*/


#include "cdp_record.h"


// Initial CascadeDP system typeID:
enum _CDP_TYPE_ID {
    CDP_TYPE_NONE,          // This is the "no type" type.
    CDP_TYPE_TYPE,

    // Book/dict types
    CDP_TYPE_BOOK,
    CDP_TYPE_DICTIONARY,    // For bootstrapping reasons this must be 0x03 (see cdp_record.h)
    CDP_TYPE_CATALOG,
    CDP_TYPE_LIST,
    CDP_TYPE_SET,
    CDP_TYPE_QUEUE,
    CDP_TYPE_CHRONICLE,
    CDP_TYPE_ENCYCLOPEDIA,
    CDP_TYPE_COMPENDIUM,
    CDP_TYPE_COLLECTION,
    CDP_TYPE_LOG,

    // Register types
    CDP_TYPE_REGISTER,
    CDP_TYPE_PATCH,
    CDP_TYPE_NAMEID,
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

    CDP_TYPE_COUNT
};


// Initial CascadeDP system nameID:
enum _CDP_NAME_ID {
    CDP_NAME_Empty,     // This represents the empty string.

    CDP_NAME_NAME,
    CDP_NAME_VALUE,
    CDP_NAME_SIZE,
    CDP_NAME_DESCRIPTION,

    CDP_NAME_ROOT,      // For bootstrapping reasons this must be 0x05 (see cdp_record.h).
    CDP_NAME_TYPE,
    CDP_NAME_SYSTEM,
    CDP_NAME_USER,
    CDP_NAME_PRIVATE,
    CDP_NAME_PUBLIC,
    CDP_NAME_DATA,
    CDP_NAME_SERVICE,
    CDP_NAME_PROCESS,
    CDP_NAME_NETWORK,
    CDP_NAME_TEMP,

    CDP_NAME_COUNT
};



static inline cdpRecord* cdp_record_none(void)  {extern cdpRecord NONE; assert(NONE);  return NONE;}


cdpNameID cdp_name_id_add(const char* name, bool borrow);
#define cdp_name_id_add_static(name)  cdp_name_id_add(name, true);
cdpRecord* cdp_name_id_text(cdpNameID nameID);


static inline cdpRecord* cdp_record_add_uint32(cdpRecord* parent, cdpNameID name, unsigned value) {
    return cdp_record_add_register(parent, name, CDP_TYPE_UINT32, false, &value, sizeof(value));
}

static inline cdpRecord* cdp_record_add_text(cdpRecord* parent, cdpNameID name, const char* text) {
    return cdp_record_add_register(parent, name, CDP_TYPE_UTF8, false, text, strlen(text));
}

static inline cdpRecord* cdp_record_add_static_text(cdpRecord* parent, cdpNameID name, const char* text) {
    return cdp_record_add_register(parent, name, CDP_TYPE_UTF8, true, text, strlen(text));
}

typedef bool (*cdpCreate)(cdpRecord* instance, void** context);
typedef bool (*cdpInstance)(cdpRecord* instance, void* context);


cdpRecord* cdp_process_load(const char* name,
                            cdpCreate   create,
                            cdpInstance step,
                            cdpInstance destroy,
                            cdpInstance save,
                            cdpInstance restore);


cdpNameID cdp_system_enter_name(const char* name, size_t length, bool staticStr);
cdpRecord* cdp_system_name(cdpNameID);
#define cdp_system_enter_name_string(str)   cdp_system_enter_name(str, strlen(str), true)


unsigned cdp_system_enter_type(cdpNameID nameID, size_t baseSize);
cdpRecord* cdp_system_type(unsigned typeID);

void cdp_system_initiate(void);
bool cdp_system_tic(void);
void cdp_system_shutdown(void);

#endif
