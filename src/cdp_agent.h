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

    #### 1. ** /system/ **
    The `/system/` dictionary stores internal information needed by the
    local record system about agents. It associates their ID with their
    name and available actions.
    - **Example Structure**:
      ```
      /system/
          5/
              name:"catalog"
              assimilate/
                  1 -> /type/3
              collection/
                  add -> "catalog_add()"
                  remove -> "catalog_remove()"
          8/
              name:"log"
              assimilate/
                  1 -> /type/5
              collection/
                  remove -> "log_remove()"
          9/
              name:"boolean"
              value/
                  0:"false"
                  1:"true"
              size:1
      ```

    #### 2. ** /cascade/ **
    The `/cascade/` dictionary is used for storing connection and link
    instructions between agents. It contains blueprints for creating
    agent cascades.
    - **Example Structure**:
      ```
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


static inline cdpRecord* cdp_void(void)  {extern cdpRecord* CDP_VOID; assert(CDP_VOID);  return CDP_VOID;}


enum {
    CDP_VALUE_FALSE,
    CDP_VALUE_TRUE,

    CDP_VALUE_BOOLEAN_COUNT
};


// Name IDs:
enum _cdpNameID {
    // Core directories
    CDP_NAME_SYSTEM = CDP_NAME_INITIAL_COUNT,
    CDP_NAME_CASCADE,
    CDP_NAME_USER,
    CDP_NAME_PRIVATE,
    CDP_NAME_PUBLIC,
    CDP_NAME_DATA,
    CDP_NAME_NETWORK,
    CDP_NAME_TEMP,

    // Basic fields
    CDP_NAME_ASSIMILATE,
    CDP_NAME_NAME,
    CDP_NAME_SIZE,
    CDP_NAME_ENUMERATION,
    //
    CDP_NAME_AGENT,
    CDP_NAME_ACTION,
    CDP_NAME_OUTPUT,
    CDP_NAME_DEBUG,
    CDP_NAME_WARNING,
    CDP_NAME_ERROR,
    CDP_NAME_FATAL,

    CDP_NAME_FLAG_COUNT
};

#define CDP_NAME_COUNT  (CDP_NAME_FLAG_COUNT - CDP_NAME_VOID)


cdpID      cdp_name_id_add(const char* name, bool borrow);
#define    cdp_name_id_add_static(name)   cdp_name_id_add(name, true);
cdpRecord* cdp_name_id_text(cdpID nameID);


cdpID cdp_system_set_agent( const char* name,
                            size_t      baseSize,
                            unsigned    assimLenght,
                            cdpID*      assimilate,
                            unsigned    numAction,
                            cdpAction   initiate,
                            cdpAction   finalize );
cdpRecord* cdp_system_get_agent(cdpID id);

cdpID      cdp_system_set_action(cdpID agentID, const char* name, cdpAction action);
cdpAction  cdp_system_get_action(cdpID agentID, cdpID actionID);
bool       cdp_system_does_action(cdpRecord* instance, cdpSignal* signal);

bool       cdp_system_startup(void);
bool       cdp_system_step(void);
void       cdp_system_shutdown(void);


#endif
