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


#include "cdp_system.h"
#include "domain/cdp_binary.h"


extern cdpRecord CDP_ROOT;

cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* DOMAIN;
//cdpRecord* LIBRARY;

cdpRecord* CDP_STEP;
cdpRecord* CDP_VOID;




static void system_initiate(void) {
    cdp_record_system_initiate();

    // Initiate root structure
    cdpRecord* system  = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD("system"),  CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_ARRAY, 4);

    USER    = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD("user"),    CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_RED_BLACK_T);
    PUBLIC  = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD("public"),  CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_RED_BLACK_T);
    DATA    = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD("data"),    CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_RED_BLACK_T);
    NETWORK = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD("network"), CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_RED_BLACK_T);

    TEMP    = cdp_dict_add_list(&CDP_ROOT, CDP_WORD("temp"), CDP_ACRO("CDP"), CDP_WORD("list"), CDP_STORAGE_LINKED_LIST);

    // Initiate system structure
    DOMAIN  = cdp_dict_add_dictionary(system, CDP_WORD("domain"), CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_RED_BLACK_T);
    //LIBRARY = cdp_dict_add_dictionary(system, CDP_WORD_LIBRARY, CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_RED_BLACK_T);

    // Add system agents
    cdp_system_register_agent(CDP_ACRO("CDP"), CDP_WORD("step"), agent_system_step);

    // Initiate global records.
    cdpRecord step = {0};
    cdp_instance_new(cdp_root(), &step, CDP_WORD("step"), CDP_ACRO("CDP"), CDP_WORD("step"), NULL, CDP_V(0));
    CDP_STEP = cdp_dict_add(DOMAIN, &step);

    //CDP_VOID = cdp_record_append_value(TEMP, CDP_WORD_VOID, CDP_ACRO("CDP"), CDP_WORD_VOID, 0, 0, sizeof(bool), sizeof(bool));
    //CDP_VOID->data->writable = false;
}


void cdp_system_register_agent(cdpID domain, cdpID agency, cdpID action, cdpAgent agent) {
    assert(cdp_id_valid(domain) && cdp_id_valid(tag) && cdp_id_valid(agency) && cdp_id_valid(action) && agent);

    if (!DOMAIN)
        system_initiate();

    cdpRecord* rdomain = cdp_record_find_by_name(DOMAIN, domain);
    if (!rdomain)
        rdomain = cdp_dict_add_dictionary(DOMAIN, domain, CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_RED_BLACK_T);

    cdpRecord* ragency = cdp_record_find_by_name(rdomain, agency);
    if (!ragency)
        ragency = cdp_dict_add_dictionary(rdomain, agency, CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_ARRAY, 3);

    cdpRecord* raction = cdp_record_find_by_name(ragency, CDP_WORD("action"));
    if (!raction)
        raction = cdp_dict_add_dictionary(ragency, CDP_WORD("action"), CDP_ACRO("CDP"), CDP_WORD("dictionary"), CDP_STORAGE_ARRAY, 32);     // ToDo: better agent number estimation.

    cdpRecord* ragent = cdp_record_find_by_name(raction, action);
    if (ragent)
        //cdp_record_update(ragent, agent);
    else
        cdp_dict_add_binary_agent(raction, action, agent);
}


cdpAgent cdp_system_agent(cdpID domain, cdpID tag) {
    if (!AGENT)
        return NULL;

    for (cdpAgentList* list = AGENT;  list;  list = list->next) {
        if (list->domain == domain  &&  list->tag == tag)
            return list->agent;
    }

    return NULL;
}


bool cdp_system_startup(void) {
    assert(DOMAIN);
    // ToDo: Traverse all records. On each record, call the "startup" agency.
    return true;
}


bool cdp_system_step(void) {
    assert(DOMAIN);

    static uint64_t tic;

    if CDP_RARELY(!cdp_instance_data_update(cdp_root(), CDP_STEP, sizeof(uint64_t), sizeof(uint64_t), CDP_V(tic++)))
        return false;

    // ToDo: traverse all agents. On each agent, do jobs listed on "work", then move them to "done".
    return true;
}


void cdp_system_shutdown(void) {
    assert(DOMAIN);

    // ToDo: Traverse all records. On each record, call the "shutdown" agency.

    //cdp_system_finalize_tasks();
    cdp_record_delete_children(&CDP_ROOT);
    cdp_record_system_shutdown();

    DOMAIN = NULL;
}


void cdp_system_log(cdpRecord* instance, const char* message) {
    // ToDo: pass evemt to (registered) interface.
}




/*
 * Cascade API
 */


bool cdp_instance_agent_call_data(cdpRecord* instance, cdpCall* call) {
    assert(!cdp_record_is_floating(instance) && call && cdp_record_has_data(instance));

    for (cdpAgentList* list = instance->data->agent;  list;  list = list->next) {
        if (!list->agent(instance, call))
            return false;
    }

    return true;
}


bool cdp_instance_agent_call_store(cdpRecord* instance, cdpCall* call) {
    assert(!cdp_record_is_floating(instance) && call && cdp_record_has_store(instance));

    for (cdpAgentList* list = instance->store->agent;  list;  list = list->next) {
        if (!list->agent(instance, call))
            return false;
    }

    return true;
}


static inline bool cdp_instance_agent_call(cdpRecord* instance, cdpCall* call) {
    assert(!cdp_record_is_floating(instance) && call);

    if (cdp_record_has_data(self))
        return cdp_instance_agent_call_data(instance, call);

    else // FixMe: check agent calling policy to avoid calling twice the same agent over the same instance!
    if (cdp_record_has_store(self))
        return cdp_instance_agent_call_store(instance, call);

    return false;
}


static inline bool cdp_instance_initiate(cdpRecord* instance, cdpCall* call, cdpID domain, cdpID tag) {
    assert(!cdp_record_is_floating(instance) && call && cdp_id_valid(domain) && cdp_id_valid(tag));
    cdp_call_clean(call);

    cdpAgent agent = cdp_system_agent(domain, tag);
    if (!agent) {
        call->status  = CDP_STATUS_FAIL;
        call->log     = CDP_LOG_FATAL;
        call->message = "Agent not found."
        return false;
    }

    call->action = CDP_ACTION_INSTANCE_INITIATE;
    call->domain = domain;
    call->name   = tag;

    if (!agent(instance, call))
        return false;

    if (cdp_record_has_data(instance))
        cdp_data_add_agent(instance->data, domain, tag, agent);
    else
    if (cdp_record_has_store(instance))
        cdp_store_add_agent(instance->store, domain, tag, agent);

    call->status = CDP_STATUS_OK;

    return true;
}


static inline bool cdp_instance_validate(cdpRecord* instance, cdpCall* call) {
    assert(!cdp_record_is_floating(instance) && call);
    cdp_call_clean(call);

    call->action = CDP_ACTION_INSTANCE_VALIDATE;

    return cdp_instance_agent_call(instance, call);
}


static inline cdpRecord* cdp_instance_inlet(cdpRecord* instance, cdpCall* call, cdpID inlet) {
    assert(!cdp_record_is_floating(instance) && call && cdp_id_text_valid(inlet));
    cdp_call_clean(call);

    call->action = CDP_ACTION_INSTANCE_INLET;
    call->name = inlet;

    if (!cdp_instance_agent_call(instance, call))
        return NULL;

    return call->result;
}


static inline cdpRecord* cdp_instance_connect(cdpRecord* instance, cdpCall* call, cdpID output, cdpRecord* inlet) {
    assert(!cdp_record_is_floating(instance) && call && cdp_id_text_valid(output) && !cdp_record_is_floating(inlet));
    cdp_call_clean(call);

    call->action = CDP_ACTION_INSTANCE_CONNECT;
    call->record = inlet;
    call->name   = output;

    if (!cdp_instance_agent_call(instance, call))
        return NULL;

    return call->result;
}


static inline bool cdp_instance_unplug(cdpRecord* instance, cdpCall* call, cdpRecord* output) {
    assert(!cdp_record_is_floating(instance) && call && !cdp_record_is_floating(output));
    cdp_call_clean(call);

    call->action = CDP_ACTION_INSTANCE_UNPLUG;
    call->record = output;

    return cdp_instance_agent_call(instance, call);
}


static inline bool cdp_instance_clean(cdpRecord* instance, cdpCall* call) {
    assert(!cdp_record_is_floating(instance) && call);
    cdp_call_clean(call);

    call->action = CDP_ACTION_INSTANCE_CLEAN;

    return cdp_instance_agent_call(instance, call);
}


static inline bool cdp_instance_data_new(cdpRecord* instance, cdpCall* call, cdpData* data) {
    assert(!cdp_record_is_floating(instance) && call && cdp_data_valid(data));
    cdp_call_clean(call);

    cdpRecord* target = cdp_link_pull(instance);

    if (cdp_record_has_data(target)) {
        call->status  = CDP_STATUS_FAIL;
        call->log     = CDP_LOG_FATAL;
        call->message = "Target record already has data."
        return false;
    }

    // cdp_record_set_data(target, data);

    call->action = CDP_ACTION_DATA_NEW;
    call->record = target;
    call->data   = data;

    return cdp_instance_agent_call_data(target->instance, call);
}


bool cdp_instance_data_update(cdpRecord* self, cdpRecord* output, cdpData* data) {
    assert(!cdp_record_is_floating(instance) && call && cdp_data_valid(data));
    cdp_call_clean(call);

    cdpRecord* target = cdp_link_pull(instance);

    if (!cdp_record_has_data(target)) {
        call->status  = CDP_STATUS_FAIL;
        call->log     = CDP_LOG_FATAL;
        call->message = "Target record has no data."
        return false;
    }

    //cdp_record_update(target, size, capacity, data, false);

    call->action = CDP_ACTION_DATA_UPDATE;
    call->record = target;
    call->data   = data;

    return cdp_instance_agent_call_data(target->instance, call);
}


static inline bool cdp_instance_data_dalete(cdpRecord* instance, cdpCall* call) {
    assert(!cdp_record_is_floating(instance) && call && cdp_data_valid(data));
    cdp_call_clean(call);

    cdpRecord* target = cdp_link_pull(instance);

    if (!cdp_record_has_data(target)) {
        call->status  = CDP_STATUS_OK;
        call->log     = CDP_LOG_WARNING;
        call->message = "Target record has no data."
        return true;
    }

    //cdp_record_delete_data(target);

    call->action = CDP_ACTION_DATA_DELETE;
    call->record = target;

    return cdp_instance_agent_call_data(target->instance, call);
}


static inline int cdp_instance_store_new(cdpRecord* client, cdpStore** returned, cdpRecord* self, cdpID domain, cdpID tag, cdpRecord* params, cdpValue value) {
    self = cdp_link_pull(self);
    assert(cdp_record_is_dictionary(pipeline) && !cdp_record_has_store(self));
    int status;

    cdpAgent agent = cdp_system_agent(domain, tag);
    if (!agent)
        return CDP_STATUS_FAIL;

    status = agent(client, CDP_P(returned), self, CDP_ACTION_STORE_NEW, params, value);
    if (status != CDP_STATUS_PROGRESS)
        return status;

    cdp_store_add_agent(self->store, domain, tag, agent);
    return CDP_STATUS_SUCCESS;
}


static inline int cdp_instance_store_add_item(cdpRecord* client, cdpRecord** returned, cdpRecord* self, cdpRecord* child, cdpValue context) {
    assert(!cdp_record_is_floating(instance) && call && cdp_data_valid(data));
    cdp_call_clean(call);

    cdpRecord* target = cdp_link_pull(instance);

    if (!cdp_record_has_data(target)) {
        call->status  = CDP_STATUS_OK;
        call->log     = CDP_LOG_WARNING;
        call->message = "Target record has no data."
        return true;
    }

    self  = cdp_link_pull(self);
    child = cdp_link_pull(child);
    assert(cdp_record_is_dictionary(pipeline) && cdp_record_has_store(self) && !cdp_record_is_void(child));
    int status;

    cdpRecord* r = cdp_record_add(self, context, child);

    call->action = CDP_ACTION_STORE_ADD_ITEM;
    call->record = target;

    return cdp_instance_agent_call_store(target->instance, call);
}


static inline int cdp_instance_store_remove_item(cdpRecord* client, cdpRecord* self, cdpRecord* child) {
    assert(cdp_record_is_dictionary(pipeline) && !cdp_record_is_void(child));
    if (!self)
        self = cdp_record_parent(child);
    assert(cdp_record_has_store(self));
    int status;

    for (cdpAgentList* list = self->store->agent;  list;  list = list->next) {
        status = list->agent(client, NULL, self, CDP_ACTION_STORE_REMOVE_ITEM, child, CDP_V(0));
        if (status < CDP_STATUS_OK)
            return status;
    }

    cdp_record_remove(child, NULL);

    return CDP_STATUS_SUCCESS;
}


static inline int cdp_instance_store_delete(cdpRecord* client, cdpRecord* self) {
    self = cdp_link_pull(self);
    assert(cdp_record_is_dictionary(pipeline) && !cdp_record_is_void(self));
    int status;

    for (cdpAgentList* list = self->data->agent;  list;  list = list->next) {
        status = list->agent(client, NULL, self, CDP_ACTION_STORE_DELETE, NULL, CDP_V(0));
        if (status < CDP_STATUS_OK)
            return status;
    }

    cdp_record_delete_store(self);

    return CDP_STATUS_OK;
}


/* Agent: 'System Step'
 *
 * It generates an output each time the system is ready for another execution
 * step. Agents needing cooperative coroutine behaviour should connect to this.
 * If a base time is specified in the instance then System Step will sleep the
 * remaining time after completion (if any) to keep things in sync.
 *
 * Output:
 *      CDPID: a dynamically named event output.
 *
 * Config:
 *      'base-time':
 *
 */
struct _step {
    cdpRecord*  client;
    cdpValue    tic;
};

static inline bool agent_step_on_each_output(cdpEntry* entry, struct _step* step){
    cdp_instance_data_update(step->client, entry->record, sizeof(step->tic), sizeof(step->tic), step->tic);
    return false;
}

static int agent_system_step(cdpRecord* pipeline, cdpRecord* instance, cdpRecord* record, unsigned action) {
    assert(client && self);

    switch (action) {
      case CDP_ACTION_INSTANCE_NEW: {
        cdp_record_set_data(self, cdp_data_new_binary_uint64(0));
        cdp_record_set_store(self, cdp_store_new(CDP_ACRO("CDP"), CDP_WORD("list"), CDP_STORAGE_LINKED_LIST, CDP_INDEX_BY_INSERTION));
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_INSTANCE_CONNECT: {
        cdpRecord* link = cdp_record_append_link(self, CDP_AUTOID, record);
        CDP_PTR_SEC_SET(returned, link);
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_INSTANCE_UNPLUG: {
        assert(self == cdp_record_parent(record));
        cdp_record_remove(record, NULL);
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_DATA_UPDATE: {
        struct _step step = {.client = self, .tic = value};
        cdpEntry entry = {0};
        if (true == cdp_record_traverse(self, (cdpTraverse) agent_step_on_each_output, &step, &entry))
            return CDP_STATUS_FAIL;
        return CDP_STATUS_SUCCESS;
      }
    }

    return CDP_STATUS_OK;
}






