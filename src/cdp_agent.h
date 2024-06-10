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

#ifndef CDP_AGENT_H
#define CDP_AGENT_H


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
    signals to other agents, processing events and information in
    behalf of the contained data (and may even propagate record
    instances all across the network). In a way, agents are executable
    functions that "travel" along the data they are bound to.

    The agent system uses a factory pattern combined with dynamic type
    specification and validation. This approach enables structured
    creation of record and instances, ensuring adherence to defined
    types while supporting graph-like relationships with multiple
    parents.
    - **Encapsulation**: Centralizes book creation logic.
    - **Consistency**: Enforces structure and metadata for each book type.
    - **Flexibility**: Facilitates adding new agents and dynamic creation.
    - **Separation of Concerns**: Decouples creation from business logic.
    - **Context-Aware Initialization**: Adapts structures based on
    parent relationships and specific requirements.

    ### Action:
    Agents perform the action contained in the signals they receive.
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

    (Note: In the following CascadeDP structures, book entries have an
    ID represented with text or number, and/or register value following
    the colom. Links are represented by the arrow "->".)

    #### 1. ** /agent/ **
    The `/agent/` dictionary stores internal information needed by the
    local record system about agents. It associates their ID with their
    name and available actions.
    - **Example Structure**:
      ```
      /agent/
          5/
              name:"catalog"
              parent/
                  1 -> /type/3
              collection/
                  add -> "catalog_add()"
                  remove -> "catalog_remove()"
          8/
              name:"log"
              parent/
                  1 -> /type/5
              collection/
                  remove -> "log_remove()"
          9/
              name:"boolean"
              description: "Boolean value."
              value/
                  0:"false"
                  1:"true"
              size:1
      ```

    #### 2. ** /system/ **
    The `/system/` dictionary is used for storing connection and link
    instructions between agents. It is a blueprint for creating agent
    cascades.
    - **Example Structure**:
      ```
      /system/
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

    #### 3. ** /user/ **
    This book serves as the personal space for user-specific
    configurations and data. Each user or administrative entity
    interacting with the nodes might have a separate entry here.
    This book may be replicated to other nodes.
    - **Example Structure**:
      ```
      /user/
          user1/
          user2/
      ```

    #### 4. ** ~/private/ **
    This book (inside a user's book) stores persistent records generated
    by agents and meant to be accessed only by the (owner) network
    user. This book is never replicated.
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

    #### 5. ** /public/ **
    The `/public/` book is used for storing public records generated by
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

    #### 6. ** /data/ **
    The `/data/` book is a virtual space used for mapping distributed
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

    #### 7. ** /data/service/ **
    The `service/` book inside `/data/` contains the agent instance
    creation service (AICS) locations for available agents.
    - **Example Structure**:
      ```
      /data/
          service/
              agent001/
                  node -> /network/node001/system/agent001
                  node -> /network/node002/system/agent001

      ```

    #### 8. ** /data/agent/ **
    The `agent/` book inside `/data/` is a repository that contains
    agent specific resources (such as executables) necessary for
    the propagation and loading of various agents across the nodes.
    - **Example Structure**:
      ```
      /data/
          type/
              agent001/
                  description
                  executable -> /network/node002/public/agent001/executable
      ```

    #### 9. ** /network/ **
    This book is central to managing network-specific configurations
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


static inline cdpRecord* cdp_record_void(void)  {extern cdpRecord* CDP_VOID; assert(CDP_VOID);  return CDP_VOID;}

enum {
    CDP_BOOLEAN_FALSE,
    CDP_BOOLEAN_TRUE,

    CDP_BOOLEAN_COUNT
};

enum {
    CDP_CALL_STARTUP,
    CDP_CALL_SHUTDOWN,

    CDP_CALL_CONSTRUCT,
    CDP_CALL_DESTRUCT,
    CDP_CALL_REFERENCE,
    CDP_CALL_FREE,

    CDP_CALL_APPEND,
    CDP_CALL_PREPEND,
    CDP_CALL_INSERT,
    CDP_CALL_UPDATE,
    CDP_CALL_REMOVE,

    //CDP_CALL_SORT,
    //CDP_CALL_COPY,
    //CDP_CALL_MOVE,
    //CDP_CALL_PATCH,
    //CDP_CALL_LINK,

    CDP_CALL_SERIALIZE,
    CDP_CALL_TEXTUALIZE,

    CDP_CALL_COUNT
};


cdpID      cdp_name_id_add(const char* name, bool borrow);
#define    cdp_name_id_add_static(name)   cdp_name_id_add(name, true);
cdpRecord* cdp_name_id_text(cdpID id);


typedef struct {
    unsigned length;
    cdpID    assimilate[];
} cdpAssimilation;

cdpID cdp_agent_add(const char* name,
                    size_t baseSize,
                    cdpAssimilation assimilation,
                    cdpAction create,
                    cdpAction destroy,
                    cdpAction startup,
                    cdpAction shutdown);

bool cdp_agent_add_action(cdpID agentID, cdpID contextID, cdpAction action);

cdpRecord* cdp_agent(cdpID id);

static inline cdpAction cdp_agent_action(cdpID typeID) {
    cdpRecord* agent = cdp_type(typeID);
    assert(agent);
    cdpAction action = cdp_book_find_by_name(agent, CDP_NAME_CALL);
    assert(action);
    return action;
}

void       cdp_agent_construct(cdpRecord* agent, cdpID nameID, cdpID agentTypeID, cdpID storage, uint32_t base);
void       cdp_agent_destruct (cdpRecord* agent);
void       cdp_agent_reference(cdpRecord* agent);
void       cdp_agent_free     (cdpRecord* agent);

cdpRecord* cdp_agent_append   (cdpRecord* agent, cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_agent_prepend  (cdpRecord* agent, cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_agent_insert   (cdpRecord* agent, cdpRecord* book, cdpRecord* record);

bool       cdp_agent_update(cdpRecord* agent, cdpRecord* record, void* data, size_t size);
bool       cdp_agent_remove(cdpRecord* agent, cdpRecord* book, cdpRecord* record);

//void       cdp_agent_sort(cdpRecord* agent);

bool       cdp_agent_validate(cdpRecord* agent);


bool       cdp_system_startup(void);
bool       cdp_system_step(void);
void       cdp_system_shutdown(void);


#endif
