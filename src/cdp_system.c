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
#include "cdp_agent.h"
#include <ctype.h>        // isupper()




extern cdpRecord CDP_ROOT;

cdpRecord* SYSTEM;
cdpRecord* CASCADE;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* TAG;
cdpRecord* NAME;
cdpRecord* AGENCY;

cdpRecord* CDP_VOID;

cdpTask* SYSTEM_SIGNAL;
cdpTask* CONNECT_SIGNAL;


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
        return cdp_book_find_by_position(NAME, id - CDP_TAG_MAXVAL);  // Find by position index instead of its own id.
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


bool cdp_agency_add_agent(cdpRecord* agency, cdpTag tag, cdpAgent agent) {
    assert(cdp_record_is_book(agency) && cdp_tag_id_valid(tag) && agent);

    if (cdp_book_find_by_name(agency, tag))
        return false

    cdpRecord* atag = cdp_book_add_dictionary(agency, tag, CDP_TAG_DICTIONARY, CDP_STO_CHD_ARRAY, 4);
    cdp_book_add_agent(atag, CDP_NAME_AGENT, agent);
    cdp_book_add_book(atag, CDP_NAME_CALL, CDP_TAG_BOOK, CDP_STO_CHD_LINKED_LIST);
    cdp_book_add_book(atag, CDP_NAME_TASK, CDP_TAG_BOOK, CDP_STO_CHD_LINKED_LIST);
    cdp_book_add_book(atag, CDP_NAME_DONE, CDP_TAG_BOOK, CDP_STO_CHD_LINKED_LIST);

    return true;
}


cdpAgent cdp_agency_get_agent(cdpRecord* agency, cdpTag tag) {
    return cdp_book_find_by_name(agency, tag);
}


static bool cdp_agency_task_agent_internal(cdpRecord* instance, cdpTask* signal) {
    cdpID agentID = cdp_record_tag(instance);
    while (agentID) {
        cdpRecord* agent = cdp_system_get_agent(agentID);
        cdpAgent action = cdp_dict_get_agent(agent, signal->nameID);
        if (action)
            return action(instance, signal);

        //agentID = cdp_dict_get_id(agent, CDP_NAME_ASSIMILATE);
        cdpRecord* assimilate = cdp_book_first(agent);  // FixMe: multiple parents.
        if (!assimilate)    break;
        agentID = cdp_register_read_id(assimilate);
    }

    return cdp_agent_ignore(instance, signal);
}


bool cdp_agency_task_agent(cdpRecord* agency, cdpTask* parent, cdpRecord* instance, cdpTask* task) {
    assert(signal);
    for (   cdpRecord* book = instance;
            book && cdp_record_is_baby(book);
            book = cdp_record_parent(book)  ) {
        // FixMe: traverse parents in backward order.
        if (!cdp_system_does_action_internal(book, signal))
            return false;
    }
    return true;
}




cdpRecord* cdp_system_agency_add(cdpID name, cdpTag tag, cdpAgent agent) {
    if (!SYSTEM)  system_initiate();

    assert(cdp_name_id_valid(name));

    // Find previous
    cdpRecord* agency = cdp_book_find_by_name(AGENCY, name);
    if (!agency)
        agency = cdp_book_add_dictionary(AGENCY, name, CDP_TAG_DICTIONARY, CDP_STO_CHD_RED_BLACK_T);

    cdp_book_add_agent(agency, tag, agent);

    return cdp_agency_add_agent(agency, tag, agent);
}


bool cdp_system_connect(cdpRecord* instanceSrc, cdpID output, cdpRecord* recordTgt) {
    assert(SYSTEM);

    if (!CONNECT_SIGNAL)
        CONNECT_SIGNAL = cdp_task_new(CDP_NAME_CONNECT, 1, 0);

    cdp_book_add_link(&CONNECT_SIGNAL->input, output, recordTgt);

    bool done = cdp_system_does_action(instanceSrc, CONNECT_SIGNAL);

    CDP_RECORD_SET_ATTRIB(recordTgt, CDP_ATTRIB_CONNECTED);   // FixMe: Only outputs need to be marked as "connected".

    cdp_task_reset(CONNECT_SIGNAL);

    return done;
}


bool cdp_system_disconnect(cdpRecord* link) {
    return false;
}




static void system_initiate_tags(void) {
    TAG = cdp_book_add_book(&SYSTEM, CDP_NAME_TAG, CDP_TAG_STACK, CDP_STO_CHD_PACKED_QUEUE, CDP_TAG_COUNT);

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
    NAME = cdp_book_add_book(&SYSTEM, CDP_NAME_NAME, CDP_TAG_STACK, CDP_STO_CHD_PACKED_QUEUE, ...);
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
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "size");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "private");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "action");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "input");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "output");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "debug");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "warning");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "error");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "fatal");

    cdp_system_initiate_tasks();
    cdp_system_initiate_actions();

    assert(cdp_book_children(NAME) == ...  &&  cdp_book_get_auto_id(NAME) == (CDP_NPOS_MINVAL + ...));
}


static void system_initiate_agents(void) {
    AGENCY = cdp_book_add_dictionary(&SYSTEM, CDP_NAME_AGENCY, CDP_TAG_DICTIONARY, CDP_STO_CHD_RED_BLACK_T);

    // Core tags

    cdpID recordID = cdp_agency_add_task("record", 0, 0, NULL, 4, NULL, cdp_agent_terminate);

    cdp_system_agency_add(recordID, CDP_NAME_CONNECT, cdp_agent_connect);
    cdp_system_set_action_by_id(recordID, CDP_NAME_REMOVE, cdp_agent_remove);
    cdp_system_set_action_by_id(recordID, CDP_NAME_NEXT, cdp_agent_next);
    cdp_system_set_action_by_id(recordID, CDP_NAME_PREVIOUS, cdp_agent_previous);
    cdp_system_set_action_by_id(recordID, CDP_NAME_VALIDATE, cdp_agent_validate);

    cdpID bookID = cdp_system_set_agent("book", 0, 1, &recordID, 13, cdp_agent_initiate_book, NULL);

    cdp_system_set_action_by_id(bookID, CDP_NAME_RESET, cdp_agent_reset_book);

    cdp_system_set_action_by_id(bookID, CDP_NAME_ADD, cdp_agent_add);
    cdp_system_set_action_by_id(bookID, CDP_NAME_PREPEND, cdp_agent_prepend);
    cdp_system_set_action_by_id(bookID, CDP_NAME_INSERT, cdp_agent_insert);
    cdp_system_set_action_by_id(bookID, CDP_NAME_FIRST, cdp_agent_first);
    cdp_system_set_action_by_id(bookID, CDP_NAME_LAST, cdp_agent_last);
    cdp_system_set_action_by_id(bookID, CDP_NAME_TAKE, cdp_agent_take);
    cdp_system_set_action_by_id(bookID, CDP_NAME_POP, cdp_agent_pop);
    cdp_system_set_action_by_id(bookID, CDP_NAME_SEARCH, cdp_agent_search);
    cdp_system_set_action_by_id(bookID, CDP_NAME_LINK, cdp_agent_link);
    cdp_system_set_action_by_id(bookID, CDP_NAME_SHADOW, cdp_agent_shadow);
    cdp_system_set_action_by_id(bookID, CDP_NAME_CLONE, cdp_agent_clone);
    cdp_system_set_action_by_id(bookID, CDP_NAME_MOVE, cdp_agent_move);

    cdpID registerID = cdp_system_set_agent("register", 1, 1, &recordID, 10, cdp_agent_initiate_register, NULL);

    cdp_system_set_action_by_id(registerID, CDP_NAME_RESET, cdp_agent_reset_register);

    cdp_system_set_action_by_id(registerID, CDP_NAME_REFERENCE, cdp_agent_reference);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNREFERENCE, cdp_agent_unreference);
    cdp_system_set_action_by_id(registerID, CDP_NAME_SERIALIZE, cdp_agent_serialize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNSERIALIZE, cdp_agent_unserialize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_TEXTUALIZE, cdp_agent_textualize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNTEXTUALIZE, cdp_agent_untextualize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_READ, cdp_agent_read);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UPDATE, cdp_agent_update);
    cdp_system_set_action_by_id(registerID, CDP_NAME_PATCH, cdp_agent_patch);

    /*cdpID linkID =*/ cdp_system_set_agent("link", 0, 1, &recordID, 0, cdp_agent_initiate_link, NULL);


    // Book agents

    cdpID dictID = cdp_system_set_agent("dictionary", 0, 1, &bookID, 2, NULL, NULL);

    cdp_system_set_action_by_id(dictID,     CDP_NAME_PREPEND,   cdp_agent_ignore);
    cdp_system_set_action_by_id(dictID,     CDP_NAME_INSERT,    cdp_agent_ignore);

    cdpID listID = cdp_system_set_agent("list", 0, 1, &bookID, 3, NULL, NULL);

    cdp_system_set_action_by_id(listID,     CDP_NAME_MOVE,      cdp_agent_ignore);
    cdp_system_set_action_by_id(listID,     CDP_NAME_REMOVE,    cdp_agent_ignore);
    cdp_system_set_action_by_id(listID,     CDP_NAME_INSERT,    cdp_agent_ignore);

    cdpID queueID = cdp_system_set_agent("queue", 0, 1, &bookID, 5, NULL, NULL);

    cdp_system_set_action_by_id(queueID,    CDP_NAME_MOVE,      cdp_agent_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_REMOVE,    cdp_agent_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_PREPEND,   cdp_agent_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_INSERT,    cdp_agent_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_TAKE,      cdp_agent_ignore);

    cdpID stackID = cdp_system_set_agent("stack", 0, 1, &bookID, 5, NULL, NULL);

    cdp_system_set_action_by_id(stackID,    CDP_NAME_MOVE,      cdp_agent_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_REMOVE,    cdp_agent_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_ADD,       cdp_agent_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_INSERT,    cdp_agent_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_TAKE,      cdp_agent_ignore);


    // Register agents

    #define set_agent_and_textualization(agent, size)                          \
        cdpID agent##ID = cdp_system_set_agent(#agent, size, 1, &registerID, 2, NULL, NULL);\
        cdp_system_set_action_by_id(agent##ID, CDP_NAME_TEXTUALIZE, cdp_agent_textualize_##agent);\
        cdp_system_set_action_by_id(agent##ID, CDP_NAME_UNTEXTUALIZE, cdp_agent_untextualize_##agent)

    set_agent_and_textualization(byte,    sizeof(uint8_t));
    set_agent_and_textualization(uint16,  sizeof(uint16_t));
    set_agent_and_textualization(uint32,  sizeof(uint32_t));
    set_agent_and_textualization(uint64,  sizeof(uint64_t));
    set_agent_and_textualization(int16,   sizeof(int16_t));
    set_agent_and_textualization(int32,   sizeof(int32_t));
    set_agent_and_textualization(int64,   sizeof(int64_t));
    set_agent_and_textualization(float32, sizeof(float));
    set_agent_and_textualization(float64, sizeof(double));

    /*cdpID idID =*/ cdp_system_set_agent("id", sizeof(cdpID), 1, &registerID, 4, NULL, NULL);
    //cdp_system_set_action_by_id(idID, CDP_NAME_SERIALIZE, cdp_agent_serialize_id);
    //cdp_system_set_action_by_id(idID, CDP_NAME_UNSERIALIZE, cdp_agent_unserialize_id);
    //cdp_system_set_action_by_id(idID, CDP_NAME_TEXTUALIZE, cdp_agent_textualize_id);
    //cdp_system_set_action_by_id(idID, CDP_NAME_UNTEXTUALIZE, cdp_agent_untextualize_id);

    /*cdpID utf8ID =*/ cdp_system_set_agent("utf8", 1, 1, &registerID, 0, NULL, NULL);

    /*cdpID patchID =*/ cdp_system_set_agent("patch", 1, 1, &registerID, 0, NULL, NULL);


    // Enumerations

    cdpID booleanID = cdp_system_set_agent("boolean", 1, 1, &registerID, 1, NULL, NULL);

    cdpRecord* value = cdp_book_add_dictionary( cdp_system_get_agent(booleanID),
                                                CDP_NAME_ENUMERATION,
                                                CDP_STO_CHD_ARRAY,
                                                CDP_VALUE_BOOLEAN_COUNT ); {
        cdp_book_add_static_text(value, CDP_VALUE_FALSE, "false");
        cdp_book_add_static_text(value, CDP_VALUE_TRUE,  "true");

        assert(cdp_book_children(value) == CDP_VALUE_BOOLEAN_COUNT);
        cdp_book_set_auto_id(value, CDP_VALUE_BOOLEAN_COUNT);
    }

    cdpID internedID = cdp_system_set_agent("interned", sizeof(cdpID), 1, &registerID, 1, NULL, NULL);

    NAME = cdp_book_add_book(   cdp_system_get_agent(internedID),
                                CDP_NAME_ENUMERATION,
                                CDP_TAG_BOOK,
                                CDP_STO_CHD_PACKED_QUEUE,
                                cdp_next_pow_of_two(CDP_NAME_COUNT + CDP_TASK_COUNT + CDP_ACTION_COUNT) );


    // Link types
    // ...


    // Structured agents

    /*cdpID agentID =*/ cdp_system_set_agent("agent", 0, 1, &bookID, 1, NULL, NULL);


    // Finish agents
    assert(cdp_book_children(SYSTEM) == CDP_TAG_COUNT);
    cdp_book_set_auto_id(SYSTEM, CDP_TAG_COUNT);
}



static void system_initiate(void) {
    cdp_record_system_initiate();

    /* Initiate root book structure.
    */
    SYSTEM  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_SYSTEM,  CDP_TAG_DICTIONARY, CDP_STO_CHD_ARRAY, 4);
    USER    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_USER,    CDP_TAG_DICTIONARY, CDP_STO_CHD_RED_BLACK_T);
    PUBLIC  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_PUBLIC,  CDP_TAG_DICTIONARY, CDP_STO_CHD_RED_BLACK_T);
    DATA    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_DATA,    CDP_TAG_DICTIONARY, CDP_STO_CHD_RED_BLACK_T);
    NETWORK = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_NETWORK, CDP_TAG_DICTIONARY, CDP_STO_CHD_RED_BLACK_T);
    TEMP    = cdp_book_add_book(&CDP_ROOT, CDP_NAME_TEMP, CDP_TAG_BOOK, CDP_STO_CHD_RED_BLACK_T);

    /* Initiate agents and names (in that order).
    */
    system_initiate_tags();
    system_initiate_names();
    system_initiate_agents();

    CASCADE = cdp_book_add_dictionary(&SYSTEM, CDP_NAME_CASCADE, CDP_TAG_DICTIONARY, CDP_STO_CHD_RED_BLACK_T);

    /* Initiate global records.
    */
    {
        CDP_VOID = cdp_book_add_bool(TEMP, CDP_TAG_VOID, 0);
        CDP_VOID->metadata.agent = CDP_VOID->metadata.type = CDP_TYPE_VOID;
        CDP_VOID->metadata.id = CDP_TAG_VOID;
        CDP_RECORD_SET_ATTRIB(CDP_VOID, CDP_ATTRIB_FACTUAL);
    }
}




static bool system_traverse(cdpBookEntry* entry, void* p) {
    cdpID signalID = cdp_p2v(p);
    cdpAgent action = cdp_dict_get_agent(entry->record, signalID);
    if (action) {
        if (SYSTEM_SIGNAL)
            SYSTEM_SIGNAL->nameID = signalID;
        else
            SYSTEM_SIGNAL = cdp_task_new(signalID, 1, 0);
        return action(NULL, SYSTEM_SIGNAL);     // ToDo: report errors during startup.
    }
    return true;
}


bool cdp_system_startup(void) {
    assert(SYSTEM);
    return cdp_book_traverse(SYSTEM, (cdpTraverse)system_traverse, cdp_v2p(CDP_NAME_STARTUP), NULL);
}




bool cdp_system_step(void) {
    assert(SYSTEM);
    // Pending...
    return true;
}




void cdp_system_shutdown(void) {
    assert(SYSTEM);

    cdp_book_traverse(SYSTEM, system_traverse, cdp_v2p(CDP_NAME_SHUTDOWN), NULL);

    if (CONNECT_SIGNAL)
        cdp_task_del(CONNECT_SIGNAL);
    if (SYSTEM_SIGNAL)
        cdp_task_del(SYSTEM_SIGNAL);
    cdp_system_finalize_tasks();

    cdp_book_reset(&CDP_ROOT);
    cdp_record_system_shutdown();

    SYSTEM = NULL;
}

