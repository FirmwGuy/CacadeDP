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
//#include "cdp_task.h"




extern cdpRecord CDP_ROOT;

cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* DOMAIN;
cdpRecord* CASCADE;
cdpRecord* LIBRARY;

cdpRecord* CDP_VOID;


static void system_initiate(void);




/*
 *    Agent related routines
 */

cdpRecord* cdp_task_set_agent(cdpID domain, cdpID agency, cdpID tag, cdpAgent agent) {
    assert(cdp_id_text_valid(domain) && cdp_id_text_valid(agency) && cdp_id_text_valid(tag) && agent);
    if (!CASCADE)
        system_initiate();

    cdpRecord* d = cdp_record_find_by_name(DOMAIN, domain);
    if CDP_RARELY(!d) {
        d = cdp_dict_add_dictionary(DOMAIN, domain, CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    }

    cdpRecord* a = cdp_record_find_by_name(d, agency);
    if CDP_RARELY(!a) {
        a = cdp_dict_add_dictionary(d, agency, CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    }

    cdpRecord* t = cdp_record_find_by_name(a, tag);
    if CDP_EXPECT(!t) {
        t = cdp_dict_add_agent(a, tag, agent);
    } else {
        // ToDo: what if already set?
        assert(t);
    }

    return t;
}


cdpRecord* cdp_task_begin(  cdpTask* cTask, cdpRecord* agency, cdpID cast, cdpRecord* instance,
                            cdpRecord* parentTask, cdpRecord* baby,
                            int numInput, int numOutput ) {
    assert(cTask && cdp_record_is_dictionary(agency) && !cdp_record_is_void(instance

    //CDP_0(cTask);

    // Find instance tag to define agent
    cdpID tag = cdp_record_tag(instance);    // ToDo: traverse all multiple tags on books.
    cTask->agTag = cdp_record_find_by_name(agency, tag);
    if (!cTask->agTag) {
        // If tag is not found, use tag being cast.
        if (cast != CDP_TAG_VOID)
            cTask->agTag = cdp_record_find_by_name(agency, cast);
        if (!cTask->agTag) {
            assert(cTask->agTag);       // No suitable agent was ever found.
            return NULL;
        }
    }
    cdpRecord* call = cdp_record_find_by_name(cTask->agTag, CDP_NAME_CALL);

    // ToDo: check in the "done" book to find recyclable entries.

    cTask->task = cdp_record_add_dictionary(call, CDP_AUTOID, CDP_TAG_DICTIONARY, CDP_STORAGE_ARRAY, 6);
    cdp_book_add_link(cTask->task, CDP_NAME_PARENT, parentTask);
    cdp_book_add_link(cTask->task, CDP_NAME_INSTANCE, instance);

    if (baby)
        cdp_book_add_link(task, CDP_NAME_BABY, baby);

    if (0 <= numInput)
        cTask->input = cdp_record_add_dictionary(cTask->task, CDP_NAME_INPUT, CDP_TAG_DICTIONARY, ((0 == numInput)? CDP_STORAGE_RED_BLACK_T: CDP_STORAGE_ARRAY), numInput);
    else
        cTask->input = NULL;

    if (0 <= numOutput)
        cdp_record_add_dictionary(cTask->task, CDP_NAME_OUTPUT, CDP_TAG_DICTIONARY, ((0 == numOutput)? CDP_STORAGE_RED_BLACK_T: CDP_STORAGE_ARRAY), numOutput);

    cdp_record_add_dictionary(cTask->task, CDP_NAME_STATUS, CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);

    return cTask->task;
}


cdpRecord* cdp_task_commit(cdpTask* cTask) {
    assert(cTask && cdp_record_is_dictionary(cTask->task));
    cdpRecord* work = cdp_record_find_by_name(cTask->agTag, CDP_NAME_WORK);
    return cdp_book_move_to(work, CDP_AUTOID, cTask->task);
}




/* System related routines */



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
    DOMAIN  = cdp_dict_add_dictionary(system, CDP_WORD_AGENCY,  CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    CASCADE = cdp_dict_add_dictionary(system, CDP_WORD_CASCADE, CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    //LIBRARY = cdp_dict_add_dictionary(system, CDP_WORD_LIBRARY, CDP_ACRON_CDP, CDP_WORD_DICTIONARY, CDP_STORAGE_RED_BLACK_T);

    /* Initiage tasks
    */
    //cdp_system_initiate_tasks();

    /* Initiate global records.
    */
    CDP_VOID = cdp_record_append_value(TEMP, CDP_WORD_VOID, CDP_ACRON_CDP, CDP_WORD_VOID, 0, 0, sizeof(bool), sizeof(bool));
    CDP_VOID->data->writable = false;
}


bool cdp_system_startup(void) {
    assert(CASCADE);
    // ToDo: Traverse all records. On each record, call the "startup" agency.
    return true;
}


bool cdp_system_step(void) {
    assert(CASCADE);
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

