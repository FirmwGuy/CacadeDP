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
    the Process Instance Creation Service (or PICS), a unit whose only
    task is to instantiate a local process by request. On the local node
    each process allowed to run is advertised (along with its capabilities
    and data), allowing other processes to connect to them all across
    the network.

    ### Types
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
    parentheses), and register value (following the ":"). Links are represented
    by the arrow "->", while UTF8 text content is represented by quoted text
    (its type is implied). The root of the tree is the directory "/".

    #### 1. **/type/**
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

    #### 2. **/system/**
    The `/system/` book is used for storing records related to each
    local process instance, such as input queues and output
    linkage to connected processes.
    - **Example Structure**:
      ```
      /system/ (dictionary)
          process001/ (dictionary)
              555/ (instance)
                  input/ (dictionary)
                      arg/ (queue)
                  output/ (dictionary)
                      result -> /instance/process002/557/input/arg
              556/ (instance)
                  input/ (dictionary)
                      arg/ (queue)
                  output/ (dictionary)
                      result -> /instance/process002/558/input/arg
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
    This book (inside a user's book) stores persistent records generated
    by processes and meant to be accessed only by the (owner) network
    user. This book is never replicated.
    - **Example Structure**:
      ```
      /user/ (dictionary)
          user1/ (dictionary)
              private/ (dictionary)
                  system/ (dictionary)
                      process01/ (dictionary)
                          555/ (private_instance)
                             states/ (book)
                          saved-data/ (book)
      ```

    #### 5. **/public/**
    The `/public/` book is used for storing public records generated by
    the processes in the local node. These records are advertised along
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
    The `/data/` book is a virtual space used for mapping distributed
    public records into a communal coherent structure. This includes
    registers and links as shared resources that might be accessed
    within the network.
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
    and replicated as needed in the whole network.
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
    book is advertised and replicated as needed in the whole network.
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


static inline cdpRecord* cdp_record_none(void)  {extern cdpRecord NONE; assert(NONE);  return NONE;}


cdpID cdp_name_id_add(const char* name, bool borrow);
#define cdp_name_id_add_static(name)  cdp_name_id_add(name, true);
cdpRecord* cdp_name_id_text(cdpID id);


typedef bool (*cdpCreate)(cdpRecord* instance, void** context);
typedef bool (*cdpInstance)(cdpRecord* instance, void* context);


cdpRecord* cdp_process_load(const char* name,
                            cdpCreate   create,
                            cdpInstance step,
                            cdpInstance destroy,
                            cdpInstance save,
                            cdpInstance restore);


cdpID cdp_system_enter_name(const char* name, size_t length, bool staticStr);
cdpRecord* cdp_system_name(cdpID);
#define cdp_system_enter_name_string(str)   cdp_system_enter_name(str, strlen(str), true)


unsigned cdp_system_enter_type(cdpID id, size_t baseSize);
cdpRecord* cdp_system_type(unsigned type);

void cdp_system_initiate(void);
bool cdp_system_tic(void);
void cdp_system_shutdown(void);

#endif
