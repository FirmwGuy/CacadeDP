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

#ifndef CDP_SYSTEM_H
#define CDP_SYSTEM_H


/*
    Cascade Data Objecting System - Layer 2
    ------------------------------------------

    ### System Overview:
    CascadeDP Layer 1 implemented a record solution, intended to be used
    as the basis of a RAM file system (similar to Plan 9). This Layer 2
    is designed to handle a distributed system, that is, data sharing
    and service management across a network of devices.

    ### Agent:
    An agent is a smart record that can receive, handle and send
    tasks to other agents, processing events and information in
    behalf of the contained data (and may even propagate record
    instances all across the network). In a way, agents are executable
    functions that "travel" along the data they are bound to.

    The agent system uses a factory pattern combined with dynamic type
    specification and validation. This approach enables structured
    creation of record and instances, ensuring adherence to defined
    types while supporting graph-like relationships with multiple
    parents.
    - **Encapsulation**: Centralizes branched record creation logic.
    - **Consistency**: Enforces structure and metadata for each record type.
    - **Flexibility**: Facilitates adding new agents and dynamic creation.
    - **Separation of Concerns**: Decouples creation from business logic.
    - **Context-Aware Initialization**: Adapts structures based on
    parent relationships and specific requirements.

    ### Action:
    Agents perform the action contained in the tasks they receive.
    Actions differ depending of the context they are called (or
    signaled). The context is specified by the role agents had in
    assembled systems.

    ### Cascade:
    A Cascade is a system of agents acting (signaling) over other
    agent's records. In the cascade, connections are made by linking
    some agent's record to another agent's record, in such a way that
    one single action will produce a sequence of actions (domino
    effect style).

    ### Directory Structure:
    The base agent system is shaped by an universal hierarchycal
    data structure.

    (Note: In the following CascadeDP structures, record entries have an
    ID represented with text or number, and/or record value following
    the colom. Links are represented by the arrow "->".)

    #### 1. ** /system/ **
    The `/system/` dictionary stores internal information needed by the
    local record system, for example tags, names, etc.
    - **Example Structure**:
      ```
      /system/
          tag/
              1: "list"
              2: "integer"
          name/
              1: "data"
              2: "public"
          selector/
              10: "urgent"
              11: "slow"
          inhibitor/
              1: "heated"
              2: "loaded"
      ```

    #### 2. ** /system/agency/ **
    The `agency/` dictionary (inside /system/) stores internal
    information needed for tasking (calling) agents. It classifies
    them by tag and task.
    - **Example Structure**:
      ```
      /system/
          agency/
              add/
                  int/ (tag)
                      agent: add_int()
                      call/ (queue)
                          101/ (task)
                              parent -> /system/agency/sum/int/task/10/
                              instance -> /system/cascade/pipeline01/agent001/adder
                      working/ (queue)
                          100/ (task)
                              parent -> /system/agency/sum/int/task/10/
                              instance -> /system/cascade/pipeline01/agent001/adder01
                              baby -> /system/cascade/pipeline01/agent001/adder01/op02
                              input/
                                  op1: 5
                              status/
                                  completion: 99
                      done/ (queue)
                          99/ (task)
                              parent -> /system/agency/sum/int/task/10/
                              instance -> /system/cascade/pipeline01/agent001/adder01
                              baby -> /system/cascade/pipeline01/agent001/adder01/op02
                              input/
                                  op1: 1
                              output/
                                  ans: 5
                              status/
                                  completion: 100
                      failed/
                  float/ (tag)
                      agent: add_float()
              multiply/
                  int/ (tag)
                      agent: mul_int()
                  float/ (tag)
                      agent: mul_float()
      ```

    #### 3. ** /system/cascade/ **
    The `cascade/` dictionary (inside /system/) is used for storing connection
    and link instructions between agents. It contains blueprints for creating
    agent cascades.
    - **Example Structure**:
      ```
      /system/
          /cascade/
              pipeline01/
                  agent001/
                      input/
                          arg/
                      output/
                          result -> /system/agent002/input/arg
                  agent002/
                      output/
                          result -> /system/agent003/input/arg
      ```

    #### 4. ** /user/ **
    This record serves as the personal space for user-specific
    configurations and data. Each user or administrative entity
    interacting with the nodes might have a separate entry here.
    This record may be replicated to other nodes.
    - **Example Structure**:
      ```
      /user/
          user1/
          user2/
      ```

    #### 5. ** ~/private/ **
    This record (inside a user's record) stores persistent records generated
    by agents and meant to be accessed only by the (owner) network
    user. This record is never replicated.
    - **Example Structure**:
      ```
      /user/
          user1/
              private/
                  system/
                      agent01/
                          555/
                             states/
                          556/
                             states/
                          saved-data/
      ```

    #### 6. ** /public/ **
    The `/public/` record is used for storing public records generated by
    the agents in the local node. These records are advertised along
    this node when it connects to the network and may be accessed
    (and/or cached/replicated) by other nodes.
    - **Example Structure**:
      ```
      /public/
          agent001/
              measurements/
                  car01/
              shared/
                  count:123
                  events/
      ```

    #### 7. ** /data/ **
    The `/data/` record is a virtual space used for mapping distributed
    public records into a communal coherent structure. This includes
    registers and links as shared resources that might be accessed
    within the network. This is replicated as needed.
    - **Example Structure**:
      ```
      /data/
          apps/
              agent001/
                  measurements/
                      car01 -> /network/node001/public/agent001/measurements/car01
                      car02 -> /network/node002/public/agent001/measurements/car02
                  shared/   -> /network/node001/public/agent001/shared/
      ```

    #### 8. ** /data/service/ **
    The `service/` record inside `/data/` contains the agent instance
    creation service (AICS) locations for available agents.
    - **Example Structure**:
      ```
      /data/
          service/
              agent001/
                  node -> /network/node001/system/agent001
                  node -> /network/node002/system/agent001

      ```

    #### 9. ** /network/ **
    This record is central to managing network-specific configurations
    and information about the reachability of each (foreign) connected
    node with respect to the local node.
    - **Example Structure**:
      ```
      /network/
          node001/
              protocol/
                  address
                  config/
                  status
      ```

    ### Additional Considerations:
    - ** /data/config/ **: Maintains system-wide configuration files that affect
    all nodes and agents.
    - ** /log/ and /data/log/ **: For comprehensive logging across the system,
    which could include logs from each node, agent, and application.
    - ** /temp/ **: Temporary private records needed during execution,
    ensuring that transient data does not consume permanent storage space.

*/


#include "cdp_record.h"


// Name IDs:
enum _cdpNameID {
    // Core directories
    CDP_NAME_SYSTEM = CDP_NAME_ID_INITIAL_COUNT,
    CDP_NAME_USER,
    CDP_NAME_PUBLIC,
    CDP_NAME_DATA,
    CDP_NAME_NETWORK,
    CDP_NAME_TEMP,

    // Core sub-dirs
    CDP_NAME_NAME,
    CDP_NAME_AGENCY,
    CDP_NAME_CASCADE,
    CDP_NAME_PRIVATE,

    // Basic fields
    CDP_NAME_CALL,
    CDP_NAME_DONE,
    CDP_NAME_WORK,

    CDP_NAME_PARENT,
    CDP_NAME_BABY,
    CDP_NAME_INSTANCE,
    CDP_NAME_SIZE,

    // Task I/O fields
    CDP_NAME_INPUT,
    CDP_NAME_OUTPUT,
    CDP_NAME_STATUS,

    CDP_NAME_DEBUG,
    CDP_NAME_WARNING,
    CDP_NAME_ERROR,
    CDP_NAME_FATAL,

    CDP_NAME_ID_SYSTEM_COUNT
};


#define CDP_NAME_SYSTEM_COUNT   (CDP_NAME_ID_SYSTEM_COUNT - CDP_NAME_SYSTEM)


typedef struct {
    cdpRecord*  agTag;
    cdpRecord*  task;
    cdpRecord*  input;
} cdpTask;


static inline cdpRecord* cdp_void(void)  {extern cdpRecord* CDP_VOID; assert(CDP_VOID);  return CDP_VOID;}

void       cdp_name_id_static_destructor(void* text);
cdpID      cdp_name_id_add(const char* name, cdpTag domain, cdpDel destructor);
#define    cdp_name_id_add_static(name)   cdp_name_id_add(name, CDP_DOMAIN_GLOBAL, cdp_name_id_static_destructor)
#define    cdp_tag_id_add_static(name)    cdp_name_id_add(name, domain, cdp_name_id_static_destructor)
#define    CDP_TAG   cdp_tag_id_add_static
#define    CDP_ID    cdp_name_id_add_static
cdpRecord* cdp_name_id_text(cdpID nameID);


cdpRecord* cdp_system_agency_add(cdpID name, cdpTag tag, cdpAgent agent);
bool       cdp_system_startup(void);
bool       cdp_system_step(void);
void       cdp_system_shutdown(void);


cdpRecord* cdp_agency(cdpID name);
bool       cdp_agency_set_agent(cdpRecord* agency, cdpTag tag, cdpAgent agent);


cdpRecord* cdp_task_begin(  cdpTask* task, cdpRecord* agency, cdpTag cast, cdpRecord* instance,
                            cdpRecord* parentTask, cdpRecord* baby,
                            int numInput, int numOutput );
cdpRecord* cdp_task_commit(cdpTask* task);



static inline cdpRecord* cdp_book_add_text(cdpRecord* record, unsigned attrib, cdpID id, bool borrow, const char* text)    {assert(cdp_record_children(record) && text && *text);  cdpRecord* reg = cdp_record_add_data(record, attrib, id, CDP_TAG_UTF8, borrow, text, strlen(text) + 1); reg->recData.reg.size--; return reg;}
#define cdp_book_add_static_text(b, id, text)   cdp_book_add_text(b, CDP_ATTRIB_FACTUAL, id, true, text)


#endif
