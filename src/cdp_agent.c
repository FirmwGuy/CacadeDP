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


#include "cdp_agent.h"
#include "cdp_action.h"
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

cdpSignal* SYSTEM_SIGNAL;
cdpSignal* CONNECT_SIGNAL;


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


cdpID cdp_name_id_add(const char* name, bool borrow) {
    assert(name && *name);
    if (!SYSTEM)  system_initiate();

    size_t length = 0;
    for (const char* c=name; *c; c++, length++) {
        if (isupper(*c)) {
            assert(!isupper(*c));   // ToDo: make lowercase.
            return CDP_NAME_VOID;
    }   }

    // Find previous
    struct NID nid = {name, length};
    if (!cdp_book_traverse(NAME, (cdpTraverse)name_id_traverse_find_text, &nid, NULL))
        return nid.id;

    // Add new
    cdpRecord* reg = cdp_book_add_text(NAME, borrow? CDP_ATTRIB_FACTUAL: 0, CDP_AUTO_ID, borrow, name);
    return CDP_POS2NAMEID(cdp_record_get_id(reg));
}


cdpRecord* cdp_name_id_text(cdpID nameID) {
    cdpID id = CDP_NAMEID2POS(nameID);
    assert(id < cdp_book_get_auto_id(NAME));
    return cdp_book_find_by_position(NAME, id);     // FixMe: check if entry is disabled.
}




/*
 *    Agent related routines
 */


static bool agent_traverse_find_by_text(cdpBookEntry* entry, struct NID* nid) {
    cdpRecord* nameReg = cdp_book_find_by_name(entry->record, CDP_NAME_NAME);
    const char* name = cdp_register_read_utf8(nameReg);
    if (cdp_register_size(nameReg) == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = cdp_record_tag(entry->record);
        return false;
    }
    return true;
}


cdpID cdp_system_set_agent( const char* name,
                            size_t      baseSize,
                            unsigned    assimLength,
                            cdpID*      assimilate,
                            unsigned    numAction,
                            cdpAgent   initiate,
                            cdpAgent   finalize ) {
    assert(name && *name);
    if (!SYSTEM)  system_initiate();

    // Find previous
    struct NID nid = {name, strlen(name)};
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
    if (initiate)   numAction++;
    if (finalize)   numAction++;

    cdpRecord* agent = cdp_book_add_dictionary(SYSTEM, CDP_AUTO_ID, storage, numAction);
    if (assimLength) {
        assert(assimLength == 1);   // FixMe: multiple parents.
        cdp_book_add_id(agent, CDP_NAME_ASSIMILATE, *assimilate);
    }
    cdp_book_add_static_text(agent, CDP_NAME_NAME, name);
    if (baseSize)
        cdp_book_add_uint32(agent, CDP_NAME_SIZE, baseSize);
    if (initiate)
        cdp_book_add_action(agent, CDP_NAME_INITIATE, initiate);
    if (finalize)
        cdp_book_add_action(agent, CDP_NAME_TERMINATE, finalize);

    return cdp_record_get_id(agent);
}


cdpRecord* cdp_system_get_agent(cdpID agentID) {
    assert(agentID < cdp_book_get_auto_id(SYSTEM));
    //return cdp_book_find_by_name(SYSTEM, agentID);
    return cdp_book_find_by_position(SYSTEM, agentID);     // FixMe: check if entry is disabled.
}


void cdp_system_set_action_by_id(cdpID agentID, cdpID nameID, cdpAgent action) {
    assert(SYSTEM && action);
    cdpRecord* agent = cdp_system_get_agent(agentID);
    cdp_book_add_action(agent, nameID, action);
}


cdpID cdp_system_set_action(cdpID agentID, const char* name, cdpAgent action) {
    assert(SYSTEM && action);
    cdpID nameID = cdp_name_id_add_static(name);
    assert(nameID != CDP_NAME_ASSIMILATE);
    cdp_system_set_action_by_id(agentID, nameID, action);
    return nameID;
}


cdpAgent cdp_system_get_action(cdpID agentID, cdpID actionID) {
    assert(agentID < cdp_book_get_auto_id(SYSTEM));
    cdpRecord* agent = cdp_system_get_agent(agentID);
    return cdp_dict_get_agent(agent, actionID);
}


/* Executes the associated signal handler in the specified agent instance.
*/
static bool cdp_system_does_action_internal(cdpRecord* instance, cdpSignal* signal) {
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

    return cdp_action_ignore(instance, signal);
}


bool cdp_system_does_action(cdpRecord* instance, cdpSignal* signal) {
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




bool cdp_system_agent_is_compatible(cdpID agentIdSrc, cdpID agentIdTgt) {
    // FixMe: check assimilations.
    return (agentIdSrc != agentIdTgt);
}


bool cdp_system_connect(cdpRecord* instanceSrc, cdpID output, cdpRecord* recordTgt) {
    assert(SYSTEM);

    if (!CONNECT_SIGNAL)
        CONNECT_SIGNAL = cdp_signal_new(CDP_NAME_CONNECT, 1, 0);

    cdp_book_add_link(&CONNECT_SIGNAL->input, output, recordTgt);

    bool done = cdp_system_does_action(instanceSrc, CONNECT_SIGNAL);

    CDP_RECORD_SET_ATTRIB(recordTgt, CDP_ATTRIB_CONNECTED);   // FixMe: Only outputs need to be marked as "connected".

    cdp_signal_reset(CONNECT_SIGNAL);

    return done;
}


bool cdp_system_disconnect(cdpRecord* link) {
    return false;
}




static void system_initiate_agents(void) {
    /**** WARNING: cdp_system_set_agent() must be done in the same order
                   as the _cdpTagID enumeration in "cdp_record.h". ****/


    // Core agents

    cdp_system_set_agent("void", 0, 0, NULL, 0, NULL, NULL);

    cdpID recordID = cdp_system_set_agent("record", 0, 0, NULL, 4, NULL, cdp_action_terminate);

    cdp_system_set_action_by_id(recordID, CDP_NAME_CONNECT, cdp_action_connect);
    cdp_system_set_action_by_id(recordID, CDP_NAME_REMOVE, cdp_action_remove);
    cdp_system_set_action_by_id(recordID, CDP_NAME_NEXT, cdp_action_next);
    cdp_system_set_action_by_id(recordID, CDP_NAME_PREVIOUS, cdp_action_previous);
    cdp_system_set_action_by_id(recordID, CDP_NAME_VALIDATE, cdp_action_validate);

    cdpID bookID = cdp_system_set_agent("book", 0, 1, &recordID, 13, cdp_action_initiate_book, NULL);

    cdp_system_set_action_by_id(bookID, CDP_NAME_RESET, cdp_action_reset_book);

    cdp_system_set_action_by_id(bookID, CDP_NAME_ADD, cdp_action_add);
    cdp_system_set_action_by_id(bookID, CDP_NAME_PREPEND, cdp_action_prepend);
    cdp_system_set_action_by_id(bookID, CDP_NAME_INSERT, cdp_action_insert);
    cdp_system_set_action_by_id(bookID, CDP_NAME_FIRST, cdp_action_first);
    cdp_system_set_action_by_id(bookID, CDP_NAME_LAST, cdp_action_last);
    cdp_system_set_action_by_id(bookID, CDP_NAME_TAKE, cdp_action_take);
    cdp_system_set_action_by_id(bookID, CDP_NAME_POP, cdp_action_pop);
    cdp_system_set_action_by_id(bookID, CDP_NAME_SEARCH, cdp_action_search);
    cdp_system_set_action_by_id(bookID, CDP_NAME_LINK, cdp_action_link);
    cdp_system_set_action_by_id(bookID, CDP_NAME_SHADOW, cdp_action_shadow);
    cdp_system_set_action_by_id(bookID, CDP_NAME_CLONE, cdp_action_clone);
    cdp_system_set_action_by_id(bookID, CDP_NAME_MOVE, cdp_action_move);

    cdpID registerID = cdp_system_set_agent("register", 1, 1, &recordID, 10, cdp_action_initiate_register, NULL);

    cdp_system_set_action_by_id(registerID, CDP_NAME_RESET, cdp_action_reset_register);

    cdp_system_set_action_by_id(registerID, CDP_NAME_REFERENCE, cdp_action_reference);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNREFERENCE, cdp_action_unreference);
    cdp_system_set_action_by_id(registerID, CDP_NAME_SERIALIZE, cdp_action_serialize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNSERIALIZE, cdp_action_unserialize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_TEXTUALIZE, cdp_action_textualize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UNTEXTUALIZE, cdp_action_untextualize);
    cdp_system_set_action_by_id(registerID, CDP_NAME_READ, cdp_action_read);
    cdp_system_set_action_by_id(registerID, CDP_NAME_UPDATE, cdp_action_update);
    cdp_system_set_action_by_id(registerID, CDP_NAME_PATCH, cdp_action_patch);

    /*cdpID linkID =*/ cdp_system_set_agent("link", 0, 1, &recordID, 0, cdp_action_initiate_link, NULL);


    // Book agents

    cdpID dictID = cdp_system_set_agent("dictionary", 0, 1, &bookID, 2, NULL, NULL);

    cdp_system_set_action_by_id(dictID,     CDP_NAME_PREPEND,   cdp_action_ignore);
    cdp_system_set_action_by_id(dictID,     CDP_NAME_INSERT,    cdp_action_ignore);

    cdpID listID = cdp_system_set_agent("list", 0, 1, &bookID, 3, NULL, NULL);

    cdp_system_set_action_by_id(listID,     CDP_NAME_MOVE,      cdp_action_ignore);
    cdp_system_set_action_by_id(listID,     CDP_NAME_REMOVE,    cdp_action_ignore);
    cdp_system_set_action_by_id(listID,     CDP_NAME_INSERT,    cdp_action_ignore);

    cdpID queueID = cdp_system_set_agent("queue", 0, 1, &bookID, 5, NULL, NULL);

    cdp_system_set_action_by_id(queueID,    CDP_NAME_MOVE,      cdp_action_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_REMOVE,    cdp_action_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_PREPEND,   cdp_action_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_INSERT,    cdp_action_ignore);
    cdp_system_set_action_by_id(queueID,    CDP_NAME_TAKE,      cdp_action_ignore);

    cdpID stackID = cdp_system_set_agent("stack", 0, 1, &bookID, 5, NULL, NULL);

    cdp_system_set_action_by_id(stackID,    CDP_NAME_MOVE,      cdp_action_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_REMOVE,    cdp_action_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_ADD,       cdp_action_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_INSERT,    cdp_action_ignore);
    cdp_system_set_action_by_id(stackID,    CDP_NAME_TAKE,      cdp_action_ignore);


    // Register agents

    #define set_agent_and_textualization(agent, size)                          \
        cdpID agent##ID = cdp_system_set_agent(#agent, size, 1, &registerID, 2, NULL, NULL);\
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

    /*cdpID idID =*/ cdp_system_set_agent("id", sizeof(cdpID), 1, &registerID, 4, NULL, NULL);
    //cdp_system_set_action_by_id(idID, CDP_NAME_SERIALIZE, cdp_action_serialize_id);
    //cdp_system_set_action_by_id(idID, CDP_NAME_UNSERIALIZE, cdp_action_unserialize_id);
    //cdp_system_set_action_by_id(idID, CDP_NAME_TEXTUALIZE, cdp_action_textualize_id);
    //cdp_system_set_action_by_id(idID, CDP_NAME_UNTEXTUALIZE, cdp_action_untextualize_id);

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
                                cdp_next_pow_of_two(CDP_NAME_COUNT + CDP_SIGNAL_COUNT + CDP_ACTION_COUNT) );


    // Link types
    // ...


    // Structured agents

    /*cdpID agentID =*/ cdp_system_set_agent("agent", 0, 1, &bookID, 1, NULL, NULL);


    // Finish agents
    assert(cdp_book_children(SYSTEM) == CDP_TAG_COUNT);
    cdp_book_set_auto_id(SYSTEM, CDP_TAG_COUNT);
}


static void system_initiate_names(void) {
    /**** WARNING: this must be done in the same order as the _cdpNameID
                   enumeration in "cdp_record.h" and "cdp_agent.h". ****/

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,            "");     // Void text.
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,           "/");     // The root book.

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "system");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "cascade");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "user");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "private");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "public");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "data");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "network");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "temp");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "assimilate");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "name");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "size");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "enumeration");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "agent");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "action");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "input");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "output");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "debug");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "warning");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "error");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "fatal");

    cdp_system_initiate_signals();
    cdp_system_initiate_actions();

    assert(cdp_book_get_auto_id(NAME) == (CDP_NAME_COUNT + CDP_SIGNAL_COUNT + CDP_ACTION_COUNT));
}


static void system_initiate(void) {
    cdp_record_system_initiate();

    /* Initiate root book structure.
    */
    SYSTEM  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_SYSTEM,  0, CDP_STO_CHD_ARRAY, CDP_TAG_COUNT);
    CASCADE = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_CASCADE, 0, CDP_STO_CHD_RED_BLACK_T);
    USER    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_USER,    0, CDP_STO_CHD_RED_BLACK_T);
    PUBLIC  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_PUBLIC,  0, CDP_STO_CHD_RED_BLACK_T);
    DATA    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_DATA,    0, CDP_STO_CHD_RED_BLACK_T);
    NETWORK = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_NETWORK, 0, CDP_STO_CHD_RED_BLACK_T);
    TEMP    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_TEMP,    0, CDP_STO_CHD_RED_BLACK_T);

    /* Initiate agents and names (in that order).
    */
    system_initiate_agents();
    system_initiate_names();

    /* Initiate global records.
    */
    {
        CDP_VOID = cdp_book_add_bool(TEMP, CDP_NAME_VOID, 0);
        CDP_VOID->metadata.agent = CDP_VOID->metadata.type = CDP_TYPE_VOID;
        CDP_VOID->metadata.id = CDP_NAME_VOID;
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
            SYSTEM_SIGNAL = cdp_signal_new(signalID, 1, 0);
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
        cdp_signal_del(CONNECT_SIGNAL);
    if (SYSTEM_SIGNAL)
        cdp_signal_del(SYSTEM_SIGNAL);
    cdp_system_finalize_signals();

    cdp_book_reset(&CDP_ROOT);
    cdp_record_system_shutdown();

    SYSTEM = NULL;
}

