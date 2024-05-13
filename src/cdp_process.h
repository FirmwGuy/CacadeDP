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
    is designed to handle data sharing and device management across a 
    distributed network of procesess. Each process can advertise its 
    capabilities and data, allowing for efficient resource sharing and 
    management within the network.

    ### Directory Structure:
    The directory structure is explained using CascadeDP records, hece 
    with each entry having a nameID (represented with text), a type 
    (enclosed in parentheses), and an index (represented here inside 
    brackets). Registers may have values (following the ":"), while links 
    are represented by "->". UTF8 text content is represented by quoted
    text (its type is implied). The root of the tree is the dictionary "/".

    #### 1. **/type/**
    The `/type/` directory stores internal information needed by the 
    (distributed) record system about types. This is local to each node.
    - **Example Structure**:
      ```
      /type/ (log)
          [0] type/ (type)
              name:"none"
              description: "A type for describing nothingness."
          [1] type/ (type)
              name:"type"
              description: "A type for describing other types."
          [2] type/ (type)
              name:"book"
              description: "A generic container of records."
          [3] set/ (type)
              name:"set"
              description: "A book of unsorted records with unique names."
          [4] type/ (type)
              name:"catalog"
              description: "A book with records ordered by some criteria."
          [5] type/ (type)
              name:"dictionary"
              description: "A catalog of records sorted by their unique name."
          [6] list/ (type)
              name:"list"
              description: "A book that adds/removes only on his beginning and/or end."
          [7] queue/ (type)
              name:"queue"
              description: "A list that only removes from its beginning and adds to its end."
          [8] type/ (type)
              name:"chronicle"
              description: "A book that never removes records."
          [9] type/ (type)
              name:"collection"
              description: "A set that never removes records."
          [10] type/ (type)
              name:"compendium"
              description: "A catalog that never removes records."
          [11] type/ (type)
              name:"encyclopedia"
              description: "A dictionary that never removes records."
          [12] type/ (type)
              name:"log"
              description: "A queue that never removes records."
              
          [13] type/ (type)
              name:"register"
              description: "A generic record that holds data."
          [14] type/ (type)
              name:"nameid"
              description: "A name token for creating record paths."
              value/ (log)
                  [0] value:""
                  [1] value:"/"
                  [2] value:"type"
                  [3] value:"system"
          /[] name/ (type)
              /name:"name"
              /description: "A register where the only data is its own nameID."
          /[7] type/
              /name:"utf8"
              /description: "A register with text in UTF8 format."
          /[] boolean (type)
              /name:"boolean"
              /description: "A boolean value."
              /value/ (log)
                  /[0] value:"false"
                  /[1] value:"true"
              /size:1 (unsigned)
          /[] byte (type)
              /name:"byte"
              /description: "A register with a unsigned 8 bit integer (byte) value."
              /size:1 (unsigned)
          /[] uword (type)
              /name:"uword"
              /description: "A register with a unsigned 16 bit integer (word) value."
              /size:2 (unsigned)
          /[] unsigned (type)
              /name:"unsigned"
              /description: "A register with a unsigned 32 bit integer value."
              /size:4 (unsigned)
          /[] qword (type)
              /name:"qword"
              /description: "A register with an unsigned 64 bit integer (quad-word) value."
              /size:8 (unsigned)
          /shorti
          /integer
          /longi
          /float
          /double
      ```

    #### 2. **/system/**
    The `/system/` directory is used for storing (per process) 
    private records generated or used by the local node. This includes 
    registers and links as local resources that might be only accessed 
    inside each process instance.
    - **Example Structure**:
      ```
      /system/ (dictionary)
          process001/ (catalog)
              instance/ (dictionary)
                  id:555 (unsigned)
                  input/ (dictionary)
                      arg
                  output/ (dictionary)
                      result -> /instance/process002/instance(id=557)/input/arg
              instance/ (dictionary)
                  id:556 (unsigned)
                  input/ (dictionary)
                      arg
                  output/ (dictionary)
                      result -> /instance/process002/instance(id=558)/input/arg
      ```

    #### 3. **/user/**
    This directory serves as the personal space for user-specific 
    configurations and data. Each user or administrative entity 
    interacting with the nodes might have a separate subdirectory here.
    This directory may be replicated to other nodes.
    - **Example Structure**:
      ```
      /user/ (dictionary)
          user1/ (dictionary)
          user2/ (dictionary)
      ```

    #### 4. **~/private/**
    This sub-directory (inside a user's directory) stores peristent 
    records generated by processes and meant to be accessed only by the 
    (owner) user inside the current node. This directory is never 
    replicated.
    - **Example Structure**:
      ```
      /user/ (dictionary)
          user1/ (dictionary)
              private/ (dictionary)
                  process01/ (book)
                      saved-data/ (book)
      ```

    #### 5. **/public/**
    The `/public/` directory is used for storing public records 
    generated or used by the processes in the network. This records are 
    advertised along this node when it connects to the network and may 
    be accessed (and/or cached/replicated) by other nodes.
    - **Example Structure**:
      ```
      /public/ (dictionary)
          process001/ (dictionary)
              instance/ (catalog)
                  id:555 (unsigned)
                  measurements/ (dictionary)
                      car01/ (dictionary)
              shared/ (dictionary)
                  count:123 (unsigned)
                  events/ (queue)
      ```

    #### 6. **/data/**
    The `/data/` directory is a virtual space used for accessing public 
    records generated or used by the processes in connected nodes. 
    This includes registers and links as shared resources that might be 
    accessed within the network.
    - **Example Structure**:
      ```
      /data/ (dictionary)
          process001/ (dictionary)
              measurements/ (dictionary)
                  car01 -> /network/node001/public/process001/instance(id=555)/measurements/car01
                  car02 -> /network/node002/public/process001/instance(id=333)/measurements/car02
              shared/   -> /network/node001/public/process001/shared/
      ```

    #### 7. **/process/**
    The `/process/` directory contains service description and process 
    specific resources (such as executables) necessary for the 
    registration, adverstising and loading of various processes in the
    nodes. This directory is replicated as needded in the whole network.
    - **Example Structure**:
      ```
      /process/ (dictionary)
          process001/ (dictionary)
              node -> /network/node001/
              input/ (dictionary)
              output/ (dictionary)
              description
              bin
      ```

    #### 8. **/network/**
    This directory is central to managing network-specific 
    configurations and information about the reachability of each 
    (foreign) connected node with respect to the local one.
    - **Example Structure**:
      ```
      /network/ (dictionary)
          node001/ (dictionary)
              protocol (dictionary)
                  address
                  config/
                  status
              public/ (dictionary)
      ```

    ### Additional Considerations:

    - **/config/**: Maintains system-wide configuration files that affect 
    all nodes and processes.
    - **/log/**: For comprehensive logging across the system, which 
    could include logs from each node, process, and application.
    - **/temp/**: Temporarily private records needed during execution,
    ensuring that transient data does not consume permanent storage space.
    - **/lib/**: Contains shared executables, libraries and drivers 
    needed by applications in `/process/`.

*/


#include "cdp_record.h"


// Initial CascadeDP system typeID:
enum _CDP_TYPE_ID {
    CDP_TYPE_NONE,      // This is the "no type" type.
    
    // Book/dict types
    CDP_TYPE_TYPE,
    CDP_TYPE_BOOK,
    CDP_TYPE_SET,
    CDP_TYPE_CATALOG,
    CDP_TYPE_DICTIONARY,
    CDP_TYPE_LIST,
    CDP_TYPE_QUEUE,
    CDP_TYPE_CHRONICLE,
    CDP_TYPE_COLLECTION,
    CDP_TYPE_COMPENDIUM,
    CDP_TYPE_ENCYCLOPEDIA,
    CDP_TYPE_LOG,
    
    // Register types
    CDP_TYPE_REGISTER,
    CDP_TYPE_NAMEID,
    CDP_TYPE_NAME,
    CDP_TYPE_UTF8,
    CDP_TYPE_BOOLEAN,
    CDP_TYPE_UINT8,
    CDP_TYPE_UINT16,
    CDP_TYPE_UINT32,
    CDP_TYPE_UINT64,
    CDP_TYPE_SIGN16,
    CDP_TYPE_SIGN32,
    CDP_TYPE_SIGN64,
    CDP_TYPE_FLOAT32,
    CDP_TYPE_FLOAT64,
    
    CDP_TYPE_COUNT
};


// Initial CascadeDP system nameID:
enum _CDP_NAME_ID {
    CDP_NAME_Empty,     // This represents the empty string.
    CDP_NAME_ROOT,      // For bootstrapping reasons this must be 0x01.
    CDP_NAME_TYPE,
    CDP_NAME_NAME,
    CDP_NAME_VALUE,
    //
    CDP_NAME_SYSTEM,
    CDP_NAME_USER,
    CDP_NAME_PRIVATE,
    CDP_NAME_PUBLIC,
    CDP_NAME_DATA,
    CDP_NAME_PROCESS,
    CDP_NAME_NETWORK
};


static inline cdpRecord* cdp_record_add_unsigned(cdpRecord* parent, cdpNameID name, unsigned value) {
    return cdp_record_add_register(parent, name, CDP_ID_UINT32, false, &value, sizeof(value));
}


typedef bool (*cdpCreate)(cdpRecord* instance, void** context);
typedef bool (*cdpInstance)(cdpRecord* instance, void* context);


cdpRecord* cdp_process_load(const char* name,
                            cdpCreate   create,
                            cdpInstance tic,
                            cdpInstance save,
                            cdpInstance restore,
                            cdpInstance destroy);


cdpNameID cdp_system_enter_name(const char* name, size_t length, bool staticStr);
cdpRecord* cdp_system_name(cdpNameID);
#define cdp_system_enter_name_string(str)   cdp_system_enter_name(str, strlen(str), true)


unsigned cdp_system_enter_type(cdpNameID nameID, size_t baseSize);
cdpRecord* cdp_system_type(unsigned typeID);

void cdp_system_initiate(void);
bool cdp_system_tic(void);
void cdp_system_shutdown(void);

#endif
