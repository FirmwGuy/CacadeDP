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

#ifndef CDP_SYSTEM_H
#define CDP_SYSTEM_H


#include "cdp_record.h"


/*
    Cascade Data Objecting System - Layer 2
    ------------------------------------------

    ### System Overview:
    CascadeDP Layer 1 implemented a record solution, intended to be used
    as the basis of a RAM file system (similar to Plan 9). This Layer 2
    is designed to handle a distributed execution, that is, data sharing
    and propagation of user-provided routines across a network of devices.

    ### Agent:
    An agent is a "smart" record that can receive, handle and send
    actions to other agents, processing events and information in
    behalf of the contained data (and may even propagate record
    instances all across the network). In a way, agents are executable
    functions that "travel" along the data they are bound to.

    ### Action:
    Agents perform the action contained in the tasks they receive.
    Actions differ depending of the instance they are called (or
    signaled). The instance is specified by the role agents had in
    assembled systems. There are only a handfull of actions, but their
    meaning depend on how the agent treats them:

    - **Instance New**: Used by agents to create a new instance record.
    - **Instance Validate**: Used by agents to validate an existing instance record.
    - **Instance Inlet**: Used by agents to report/create a named input record in
    provided (self) context, suitable for future connections.
    - **Instance Connect**: Used by agents to link one of its named output records
    to the provided context input (from posibly different agent) .
    - **Instance Unplug**: Used by agents to break/remove one of its connected outputs.
    - **Data New**: Used by agents to report/create data in the context record.
    - **Data Update**: Used by agents to update data in the context record.
    - **Data Delete**: Used by agents to delete data in the context record.
    - **Store New**: Used by agents to report/create a child store in the context record.
    - **Store Add Item**: Used by agents to add a child record to context.
    - **Store Remove Item**: Used by agents to remove a child record from context.
    - **Store Delete**: Used by agents to delete child store and all children in t context.

    ### Cascade:
    A Cascade is a system of agents acting (signaling) over other
    agent's records. In the cascade, connections are made by linking
    some agent's record to another agent's record, in such a way that
    one single action will produce a sequence of actions (domino
    effect style) that propagates as needed to all connected context nodes.

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


/*
    Domain:
        'CDP'

    Agencies:
        'step'
        'buffer'
        'cloner'
        'converter'
        'step'
        'synchronizer'

        'data-update'
        'store-add'
        'store-append'

    Statuses:
        'pending'
        'working'
        'completed'
        'failed'

    Events:
        'debug'
        'warning'
        'error'
        'fatal'

    Actions:
        CDP_ACTION_DATA_UPDATE,
        CDP_ACTION_DATA_NEW,
        CDP_ACTION_DATA_DELETE,
        //
        CDP_ACTION_STORE_ADD_ITEM,
        CDP_ACTION_STORE_REMOVE_ITEM,
        CDP_ACTION_STORE_NEW,
        CDP_ACTION_STORE_DELETE,
        //
        CDP_ACTION_INSTANCE_INITIATE,
        CDP_ACTION_INSTANCE_VALIDATE,
        CDP_ACTION_INSTANCE_INLET,
        CDP_ACTION_INSTANCE_CONNECT,
        CDP_ACTION_INSTANCE_UNPLUG,
        CDP_ACTION_INSTANCE_CLEAN,
        //
        CDP_ACTION_PIPELINE_ASSEMBLED,
        CDP_ACTION_PIPELINE_STARTING,
        CDP_ACTION_PIPELINE_RUNNING,
        CDP_ACTION_PIPELINE_PAUSED,
        CDP_ACTION_PIPELINE_COMPLETED,
        //
        CDP_ACTION_REMOTE_INPUT,
        CDP_ACTION_REMOTE_CONNECTED,
        CDP_ACTION_REMOTE_WAITING,
        CDP_ACTION_REMOTE_BLOCKED,
        CDP_ACTION_REMOTE_INTERRUPTED,
        CDP_ACTION_REMOTE_ERROR,
        CDP_ACTION_REMOTE_FAILED,

    Status:
        CDP_STATUS_FAIL = -1,
        CDP_STATUS_OK,
        CDP_STATUS_PROGRESS,
        CDP_STATUS_SUCCESS

    Logs:
         CDP_LOG_DEBUG,
        CDP_LOG_LOG,
        CDP_LOG_WARNING,
        CDP_LOG_ERROR,
        CDP_LOG_FATAL


    Core directories:
        'data'
        'network'
        'public'
        'private'
        'system'
            'agent'
            'cascade'
            'domain'
            'library'
        'temp'
        'user'

*/


typedef bool (*cdpAgent)(cdpRecord* instance, cdpRecord* call);


bool  cdp_system_startup(void);
bool  cdp_system_step(void);
void  cdp_system_shutdown(void);


bool cdp_agency_set_agent(cdpID domain, cdpID agency, cdpID consumption, cdpAgent agent);
bool cdp_agency_set_produ(cdpID domain, cdpID agency, cdpID product);

cdpRecord* cdp_record_add_agency_instance(  cdpRecord* record, cdpID name, uintptr_t context,
                                            cdpID domain, cdpID agency,
                                            cdpRecord* customerI );
#define cdp_dict_add_agency_instance(dict, name, domain, agency, customerI)     cdp_record_add_agency_instance(dict, name, 0, domain, agency, customerI)

void cdp_agency_instance_dispose(cdpRecord* instance);
bool cdp_agency_instance_request(cdpRecord* instance, cdpID type, cdpRecord* message);

bool cdp_agency_pipeline_create(cdpRecord* selfI, cdpID name);
bool cdp_agency_pipeline_state(cdpRecord* selfI, cdpID pipeline, cdpID state);
bool cdp_agency_pipeline_dispose(cdpRecord* selfI, cdpID pipeline);

bool cdp_agency_product_connect(cdpRecord* selfI, cdpID pipeline, cdpRecord* providerI, cdpID product, cdpRecord* consumerI, cdpID consumption);
bool cdp_agency_product_deliver(cdpRecord* selfI, cdpID product, cdpRecord* content);

bool cdp_agency_customer_answer(cdpRecord* selfI, cdpID type, cdpRecord* answer);
bool cdp_agency_customer_log(cdpRecord* selfI, cdpID type, cdpRecord* log);
//static inline cdpRecord* cdp_void(void)  {extern cdpRecord* CDP_VOID; assert(CDP_VOID);  return CDP_VOID;}

//static inline cdpRecord* cdp_agent_step(void)  {extern cdpRecord* CDP_STEP; assert(CDP_STEP);  return CDP_STEP;}


#endif
