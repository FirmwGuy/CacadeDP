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


#include "cdp_system_get_agent.h"
#include "cdp_signal.h"
#include <ctype.h>        // isupper()



extern cdpRecord CDP_ROOT;

cdpRecord* SYSTEM;
cdpRecord* CASCADE;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* NAME;

cdpRecord* CDP_VOID;


static void system_initiate(void);




/*
 *   String interning routines
 */


struct NID {const char* name; size_t length; cdpID id;};


static bool name_id_traverse_find_text(cdpBookEntry* entry, unsigned depth, struct NID* nid) {
    const char* name = cdp_register_read_utf8(entry->record);
    if (cdp_register_size(entry->record) == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = entry->record->metadata.id;
        return false;
    }
    return true;
}


cdpID cdp_name_id_add(const char* name, bool borrow) {
    assert(name && *name);
    if (!SYSTEM)  system_initiate();

    size_t length = strlen(name);
    for (unsigned n=0; n<length; n++) {
        if (isupper(name[n])) {
            assert(!isupper(name[n]));
            return CDP_NAME_VOID;
    }   }

    // Find previous
    struct NID nid = {name, length};
    if (!cdp_book_traverse(NAME, (cdpTraverse)name_id_traverse_find_text, &nid, NULL))
        return nid.id;

    // Add new
    cdpRecord* reg = cdp_book_add_text(NAME, borrow? CDP_ATTRIB_FACTUAL: 0, CDP_AUTO_ID, borrow, name);
    return CDP_TEXT2ID(cdp_record_id(reg));
}


cdpRecord* cdp_name_id_text(cdpID id) {
    cdpID textID = CDP_ID2TEXT(id);
    assert(textID < cdp_book_get_auto_id(NAME));
    //return cdp_book_find_by_name(NAME, textID);
    return cdp_book_find_by_position(NAME, textID);     // FixMe: check if entry is disabled.
}




/*
 *    Agent related routines
 */


static bool agent_traverse_find_by_text(cdpBookEntry* entry, unsigned depth, struct NID* nid) {
    cdpRecord* nameReg = cdp_book_find_by_name(entry->record, CDP_NAME_NAME);
    const char* name = cdp_register_read_utf8(nameReg);
    if (cdp_register_size(nameReg) == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = cdp_record_agent(entry->record);
        return false;
    }
    return true;
}


cdpID cdp_system_set_agent( const char* name,
                            size_t      baseSize,
                            unsigned    assimLenght,
                            cdpID*      assimilate
                            unsigned    numAction,
                            cdpAction   create,
                            cdpAction   destroy ) {
    assert(create || destroy);
    if (!SYSTEM)  system_initiate();

    // Find previous
    struct NID nid = {name, strlen(name);};
    bool found = !cdp_book_traverse(SYSTEM, (cdpTraverse)agent_traverse_find_by_text, &nid, NULL);
    if (found) {
        assert(!found);     // FixMe.
        return CDP_TYPE_VOID;
    }

    // Add new
    unsigned storage;
    if (numAction) {
        storage = CDP_STO_CHD_ARRAY;
        if (baseSize)
            numAction += 2;
        else
            numAction++;
    }
    else
        storage = CDP_STO_CHD_RED_BLACK_T;

    cdpRecord* agent = cdp_book_add_dictionary(SYSTEM, CDP_AUTO_ID, storage, numAction);
    cdp_book_add_static_text(agent, CDP_NAME_NAME, name);
    if (baseSize)
        cdp_book_add_uint32(agent, CDP_NAME_SIZE, baseSize);
    if (create)
        cdp_book_add_action(agent, CDP_NAME_CREATE, create);
    if (destroy)
        cdp_book_add_action(agent, CDP_NAME_DESTROY, destroy);

    return cdp_record_id(agent);
}


cdpRecord* cdp_system_get_agent(cdpID agentID) {
    assert(agentID < cdp_book_get_auto_id(SYSTEM));
    //return cdp_book_find_by_name(SYSTEM, agentID);
    return cdp_book_find_by_position(SYSTEM, agentID);     // FixMe: check if entry is disabled.
}


void cdp_system_set_action_by_id(cdpID agentID, cdpID nameID, cdpAction action) {
    assert(SYSTEM && action);
    cdpRecord* agent = cdp_system_get_agent(agentID);
    cdp_book_add_action(agent, nameID, action);
}


cdpID cdp_system_set_action(cdpID agentID, const char* name, cdpAction action) {
    assert(SYSTEM && action);
    cdpID nameID = cdp_name_id_add_static(name);
    cdp_system_set_action_by_id(agentID, nameID, action);
    return nameID;
}


cdpAction cdp_system_get_action(cdpID agentID, cdpID actionID) {
    assert(agentID < cdp_book_get_auto_id(SYSTEM)  &&  actionID);

    cdpRecord* agent = cdp_system_get_agent(agentID);
    cdpRecord* actionReg = cdp_book_find_by_name(agent, actionID);
    return cdp_register_read_action(actionReg);
}


/* Executes the associated signal handler for the specified agent instance.
*/
bool cdp_system_does_action(cdpRecord* instance, cdpRecord* signal) {
    assert(instance && signal);

    cdpRecord* agent = cdp_system_get_agent(cdp_record_agent(instance));
    cdpRecord* actionReg = cdp_book_find_by_name(agent, cdp_record_id(instance));
    do {
        cdpRecord* agent = cdp_system_get_agent(cdp_record_agent(instance));
        cdpRecord* actionReg = cdp_book_find_by_name(agent, cdp_record_id(instance));
    } while(!actionReg);

    cdpAction* action = cdp_register_read_action(actionReg);
    return action(instance, signal);
}




static void system_initiate(void) {
    cdp_record_system_initiate();


    /* Initiate root book structure.
    */
    SYSTEM  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_SYSTEM,  CDP_STO_CHD_ARRAY, CDP_AGENT_COUNT);
    CASCADE = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_CASCADE, CDP_STO_CHD_RED_BLACK_T);
    USER    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_USER,    CDP_STO_CHD_RED_BLACK_T);
    PUBLIC  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_PUBLIC,  CDP_STO_CHD_RED_BLACK_T);
    DATA    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_DATA,    CDP_STO_CHD_RED_BLACK_T);
    NETWORK = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_NETWORK, CDP_STO_CHD_RED_BLACK_T);
    TEMP    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_TEMP,    CDP_STO_CHD_RED_BLACK_T);


    /* Initiate agent (ID) system:
       *** WARNING: this must be done in the same order as the _cdpAgentID
                    enumeration in "cdp_record.h". ***
    */

    // Core agents

    cdp_system_set_agent("void", 0, NULL, 0, NULL, NULL);

    cdpID recordID = cdp_system_set_agent("record", 0, 0, NULL, 8, cdp_action_create_record, cdp_action_destroy);
    cdp_system_set_action_by_id(recordID, CDP_NAME_FREE, cdp_action_free);
    cdp_system_set_action_by_id(recordID, CDP_NAME_REFERENCE, cdp_action_reference);

    cdp_system_set_action_by_id(recordID, CDP_NAME_LINK, cdp_action_link);
    cdp_system_set_action_by_id(recordID, CDP_NAME_COPY, cdp_action_copy);
    cdp_system_set_action_by_id(recordID, CDP_NAME_MOVE, cdp_action_move);
    cdp_system_set_action_by_id(recordID, CDP_NAME_REMOVE, cdp_action_remove);

    cdp_system_set_action_by_id(recordID, CDP_NAME_NEXT, cdp_action_next);
    cdp_system_set_action_by_id(recordID, CDP_NAME_PREVIOUS, cdp_action_previous);
    cdp_system_set_action_by_id(recordID, CDP_NAME_VALIDATE, cdp_action_validate);

    cdpID bookID = cdp_system_set_agent("book", 0, 1, &recordID, 10, cdp_action_create_book, NULL);
    cdp_system_set_action_by_id(bookID, CDP_NAME_RESET, cdp_action_reset_book);

    cdp_system_set_action_by_id(bookID, CDP_NAME_ADD, cdp_action_add);
    cdp_system_set_action_by_id(bookID, CDP_NAME_PREPEND, cdp_action_prepend);
    cdp_system_set_action_by_id(bookID, CDP_NAME_INSERT, cdp_action_insert);

    cdp_system_set_action_by_id(bookID, CDP_NAME_FIRST, cdp_action_first);
    cdp_system_set_action_by_id(bookID, CDP_NAME_LAST, cdp_action_last);
    cdp_system_set_action_by_id(bookID, CDP_NAME_TAKE, cdp_action_take);
    cdp_system_set_action_by_id(bookID, CDP_NAME_POP, cdp_action_pop);

    cdp_system_set_action_by_id(bookID, CDP_NAME_SEARCH, cdp_action_search);

    cdpID registerID = cdp_system_set_agent("register", 1, 1, &recordID, 9, cdp_action_create_register, NULL);
    cdp_system_set_action_by_id(registerID, CDP_NAME_RESET, cdp_action_reset_register);

    cdp_system_set_action_by_id(registerID, CDP_NAME_SERIALIZE, cdp_action_serialize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNSERIALIZE, cdp_action_unserialize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_TEXTUALIZE, cdp_action_textualize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNTEXTUALIZE, cdp_action_untextualize);

    cdp_system_set_action_by_id(registerID, CDP_NAME_READ, cdp_action_read);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UPDATE, cdp_action_update);
    cdp_system_set_action_by_id(registerID, CDP_NAME_PATCH, cdp_action_patch);

    cdpID linkID = cdp_system_set_agent("link", 0, 1, &recordID, 1, cdp_action_create_link, NULL);


    // Book agents

    cdpID dictID = cdp_system_set_agent("dictionary", 0, 1, &bookID, 2, NULL, NULL);
    cdp_system_set_action_by_id(dictID, CDP_NAME_PREPEND, cdp_action_ignore);
    cdp_system_set_action_by_id(dictID, CDP_NAME_INSERT, cdp_action_ignore);

    cdpID listID = cdp_system_set_agent("list", 0, 1, &bookID, 3, NULL, NULL);
    cdp_system_set_action_by_id(listID, CDP_NAME_MOVE, cdp_action_ignore);
    cdp_system_set_action_by_id(listID, CDP_NAME_REMOVE, cdp_action_ignore);
    cdp_system_set_action_by_id(listID, CDP_NAME_INSERT, cdp_action_ignore);

    cdpID queueID = cdp_system_set_agent("queue", 0, 1, &bookID, 2, NULL, NULL);
    cdp_system_set_action_by_id(queueID, CDP_NAME_MOVE, cdp_action_ignore);
    cdp_system_set_action_by_id(queueID, CDP_NAME_REMOVE, cdp_action_ignore);
    cdp_system_set_action_by_id(queueID, CDP_NAME_PREPEND, cdp_action_ignore);
    cdp_system_set_action_by_id(queueID, CDP_NAME_INSERT, cdp_action_ignore);
    cdp_system_set_action_by_id(queueID, CDP_NAME_TAKE, cdp_action_ignore);

    cdpID stackID = cdp_system_set_agent("stack", 0, 1, &bookID, 2, NULL, NULL);
    cdp_system_set_action_by_id(stackID, CDP_NAME_MOVE, cdp_action_ignore);
    cdp_system_set_action_by_id(stackID, CDP_NAME_REMOVE, cdp_action_ignore);
    cdp_system_set_action_by_id(stackID, CDP_NAME_ADD, cdp_action_ignore);
    cdp_system_set_action_by_id(stackID, CDP_NAME_INSERT, cdp_action_ignore);
    cdp_system_set_action_by_id(stackID, CDP_NAME_TAKE, cdp_action_ignore);


    // Register agents

#define set_agent_and_textualization(agent, size)                      \
    cdpID agent##ID = cdp_system_set_agent(#agent, size, 1, &regisgerID, 2, NULL, NULL);\
    cdp_system_set_action_by_id(agent##ID, CDP_NAME_TEXTUALIZE, cdp_action_textualize_##agent);\
    cdp_system_set_action_by_id(agent##ID, CDP_NAME_UNTEXTUALIZE, cdp_action_untextualize_##agent)

    set_agent_and_textualization(byte,    sizeof(uint8_t));
    set_agent_and_textualization(uint16,  sizeof(uint16_t));
    set_agent_and_textualization(uint32,  sizeof(uint32_t));
    set_agent_and_textualization(uint64,  sizeof(uint64_t));
    set_agent_and_textualization(int16,   sizeof(int16_t));
    set_agent_and_textualization(int32,   sizeof(int32_t));
    set_agent_and_textualization(int64,   sizeof(int64_t));
    set_agent_and_textualization(float32, sizeof(float));
    set_agent_and_textualization(float64, sizeof(double));

    cdpID idID = cdp_system_set_agent("id", sizeof(cdpID), 1, &regisgerID, 4, NULL, NULL);
    cdp_system_set_action_by_id(idID, CDP_NAME_SERIALIZE, cdp_action_serialize_id);
    cdp_system_set_action_by_id(idID, CDP_NAME_UNSERIALIZE, cdp_action_unserialize_id);
    cdp_system_set_action_by_id(idID, CDP_NAME_TEXTUALIZE, cdp_action_textualize_id);
    cdp_system_set_action_by_id(idID, CDP_NAME_UNTEXTUALIZE, cdp_action_untextualize_id);

    cdp_system_set_agent("utf8", 1, 1, &regisgerID, 1, NULL, NULL);

    set_agent_and_textualization(patch, 1);

    // Enumerations

    cdpID booleanID = cdp_system_set_agent("boolean", 1, 1, &regisgerID, 1, NULL, NULL);
    cdpRecord* value = cdp_book_add_dictionary(cdp_system_get_agent(booleanID), CDP_NAME_ENUMERATION, CDP_STO_CHD_ARRAY, CDP_ID_BOOL_COUNT); {
        cdp_book_add_static_text(value, CDP_VALUE_FALSE, "false");
        cdp_book_add_static_text(value, CDP_VALUE_TRUE,  "true");

        assert(cdp_book_children(value) == CDP_VALUE_BOOLEAN_COUNT);
        cdp_book_set_auto_id(value, CDP_VALUE_BOOLEAN_COUNT);
    }

    cdpID internedID = cdp_system_set_agent("interned", sizeof(cdpID), 1, &regisgerID, 1, NULL, NULL);
    NAME = cdp_book_add_dictionary(cdp_system_get_agent(internedID), CDP_NAME_ENUMERATION, CDP_STO_CHD_PACKED_QUEUE, cdp_next_pow_of_two(CDP_NAME_COUNT + CDP_SIGNAL_COUNT));

    // Link types

    // Structured types

    cdpID agentID = cdp_system_set_agent("agent", 0, 1, &bookID, 1, NULL, NULL);

    // Finish core types.
    assert(cdp_book_children(SYSTEM) == CDP_AGENT_COUNT);
    cdp_book_set_auto_id(SYSTEM, CDP_AGENT_COUNT);


    /* Initiate name (ID) string interning system:
       *** WARNING: this must be done in the same order as the _cdpNameID
                    enumeration in "cdp_record.h" and "cdp_agent.h". ***
    */
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,            "");     // Void text.
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,           "/");     // The root book.
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "system");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "cascade");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "user");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "private");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "public");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "data");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "network");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "temp");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "name");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "assimilate");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "size");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "enumeration");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "agent");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "action");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "input");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "output");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "debug");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "warning");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "error");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "fatal");

    cdp_signal_initiate();

    assert(cdp_book_get_auto_id(NAME) == (CDP_NAME_COUNT + CDP_SIGNAL_COUNT);


    /* Initiate global records.
    */
    CDP_VOID = cdp_book_add_boolean(TEMP, CDP_NAME_VOID, 0);
    CDP_VOID->metadata.agent = CDP_VOID->metadata.type = CDP_TYPE_VOID;
    CDP_VOID->metadata.id = CDP_NAME_VOID;
    CDP_RECORD_SET_ATRIBUTE(CDP_VOID, CDP_ATTRIB_FACTUAL);
}




static bool system_startup_traverse(cdpBookEntry* entry, unsigned unused, void* unused2) {
    cdpAction action = cdp_book_get_property(entry->record, CDP_NAME_CALL);
    if (action) {
        cdpRecord call = {0};
        cdp_record_initialize_dictionary(&call, CDP_CALL_STARTUP, CDP_STO_CHD_LINKED_LIST);
        return action(NULL, &call);
    }
    return true;
}

bool cdp_system_startup(void) {
    assert(cdp_book_children(SYSTEM));
    return cdp_book_traverse(SYSTEM, system_startup_traverse, NULL, NULL);
}


bool cdp_system_step(void) {
    assert(SYSTEM);
    return true;
}


void cdp_system_shutdown(void) {
    assert(SYSTEM);
    cdp_signal_finalize();
    cdp_book_reset(&CDP_ROOT, 64);    // FixMe: maxDepth.
    cdp_record_system_shutdown();
    SYSTEM = NULL;
}

