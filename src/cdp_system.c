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

cdpRecord* TAG;
cdpRecord* NAME;
cdpRecord* AGENCY;
cdpRecord* CASCADE;

cdpRecord* CDP_VOID;


static void system_initiate(void);




/*
 *   String interning routines
 */


struct NID {const char* name; size_t length; cdpID id;};


static bool name_id_traverse_find_text(cdpBookEntry* entry, struct NID* nid) {
    const char* name = cdp_register_read_utf8(entry->record);
    if (cdp_register_size(entry->record) == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = cdp_record_get_id(entry->record);
        return false;
    }
    return true;
}


cdpID cdp_name_id_add(const char* name, bool tag, bool borrow) {
    assert(name && *name && (tag? cdp_book_get_auto_id(TAG) <= CDP_TAG_MAXVAL: true));
    if (!SYSTEM)  system_initiate();

    size_t length = 0;
    for (const char* c=name; *c; c++, length++) {
        if (isupper(*c)) {
            assert(!isupper(*c));   // ToDo: make lowercase.
            return CDP_TAG_VOID;
    }   }

    // Find previous
    struct NID nid = {name, length};
    if (!cdp_book_traverse(TAG, (cdpTraverse)name_id_traverse_find_text, &nid, NULL))
        return nid.id;

    if (!cdp_book_traverse(NAME, (cdpTraverse)name_id_traverse_find_text, &nid, NULL)) {
        if (tag) {
            assert(!tag);   // ToDo: Tag is already registered as a name id, should we replace it with a link?
            return CDP_TAG_VOID;
        }
        return nid.id;
    }

    // Add new
    cdpRecord* reg = cdp_book_add_text((tag? TAG: NAME), (borrow? CDP_ATTRIB_FACTUAL: 0), CDP_AUTO_ID, borrow, name);
    return CDP_POS2NAMEID(cdp_record_get_id(reg));
}


cdpRecord* cdp_name_id_text(cdpID nameID) {
    cdpID id = CDP_NAMEID2POS(nameID);
    if (id > CDP_TAG_MAXVAL) {
        assert(id < cdp_book_get_auto_id(NAME));
        return cdp_book_find_by_position(NAME, id - CDP_TAG_MAXVAL);  // Find by position index (instead of by its own id).
    }
    assert(id < cdp_book_get_auto_id(TAG);
    return cdp_book_find_by_position(TAG, id);     // FixMe: check if entry is disabled.
}


static inline cdp_name_id_valid(name)   {return  name != CDP_TAG_VOID  &&  CDP_NAMEID2POS(name) < cdp_book_get_auto_id(NAME);}
static inline cdp_tag_id_valid(tag)     {return   tag != CDP_TAG_VOID  &&  CDP_NAMEID2POS(tag)  < cdp_book_get_auto_id(TAG);}




/*
 *    Agent related routines
 */


cdpRecord* cdp_agency(cdpID name) {
    assert(SYSTEM && cdp_name_id_valid(name));
    return cdp_book_find_by_name(AGENCY, name);
}


bool cdp_agency_set_agent(cdpRecord* agency, cdpTag tag, cdpAgent agent) {
    assert(cdp_record_is_dictionary(agency) && cdp_tag_id_valid(tag) && agent);

    if (cdp_book_find_by_name(agency, tag)) {
        assert(!tag);   // Tag shouldn't exist!
        return false;
    }

    cdpRecord* agTag = cdp_book_add_dictionary(agency, tag, CDP_TAG_DICTIONARY, CDP_STORAGE_ARRAY, 4);
    cdp_book_add_agent(agTag, CDP_NAME_AGENT, agent);
    cdp_book_add_book(agTag, CDP_NAME_CALL, CDP_TAG_BOOK, CDP_STORAGE_LINKED_LIST);
    cdp_book_add_book(agTag, CDP_NAME_DONE, CDP_TAG_BOOK, CDP_STORAGE_LINKED_LIST);
    cdp_book_add_book(agTag, CDP_NAME_WORK, CDP_TAG_BOOK, CDP_STORAGE_LINKED_LIST);

    return true;
}


cdpRecord* cdp_task_begin(  cdpTask* cTask, cdpRecord* agency, cdpTag cast, cdpRecord* instance,
                            cdpRecord* parentTask, cdpRecord* baby,
                            int numInput, int numOutput ) {
    assert(cTask && cdp_record_is_dictionary(agency) && !cdp_record_is_void(instance

    //CDP_0(cTask);

    // Find instance tag to define agent
    cdpTag tag = cdp_record_tag(instance);    // ToDo: traverse all multiple tags on books.
    cTask->agTag = cdp_book_find_by_name(agency, tag);
    if (!cTask->agTag) {
        // If tag is not found, use tag being cast.
        if (cast != CDP_TAG_VOID)
            cTask->agTag = cdp_book_find_by_name(agency, cast);
        if (!cTask->agTag) {
            assert(cTask->agTag);       // No suitable agent was ever found.
            return NULL;
        }
    }
    cdpRecord* call = cdp_book_find_by_name(cTask->agTag, CDP_NAME_CALL);

    // ToDo: check in the "done" book to find recyclable entries.

    cTask->task = cdp_book_add_dictionary(call, CDP_AUTO_ID, CDP_TAG_DICTIONARY, CDP_STORAGE_ARRAY, 6);
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
    cdpRecord* work = cdp_book_find_by_name(cTask->agTag, CDP_NAME_WORK);
    return cdp_book_move_to(work, CDP_AUTO_ID, cTask->task);
}




/* System related routines */

cdpRecord* cdp_system_agency_add(cdpID name, cdpTag tag, cdpAgent agent) {
    if (!SYSTEM)  system_initiate();
    assert(cdp_name_id_valid(name));

    // Find previous
    cdpRecord* agency = cdp_book_find_by_name(AGENCY, name);
    if (!agency)
        agency = cdp_book_add_dictionary(AGENCY, name, CDP_TAG_DICTIONARY, CDP_STORAGE_RED_BLACK_T);

    cdp_agency_set_agent(agency, tag, agent);

    return agency;
}


static void system_initiate_tags(void) {

    /**** WARNING: this must be done in the same order as the _cdpTagID enumeration in "cdp_record.h". ****/

    // Core tags
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "void");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "record");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "book");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "register");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "link");

    // Book tags
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "dictionary");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "list");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "queue");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "stack");

    // Register tags
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "byte");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "uint16");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "uint32");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "uint64");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "int16");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "int32");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "int64");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "float32");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "float64");

    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "tag");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "id");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "utf8");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "patch");

    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "boolean");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "interned");
    cdp_book_add_static_text(TAG, CDP_AUTO_ID, "agent");

    // Final check
    assert(cdp_book_children(TAG) == CDP_TAG_COUNT  &&  cdp_book_get_auto_id(TAG) == CDP_TAG_COUNT);
}


static void system_initiate_names(void) {
    cdp_book_set_auto_id(NAME, CDP_NPOS_MINVAL);

    /**** WARNING: this must be done in the same order as the _cdpNameID enumeration in "cdp_record.h" and "cdp_agent.h". ****/

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "/");   // The root book.

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "system");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "user");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "public");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "data");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "network");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "temp");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "name");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "agency");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "cascade");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "private");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "call");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "done");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "work");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "parent");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "baby");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,"instance");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "size");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "input");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "output");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "status");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "debug");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "warning");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "error");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "fatal");

    assert(cdp_book_get_auto_id(NAME) == CDP_NAME_ID_SYSTEM_COUNT);
}


_Static_assert( (CDP_NAME_ID_ACTION_COUNT - CDP_NAME_ID_INITIAL_COUNT) == (CDP_NAME_SYSTEM_COUNT + CDP_TASK_COUNT + CDP_ACTION_COUNT));


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

    /* Initiate tags, names, task names and agent fields (in that order).
    */
    system_initiate_tags();
    system_initiate_names();
    cdp_system_initiate_task_names();
    cdp_system_initiate_agent_fields();

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
    cdp_book_reset(&CDP_ROOT);
    cdp_record_system_shutdown();

    SYSTEM = NULL;
}

