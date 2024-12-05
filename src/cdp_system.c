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


#include "cdp_system.h"


extern cdpRecord CDP_ROOT;

cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* DOMAIN;
cdpRecord* CASCADE;
cdpRecord* LIBRARY;

cdpRecord* CDP_STEP;
cdpRecord* CDP_VOID;

cdpAgentList* AGENT;


struct _step {
    cdpRecord*  client;
    cdpValue    tic;
};

static inline bool agent_step_on_each_output(cdpEntry* entry, struct _step* step){
    cdp_cascade_data_update(step->client, entry->record, sizeof(step->tic), sizeof(step->tic), step->tic);
    return false;
}

static int agent_system_step(cdpRecord* client, void** returned, cdpRecord* self, unsigned action, cdpRecord* record, cdpValue value) {
    assert(client && self);

    switch (action) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(self, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("UINT64"), (cdpID)0, sizeof(uint64_t), 0));
        CDP_PTR_SEC_SET(returned, self->data);
        return CDP_STATUS_PROGRESS;
      }
      case CDP_ACTION_STORE_NEW: {
        cdp_record_set_store(self, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_LINKED_LIST, CDP_INDEX_BY_INSERTION));
        CDP_PTR_SEC_SET(returned, self->store);
        return CDP_STATUS_PROGRESS;
      }

      case CDP_ACTION_CONNECT: {
        cdpRecord* link = cdp_record_append_link(self, CDP_AUTOID, record);
        CDP_PTR_SEC_SET(returned, link);
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_UNPLUG: {
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




static void system_initiate(void) {
    cdp_record_system_initiate();

    // Initiate root structure
    cdpRecord* system  = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD_SYSTEM,  CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_ARRAY, 4);

    USER    = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD_USER,    CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    PUBLIC  = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD_PUBLIC,  CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    DATA    = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD_DATA,    CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    NETWORK = cdp_dict_add_dictionary(&CDP_ROOT, CDP_WORD_NETWORK, CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);

    TEMP    = cdp_dict_add_list(&CDP_ROOT, CDP_WORD_TEMP,    CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_LINKED_LIST);

    // Initiate system structure
    //DOMAIN  = cdp_dict_add_dictionary(system, CDP_WORD_AGENCY,  CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    CASCADE = cdp_dict_add_dictionary(system, CDP_WORD_CASCADE, CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    //LIBRARY = cdp_dict_add_dictionary(system, CDP_WORD_LIBRARY, CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);

    // Add system agents
    cdp_system_register_agent(CDP_ACRON_CDP, CDP_WORD_STEP, agent_system_step);

    // Initiate global records.
    cdpRecord step = {0};
    cdp_cascade_record_new(cdp_root(), &step, CDP_WORD_STEP, CDP_ACRON_CDP, CDP_WORD_STEP, NULL, CDP_V(0), NULL, CDP_V(0));
    CDP_STEP = cdp_dict_add(CASCADE, &step);

    //CDP_VOID = cdp_record_append_value(TEMP, CDP_WORD_VOID, CDP_ACRON_CDP, CDP_WORD_VOID, 0, 0, sizeof(bool), sizeof(bool));
    //CDP_VOID->data->writable = false;
}


void cdp_system_register_agent(cdpID domain, cdpID tag, cdpAgent agent) {
    if (!CASCADE)
        system_initiate();

    cdpAgentList* list;

    for (list = AGENT;  list;  list = list->next) {
        if (list->domain == domain  &&  list->tag == tag) {
            assert(!list);
            return;
        }
    }

    list = cdp_agent_list_new(domain, tag, agent);
    list->next = AGENT;
    AGENT = list;
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
    assert(CASCADE);
    // ToDo: Traverse all records. On each record, call the "startup" agency.
    return true;
}


bool cdp_system_step(void) {
    assert(CASCADE);

    static uint64_t tic;

    if CDP_RARELY(!cdp_cascade_data_update(cdp_root(), CDP_STEP, sizeof(uint64_t), sizeof(uint64_t), CDP_V(tic++)))
        return false;

    // ToDo: traverse all agents. On each agent, do jobs listed on "work", then move them to "done".
    return true;
}


void cdp_system_shutdown(void) {
    assert(CASCADE);

    // ToDo: Traverse all records. On each record, call the "shutdown" agency.

    //cdp_system_finalize_tasks();
    cdp_record_delete_children(&CDP_ROOT);
    cdp_record_system_shutdown();

    CASCADE = NULL;
}



