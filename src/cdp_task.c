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


#include "cdp_task.h"
#include "cdp_agent.h"




void cdp_system_initiate_task_names(void) {
    extern cdpRecord* NAME;

    /**** WARNING: this must be done in the same order as the _cdpTaskID enumeration in "cdp_task.h". ****/

    // System tasks
    cdp_book_add_static_text(NAME, CDP_AUTOID,     "startup");
    cdp_book_add_static_text(NAME, CDP_AUTOID,    "shutdown");

    // Record tasks
    cdp_book_add_static_text(NAME, CDP_AUTOID,    "initiate");
    cdp_book_add_static_text(NAME, CDP_AUTOID,   "terminate");
    cdp_book_add_static_text(NAME, CDP_AUTOID,       "reset");
    cdp_book_add_static_text(NAME, CDP_AUTOID,        "next");
    cdp_book_add_static_text(NAME, CDP_AUTOID,    "previous");
    cdp_book_add_static_text(NAME, CDP_AUTOID,    "validate");
    cdp_book_add_static_text(NAME, CDP_AUTOID,      "remove");

    // Book tasks
    cdp_book_add_static_text(NAME, CDP_AUTOID,         "add");
    cdp_book_add_static_text(NAME, CDP_AUTOID,     "prepend");
    cdp_book_add_static_text(NAME, CDP_AUTOID,      "insert");
    cdp_book_add_static_text(NAME, CDP_AUTOID,       "first");
    cdp_book_add_static_text(NAME, CDP_AUTOID,        "last");
    cdp_book_add_static_text(NAME, CDP_AUTOID,        "take");
    cdp_book_add_static_text(NAME, CDP_AUTOID,         "pop");
    cdp_book_add_static_text(NAME, CDP_AUTOID,      "search");
    cdp_book_add_static_text(NAME, CDP_AUTOID,        "link");
    cdp_book_add_static_text(NAME, CDP_AUTOID,      "shadow");
    cdp_book_add_static_text(NAME, CDP_AUTOID,       "clone");
    cdp_book_add_static_text(NAME, CDP_AUTOID,        "move");

    // Register tasks
    cdp_book_add_static_text(NAME, CDP_AUTOID,   "reference");
    cdp_book_add_static_text(NAME, CDP_AUTOID, "unreference");
    cdp_book_add_static_text(NAME, CDP_AUTOID,   "serialize");
    cdp_book_add_static_text(NAME, CDP_AUTOID, "unserialize");
    cdp_book_add_static_text(NAME, CDP_AUTOID,  "textualize");
    cdp_book_add_static_text(NAME, CDP_AUTOID,"untextualize");
    cdp_book_add_static_text(NAME, CDP_AUTOID,        "read");
    cdp_book_add_static_text(NAME, CDP_AUTOID,      "update");
    cdp_book_add_static_text(NAME, CDP_AUTOID,       "patch");
}



void cdp_system_initiate_tasks(void) {
    cdp_system_initiate_task_names();


    // Record tasks

    cdpRecord* agency = cdp_system_agency_add(CDP_NAME_INITIATE, CDP_TAG_BOOK, cdp_agent_initiate_book);
    cdp_agency_set_agent(agency, CDP_TAG_REGISTER, cdp_agent_initiate_register);
    cdp_agency_set_agent(agency, CDP_TAG_LINK,     cdp_agent_initiate_link);

    cdp_system_agency_add(CDP_NAME_TERMINATE, CDP_TAG_RECORD, cdp_agent_terminate);

    agency = cdp_system_agency_add(CDP_NAME_RESET, CDP_TAG_BOOK, cdp_agent_reset_book);
    cdp_agency_set_agent(agency, CDP_TAG_REGISTER, cdp_agent_reset_register);

    cdp_system_agency_add(CDP_NAME_NEXT,     CDP_TAG_RECORD, cdp_agent_next);
    cdp_system_agency_add(CDP_NAME_PREVIOUS, CDP_TAG_RECORD, cdp_agent_previous);
    cdp_system_agency_add(CDP_NAME_VALIDATE, CDP_TAG_RECORD, cdp_agent_validate);

    agency = cdp_system_agency_add(CDP_NAME_REMOVE, CDP_TAG_RECORD, cdp_agent_remove);
    cdp_agency_set_agent(agency, CDP_TAG_LIST,  cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_QUEUE, cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_STACK, cdp_agent_ignore);


    // Book tasks

    agency = cdp_system_agency_add(CDP_NAME_ADD, CDP_TAG_BOOK, cdp_agent_add);
    cdp_agency_set_agent(agency, CDP_TAG_STACK, cdp_agent_ignore);

    agency = cdp_system_agency_add(CDP_NAME_PREPEND, CDP_TAG_BOOK, cdp_agent_prepend);
    cdp_agency_set_agent(agency, CDP_TAG_DICTIONARY, cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_QUEUE,      cdp_agent_ignore);

    agency = cdp_system_agency_add(CDP_NAME_INSERT, CDP_TAG_BOOK, cdp_agent_insert);
    cdp_agency_set_agent(agency, CDP_TAG_DICTIONARY, cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_LIST,       cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_QUEUE,      cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_STACK,      cdp_agent_ignore);

    cdp_system_agency_add(CDP_NAME_FIRST, CDP_TAG_BOOK, cdp_agent_first);
    cdp_system_agency_add(CDP_NAME_LAST,  CDP_TAG_BOOK, cdp_agent_last);

    agency = cdp_system_agency_add(CDP_NAME_TAKE, CDP_TAG_BOOK, cdp_agent_take);
    cdp_agency_set_agent(agency, CDP_TAG_QUEUE, cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_STACK, cdp_agent_ignore);

    cdp_system_agency_add(CDP_NAME_POP,    CDP_TAG_BOOK, cdp_agent_pop);
    cdp_system_agency_add(CDP_NAME_SEARCH, CDP_TAG_BOOK, cdp_agent_search);
    cdp_system_agency_add(CDP_NAME_LINK,   CDP_TAG_BOOK, cdp_agent_link);
    cdp_system_agency_add(CDP_NAME_SHADOW, CDP_TAG_BOOK, cdp_agent_shadow);
    cdp_system_agency_add(CDP_NAME_CLONE,  CDP_TAG_BOOK, cdp_agent_clone);

    agency = cdp_system_agency_add(CDP_NAME_MOVE, CDP_TAG_BOOK, cdp_agent_move);
    cdp_agency_set_agent(agency, CDP_TAG_LIST,  cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_QUEUE, cdp_agent_ignore);
    cdp_agency_set_agent(agency, CDP_TAG_STACK, cdp_agent_ignore);


    // Register taks

    cdp_system_agency_add(CDP_NAME_REFERENCE,   CDP_TAG_REGISTER, cdp_agent_reference);
    cdp_system_agency_add(CDP_NAME_UNREFERENCE, CDP_TAG_REGISTER, cdp_agent_unreference);
    cdp_system_agency_add(CDP_NAME_SERIALIZE,   CDP_TAG_REGISTER, cdp_agent_serialize);
    cdp_system_agency_add(CDP_NAME_UNSERIALIZE, CDP_TAG_REGISTER, cdp_agent_unserialize);

    cdpRecord* agency2 = cdp_system_agency_add(CDP_NAME_TEXTUALIZE,   CDP_TAG_REGISTER, cdp_agent_textualize);
    agency             = cdp_system_agency_add(CDP_NAME_UNTEXTUALIZE, CDP_TAG_REGISTER, cdp_agent_untextualize);

    #define set_agent_and_textualization(agent, tag)                   \
        cdp_agency_set_agent(agency2, CDP_TAG_##tag, cdp_agent_textualize_##agent);\
        cdp_agency_set_agent(agency,  CDP_TAG_##tag, cdp_agent_untextualize_##agent)

    set_agent_and_textualization(byte,    BYTE);
    set_agent_and_textualization(uint16,  UINT16);
    set_agent_and_textualization(uint32,  UINT32);
    set_agent_and_textualization(uint64,  UINT64);
    set_agent_and_textualization(int16,   INT16);
    set_agent_and_textualization(int32,   INT32);
    set_agent_and_textualization(int64,   INT64);
    set_agent_and_textualization(float32, FLOAT32);
    set_agent_and_textualization(float64, FLOAT64);

    cdp_system_agency_add(CDP_NAME_READ,   CDP_TAG_REGISTER, cdp_agent_read);
    cdp_system_agency_add(CDP_NAME_UPDATE, CDP_TAG_REGISTER, cdp_agent_update);
    cdp_system_agency_add(CDP_NAME_PATCH,  CDP_TAG_REGISTER, cdp_agent_patch);


    // Finish agents
    assert(cdp_record_children(AGENCY) == CDP_TASK_COUNT);
}

void cdp_system_finalize_tasks(void) {
    ;
}






#define signaler_start(name, signal, inArgs, outArgs)                  \
    assert(instance);                                                  \
    if (!signal)                                                       \
        signal = cdp_task_new(name, inArgs, outArgs)

#define signaler_action(signal, result, get_res, ...)                          \
    if (cdp_system_does_action(instance, signal)) {                            \
        __VA_ARGS__;                                                           \
        result = get_res;                                                      \
    } else {                                                                   \
        /* ToDo: report error. */                                              \
        assert(cdp_record_children(&signal->condition));                        \
        result = 0;                                                            \
    }                                                                          \
    cdp_task_reset(signal)

#define signaler_return(type, signal, read_op)                                 \
    type result;                                                               \
    signaler_action(  signal,                                                  \
                      result,                                                  \
                      read_op(o),                                              \
                      cdpRecord* o = cdp_record_find_by_name( &signal->output,   \
                                                            CDP_NAME_OUTPUT ));\
    return result




/*
 *    Record signal API
 */


bool cdp_initiate_book(cdpRecord* instance, cdpID nameID, cdpID agentID, unsigned storage, unsigned baseLength) {
    assert(!cdp_id_is_void(nameID)  &&  agentID  &&  storage < CDP_STORAGE_COUNT);
    signaler_start(CDP_NAME_INITIATE, SIGNAL_INITIATE_BOOK, 4, 0);

    cdp_book_add_id(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_NAME, nameID);
    cdp_book_add_id(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_AGENT, agentID);
    cdp_book_add_id(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_STORAGE, storage);
    if (baseLength)
        cdp_book_add_uint32(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_BASE, baseLength);

    bool result;
    signaler_action(SIGNAL_INITIATE_BOOK, result, true);

    return result;
}


bool cdp_initiate_register(cdpRecord* instance, cdpID nameID, cdpID agentID, bool borrow, void* data, size_t size) {
    assert(!cdp_id_is_void(nameID) && agentID && size);
    signaler_start(CDP_NAME_INITIATE, SIGNAL_INITIATE_REGISTER, 3, 0);

    cdp_book_add_id(&SIGNAL_INITIATE_REGISTER->input, CDP_NAME_NAME, nameID);
    cdp_book_add_id(&SIGNAL_INITIATE_REGISTER->input, CDP_NAME_AGENT, agentID);
    cdp_record_add_value(&SIGNAL_INITIATE_REGISTER->input, 0, CDP_NAME_DATA, CDP_TAG_REGISTER, borrow, data, size);

    bool result;
    signaler_action(SIGNAL_INITIATE_REGISTER, result, true);

    return result;
}


bool cdp_initiate_link(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    assert(!cdp_id_is_void(nameID) && !cdp_record_is_void(record));
    CDP_LINK_RESOLVE(record);
    signaler_start(CDP_NAME_INITIATE, SIGNAL_INITIATE_LINK, 3, 0);

    cdp_book_add_id(&SIGNAL_INITIATE_LINK->input, CDP_NAME_NAME, nameID);
    cdp_book_add_link(&SIGNAL_INITIATE_LINK->input, CDP_NAME_LINK, record);

    bool result;
    signaler_action(SIGNAL_INITIATE_LINK, result, true);

    return result;
}


bool cdp_initiate(cdpRecord* instance, cdpID nameID, cdpRecord* bookArgs) {
    assert(instance && !cdp_id_is_void(nameID));
    if (!SIGNAL_INITIATE) {
        SIGNAL_INITIATE = cdp_new(cdpTask);
        SIGNAL_INITIATE->nameID = nameID;
        cdp_record_initialize_dictionary(&SIGNAL_INITIATE->input, CDP_NAME_INPUT, 0, CDP_STORAGE_RED_BLACK_T);
    }

    cdp_book_add_id(&SIGNAL_INITIATE->input, CDP_NAME_NAME, nameID);

    if (bookArgs && cdp_record_children(bookArgs)) {
        cdp_record_add(&SIGNAL_INITIATE->input, bookArgs, false);
    }

    bool result;
    signaler_action(SIGNAL_INITIATE, result, true);
    return result;
}


void cdp_terminate(cdpRecord* instance) {
    if (!cdp_record_is_connected(instance)) {
        cdp_record_finalize(instance);
        return;
    }
    signaler_start(CDP_NAME_TERMINATE, SIGNAL_TERMINATE, 0, 0);
    cdp_system_does_action(instance, SIGNAL_TERMINATE);
}


void cdp_reset(cdpRecord* instance) {
    if (!cdp_record_is_connected(instance)) {
        if (cdp_record_is_link(instance))
            CDP_LINK_RESOLVE(instance);

        if (cdp_record_children(instance))
            cdp_record_branch_reset(instance);
        else if (cdp_record_has_data(instance))
            cdp_record_data_reset(instance);
        return;
    }
    signaler_start(CDP_NAME_TERMINATE, SIGNAL_TERMINATE, 0, 0);
    cdp_system_does_action(instance, SIGNAL_TERMINATE);
}


cdpRecord* cdp_next(cdpRecord* instance) {
    if (!cdp_record_is_connected(instance))
        return cdp_record_next(NULL, instance);

    signaler_start(CDP_NAME_NEXT, SIGNAL_NEXT, 0, 1);
    signaler_return(cdpRecord*, SIGNAL_NEXT, cdp_link_data);
}


cdpRecord* cdp_previous(cdpRecord* instance) {
    if (!cdp_record_is_connected(instance))
        return cdp_record_prev(NULL, instance);

    signaler_start(CDP_NAME_PREVIOUS, SIGNAL_PREVIOUS, 0, 1);
    signaler_return(cdpRecord*, SIGNAL_PREVIOUS, cdp_link_data);
}


bool cdp_validate(cdpRecord* instance) {
    CDP_LINK_RESOLVE(instance);
    signaler_start(CDP_NAME_VALIDATE, SIGNAL_VALIDATE, 0, 1);
    signaler_return(bool, SIGNAL_PREVIOUS, cdp_record_read_bool);
}


void cdp_remove(cdpRecord* instance, cdpRecord* target) {
    if (!cdp_record_is_connected(instance)) {
        cdp_record_remove(instance, target);
        return;
    }

    signaler_start(CDP_NAME_REMOVE, SIGNAL_REMOVE, 0, 1);

    cdp_system_does_action(instance, SIGNAL_REMOVE);

    cdpRecord* moved = cdp_record_find_by_name(&SIGNAL_REMOVE->output, CDP_NAME_OUTPUT);
    cdp_record_remove(moved, target);

    cdp_task_reset(SIGNAL_REMOVE);
}




/*
 *    Book signal API
 */


cdpRecord* cdp_add(cdpRecord* instance, cdpRecord* record) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance) && record);
    if (!cdp_record_is_connected(instance))
        return cdp_record_add(instance, record, false);

    signaler_start(CDP_NAME_ADD, SIGNAL_ADD, 1, 1);
    cdp_book_add_link(&SIGNAL_ADD->input, CDP_NAME_RECORD, record);

    cdp_system_does_action(instance, SIGNAL_ADD);
    signaler_return(cdpRecord*, SIGNAL_ADD, cdp_link_data);
}


cdpRecord* cdp_prepend(cdpRecord* instance, cdpRecord* record) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance) && record);
    if (!cdp_record_is_connected(instance))
        return cdp_record_add(instance, record, true);

    signaler_start(CDP_NAME_PREPEND, SIGNAL_PREPEND, 1, 1);
    cdp_book_add_link(&SIGNAL_PREPEND->input, CDP_NAME_RECORD, record);

    cdp_system_does_action(instance, SIGNAL_PREPEND);
    signaler_return(cdpRecord*, SIGNAL_PREPEND, cdp_link_data);
}


cdpRecord* cdp_insert(cdpRecord* instance, size_t position, cdpRecord* record) {
    return NULL;
}


cdpRecord* cdp_first(cdpRecord* instance) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance));
    if (!cdp_record_is_connected(instance))
        return cdp_record_first(instance);

    signaler_start(CDP_NAME_FIRST, SIGNAL_FIRST, 0, 1);
    cdp_system_does_action(instance, SIGNAL_FIRST);
    signaler_return(cdpRecord*, SIGNAL_FIRST, cdp_link_data);
}


cdpRecord* cdp_last(cdpRecord* instance) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance));
    if (!cdp_record_is_connected(instance))
        return cdp_record_last(instance);

    signaler_start(CDP_NAME_LAST, SIGNAL_LAST, 0, 1);
    cdp_system_does_action(instance, SIGNAL_LAST);
    signaler_return(cdpRecord*, SIGNAL_LAST, cdp_link_data);
}


bool cdp_take(cdpRecord* instance, cdpRecord* target) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance));
    if (!cdp_record_is_connected(instance))
        return cdp_record_child_take(instance, target);

    signaler_start(CDP_NAME_TAKE, SIGNAL_TAKE, 1, 1);
    if (target)
        cdp_book_add_link(&SIGNAL_TAKE->input, CDP_NAME_RECORD, target);

    cdp_system_does_action(instance, SIGNAL_TAKE);
    if (target) {
        cdpRecord* moved = cdp_record_find_by_name(&SIGNAL_TAKE->output, CDP_NAME_OUTPUT);
        cdp_record_remove(moved, target);
    }

    cdp_task_reset(SIGNAL_TAKE);
    return true;
}


bool cdp_pop(cdpRecord* instance, cdpRecord* target) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance));
    if (!cdp_record_is_connected(instance))
        return cdp_record_child_pop(instance, target);

    signaler_start(CDP_NAME_POP, SIGNAL_POP, 1, 1);
    if (target)
        cdp_book_add_link(&SIGNAL_POP->input, CDP_NAME_RECORD, target);

    cdp_system_does_action(instance, SIGNAL_POP);
    if (target) {
        cdpRecord* moved = cdp_record_find_by_name(&SIGNAL_POP->output, CDP_NAME_OUTPUT);
        cdp_record_remove(moved, target);
    }

    cdp_task_reset(SIGNAL_POP);
    return true;
}


cdpRecord* cdp_search(cdpRecord* instance, cdpRecord* key) {
    return NULL;
}


cdpRecord* cdp_link(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance) && !cdp_id_is_void(nameID) && !cdp_record_is_void(record));
    if (!cdp_record_is_connected(instance))
        return cdp_book_add_link(instance, nameID, record);

    signaler_start(CDP_NAME_LINK, SIGNAL_LINK, 2, 1);
    cdp_book_add_id(&SIGNAL_LINK->input, CDP_NAME_NAME, nameID);
    cdp_book_add_link(&SIGNAL_LINK->input, CDP_NAME_RECORD, record);

    signaler_return(cdpRecord*, SIGNAL_LINK, cdp_link_data);
}


cdpRecord* cdp_shadow(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance) && !cdp_id_is_void(nameID) && !cdp_record_is_void(record));
    if (!cdp_record_is_connected(instance))
        return cdp_book_add_shadow(instance, nameID, record);

    signaler_start(CDP_NAME_SHADOW, SIGNAL_SHADOW, 2, 1);
    cdp_book_add_id(&SIGNAL_SHADOW->input, CDP_NAME_NAME, nameID);
    cdp_book_add_link(&SIGNAL_SHADOW->input, CDP_NAME_RECORD, record);

    signaler_return(cdpRecord*, SIGNAL_SHADOW, cdp_link_data);
}


cdpRecord* cdp_clone(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance) && !cdp_id_is_void(nameID) && !cdp_record_is_void(record));
    if (!cdp_record_is_connected(instance))
        return cdp_book_add_clone(instance, nameID, record);

    signaler_start(CDP_NAME_CLONE, SIGNAL_CLONE, 2, 1);
    cdp_book_add_id(&SIGNAL_CLONE->input, CDP_NAME_NAME, nameID);
    cdp_book_add_link(&SIGNAL_CLONE->input, CDP_NAME_RECORD, record);

    signaler_return(cdpRecord*, SIGNAL_CLONE, cdp_link_data);
}


cdpRecord* cdp_move(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    CDP_LINK_RESOLVE(instance);
    assert(cdp_record_children(instance) && !cdp_id_is_void(nameID) && !cdp_record_is_void(record));
    if (!cdp_record_is_connected(instance))
        return cdp_book_move_to(instance, nameID, record);

    signaler_start(CDP_NAME_MOVE, SIGNAL_MOVE, 2, 1);
    cdp_book_add_id(&SIGNAL_MOVE->input, CDP_NAME_NAME, nameID);
    cdp_book_add_link(&SIGNAL_MOVE->input, CDP_NAME_RECORD, record);

    signaler_return(cdpRecord*, SIGNAL_MOVE, cdp_link_data);
}




/*
 *    Register signal API
 */


void cdp_reference(cdpRecord* instance) {
    CDP_LINK_RESOLVE(instance);
    if (!cdp_record_is_connected(instance))
        return;

    signaler_start(CDP_NAME_REFERENCE, SIGNAL_REFERENCE, 0, 0);
    cdp_system_does_action(instance, SIGNAL_REFERENCE);
}


void cdp_unreference(cdpRecord* instance) {
    CDP_LINK_RESOLVE(instance);
    if (!cdp_record_is_connected(instance))
        return;

    signaler_start(CDP_NAME_UNREFERENCE, SIGNAL_UNREFERENCE, 0, 0);
    cdp_system_does_action(instance, SIGNAL_UNREFERENCE);
}


size_t cdp_serialize(cdpRecord* instance, void* data, size_t size) {
    return 0;
}


bool cdp_unserialize(cdpRecord* instance, void* data, size_t size) {
    return true;
}


bool cdp_textualize(cdpRecord* instance, char** data, size_t* length) {
    return true;
}


bool cdp_untextualize(cdpRecord* instance, char* data, size_t length) {
    return true;
}


void* cdp_read(cdpRecord* instance, void* data, size_t* size) {
    CDP_LINK_RESOLVE(instance);
    if (!cdp_record_is_connected(instance))
        return cdp_record_read_value(instance, 0, data, size);

    signaler_start(CDP_NAME_READ, SIGNAL_READ, 1, 1);
    cdp_record_add_value(&SIGNAL_READ->input, 0, CDP_NAME_DATA, CDP_TAG_REGISTER, true, data, *size);

    cdp_system_does_action(instance, SIGNAL_READ);

    cdpRecord* reg = cdp_record_find_by_name(&SIGNAL_READ->output, CDP_NAME_OUTPUT);
    assert(cdp_record_has_data(reg));
    if (size)
        *size = cdp_register_size(reg);
    void* d = cdp_register_data(reg);

    cdp_task_reset(SIGNAL_READ);
    return d;
}


void cdp_update(cdpRecord* instance, void* data, size_t size) {
    assert(data && size);
    CDP_LINK_RESOLVE(instance);
    if (!cdp_record_is_connected(instance)) {
        cdp_register_update(instance, data, size);
        return;
    }

    signaler_start(CDP_NAME_UPDATE, SIGNAL_UPDATE, 1, 0);
    cdp_record_add_value(&SIGNAL_UPDATE->input, 0, CDP_NAME_DATA, CDP_TAG_REGISTER, true, data, size);

    cdp_system_does_action(instance, SIGNAL_UPDATE);

    cdp_task_reset(SIGNAL_UPDATE);
}


void* cdp_patch(cdpRecord* instance, void* data, size_t size) {
    return NULL;
}

