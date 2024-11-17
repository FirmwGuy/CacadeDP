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
#include "cdp_task.h"
#include <ctype.h>        // isupper()




extern cdpRecord CDP_ROOT;

cdpRecord* SYSTEM;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* DOMAIN;
cdpRecord* INTERNED;

cdpRecord* AGENCY;
cdpRecord* CASCADE;

cdpRecord* CDP_VOID;


static void system_initiate(void);



cdpTag cdp_system_domain_add(const char* text, unsigned basez) {

}




/* System related routines */

cdpRecord* cdp_system_add_agent(cdpTag domain, cdpTag agency, cdpTag tag, cdpAgent agent) {
    assert(cdp_domain_valid(domain) && agent);
    if (!SYSTEM)
        system_initiate();

    cdpRecord* perdomain = cdp_record_find_by_name(DOMAIN, cdp_id_to_property(domain));  // ToDo: use tags for domains also (instead of enumeration).
    if CDP_UNLIKELY(!perdomain) {
        perdomain = cdp_record_add_dictionary(DOMAIN, cdp_id_to_property(domain), CDP_STORAGE_ARRAY, 2);
    }

    cdpRecord* agentlist;
    cdpRecord* agencydic = cdp_record_find_by_name(perdomain, cdp_id_to_tag(CDP_TAG_AGENCY));
    if CDP_UNLIKELY(!agencydic) {
        agencydic = cdp_record_add_dictionary(perdomain, cdp_id_to_tag(CDP_TAG_AGENCY), CDP_STORAGE_RED_BLACK_T, 0);
        agentlist = NULL;
    } else {
        agentlist = cdp_record_find_by_name(agencydic, cdp_id_to_tag(agency));
    }
    if (!agentlst) {
        agentlist = cdp_record_add_dictionary(agencydic, cdp_id_to_tag(agency), CDP_STORAGE_RED_BLACK_T, 0);
    }

    return cdp_record_add_agent(agentlist, cdp_id_to_tag(tag), agent);
}


static void system_initiate(void) {
    cdp_record_system_initiate();

    /* Initiate root book structure.
    */
    SYSTEM  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_SYSTEM,  CDP_TAG_DICTIONARY, CDP_STORAGE_ARRAY, 4);
    USER    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_USER,    CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    PUBLIC  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_PUBLIC,  CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    DATA    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_DATA,    CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    NETWORK = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_NETWORK, CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    TEMP    = cdp_book_add_book(&CDP_ROOT, CDP_NAME_TEMP, CDP_TAG_BOOK, CDP_STORAGE_RED_BLACK_T);

    TAG     = cdp_book_add_book(&SYSTEM, CDP_NAME_TAG, CDP_TAG_STACK, CDP_STORAGE_PACKED_QUEUE, CDP_TAG_COUNT);
    NAME    = cdp_book_add_book(&SYSTEM, CDP_NAME_NAME, CDP_TAG_STACK, CDP_STORAGE_PACKED_QUEUE, CDP_TAG_COUNT + CDP_NAME_SYSTEM_COUNT + CDP_TASK_COUNT + CDP_ACTION_COUNT);
    AGENCY  = cdp_book_add_dictionary(&SYSTEM, CDP_NAME_AGENCY, CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);
    CASCADE = cdp_book_add_dictionary(&SYSTEM, CDP_NAME_CASCADE, CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);

    /* Initiage tasks
    */
    cdp_system_initiate_tasks();

    /* Initiate global records.
    */
    {
        CDP_VOID = cdp_book_add_bool(TEMP, CDP_TAG_VOID, 0);
        CDP_VOID->metadata.tag = CDP_TAG_VOID;
        CDP_VOID->metadata.id = CDP_TAG_VOID;
        CDP_RECORD_SET_ATTRIB(CDP_VOID, CDP_ATTRIB_FACTUAL);
    }
}


bool cdp_system_startup(void) {
    assert(SYSTEM);
    // ToDo: Traverse all records. On each record, call the "startup" agency.
    return true;
}


bool cdp_system_step(void) {
    assert(SYSTEM);
    // ToDo: traverse all agents. On each agent, do jobs listed on "work", then move them to "done".
    return true;
}


void cdp_system_shutdown(void) {
    assert(SYSTEM);

    // ToDo: Traverse all records. On each record, call the "shutdown" agency.

    cdp_system_finalize_tasks();
    cdp_record_branch_reset(&CDP_ROOT);
    cdp_record_system_shutdown();

    SYSTEM = NULL;
}




/*
 *    Agent related routines
 */

cdpRecord* cdp_task_begin(  cdpTask* cTask, cdpRecord* agency, cdpTag cast, cdpRecord* instance,
                            cdpRecord* parentTask, cdpRecord* baby,
                            int numInput, int numOutput ) {
    assert(cTask && cdp_record_is_dictionary(agency) && !cdp_record_is_void(instance

    //CDP_0(cTask);

    // Find instance tag to define agent
    cdpTag tag = cdp_record_tag(instance);    // ToDo: traverse all multiple tags on books.
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

    cTask->task = cdp_book_add_dictionary(call, CDP_AUTOID, CDP_TAG_DICTIONARY, CDP_STORAGE_ARRAY, 6);
    cdp_book_add_link(cTask->task, CDP_NAME_PARENT, parentTask);
    cdp_book_add_link(cTask->task, CDP_NAME_INSTANCE, instance);

    if (baby)
        cdp_book_add_link(task, CDP_NAME_BABY, baby);

    if (0 <= numInput)
        cTask->input = cdp_book_add_dictionary(cTask->task, CDP_NAME_INPUT, CDP_TAG_DICTIONARY, ((0 == numInput)? CDP_STORAGE_RED_BLACK_T: CDP_STORAGE_ARRAY), numInput);
    else
        cTask->input = NULL;

    if (0 <= numOutput)
        cdp_book_add_dictionary(cTask->task, CDP_NAME_OUTPUT, CDP_TAG_DICTIONARY, ((0 == numOutput)? CDP_STORAGE_RED_BLACK_T: CDP_STORAGE_ARRAY), numOutput);

    cdp_book_add_dictionary(cTask->task, CDP_NAME_STATUS, CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);

    return cTask->task;
}


cdpRecord* cdp_task_commit(cdpTask* cTask) {
    assert(cTask && cdp_record_is_dictionary(cTask->task));
    cdpRecord* work = cdp_record_find_by_name(cTask->agTag, CDP_NAME_WORK);
    return cdp_book_move_to(work, CDP_AUTOID, cTask->task);
}




