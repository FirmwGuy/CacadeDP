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
#include <ctype.h>        // isupper()



extern cdpRecord CDP_ROOT;

cdpRecord* CDP_VOID;
cdpRecord* CDP_TRUE;
cdpRecord* CDP_FALSE;

cdpRecord* AGENT;
cdpRecord* SYSTEM;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* NAME;

struct NID {const char* name; size_t length; cdpID id;};


static void system_initiate(void);




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




static inline cdpRecord* agent_add_entry(cdpID agentID, const char* name, uint32_t size, cdpID nameID, unsigned items) {
    if (!items) items = 2;      // FixMe.
    if (size) items++;

    cdpRecord* agent = cdp_book_add_dictionary(AGENT, agentID, CDP_STO_CHD_ARRAY, items); {
        if (name)
            cdp_book_add_static_text(agent, CDP_NAME_NAME, name);
        else {
            assert(nameID);
            cdp_book_add_id(agent, CDP_NAME_NAME, nameID);
        }

        if (size)
            cdp_book_add_uint32(agent, CDP_NAME_SIZE, size);
    }

    return agent;
}

static bool agent_traverse_find_by_text(cdpBookEntry* entry, unsigned depth, struct NID* nid) {
    cdpRecord* nameReg = cdp_book_find_by_name(entry->record, CDP_NAME_NAME);
    if (nameReg.metadata.agent == CDP_AGENT_ID)
        nameReg = cdp_name_id_text(cdp_register_read_id(nameReg));
    const char* name = cdp_register_read_utf8(nameReg);
    if (cdp_register_size(nameReg) == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = entry->record->metadata.id;
        return false;
    }
    return true;
}

cdpID cdp_agent_add(const char* name,
                    size_t baseSize,
                    cdpAssimilation assimilation,
                    cdpAction create,
                    cdpAction destroy) {
    assert(create || destroy);
    if (!SYSTEM)  system_initiate();

    cdpID nameID = cdp_name_id_add_static(name);

    // Find previous
    struct NID nid = {name, length};
    bool found = !cdp_book_traverse(AGENT, (cdpTraverse)agent_traverse_find_by_text, &nid, NULL);
    if (found) {
        assert(!found);     // FixMe.
        return CDP_TYPE_VOID;
    }

    // Add new
    cdpRecord* agent = agent_add_entry(CDP_AUTO_ID, NULL, baseSize, nameID, 4); {  // FixMe.
        // FixMe: type/ context.
        if (create)
            cdp_book_add_action(agent, CDP_NAME_CREATE, create);
        if (destroy)
            cdp_book_add_action(agent, CDP_NAME_DESTROY, destroy);
    }

    return cdp_record_id(agent);
}


cdpRecord* cdp_agent(cdpID agentID) {
    assert(SYSTEM && agentID && (agentID < cdp_book_get_auto_id(AGENT)));
    //return cdp_book_find_by_name(AGENT, agentID);
    return cdp_book_find_by_position(AGENT, agentID);     // FixMe: check if entry is disabled.
}


cdpID cdp_agent_add_action(cdpID agentID, const char* name, cdpAction action) {
    assert(SYSTEM && action);

    cdpRecord* agent = cdp_agent(agentID);
    assert(agent);

    cdpID nameID = cdp_name_id_add_static(name);

    cdp_book_add_action(agent, nameID, action);

    return nameID;
}




#define system_initiate_agent(t, name, desc, size)   agent_add_entry(t, name, desc, size, 0)

static void system_initiate(void) {
    if (SYSTEM) return;

    cdp_record_system_initiate();


    /* Initiate root book structure.
    */
    AGENT   = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_AGENT,   CDP_STO_CHD_ARRAY, CDP_AGENT_COUNT);
    SYSTEM  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_SYSTEM,  CDP_STO_CHD_RED_BLACK_T);
    USER    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_USER,    CDP_STO_CHD_RED_BLACK_T);
    PUBLIC  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_PUBLIC,  CDP_STO_CHD_RED_BLACK_T);
    DATA    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_DATA,    CDP_STO_CHD_RED_BLACK_T);
    NETWORK = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_NETWORK, CDP_STO_CHD_RED_BLACK_T);
    TEMP    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_TEMP,    CDP_STO_CHD_RED_BLACK_T);


    /* Initiate agent system. */
    cdpRecord* agent, *value;

    // Abstract types
    system_initiate_agent(CDP_TYPE_VOID,           "void",           "Type for describing nothingness.", 0);

    // Book types
    system_initiate_agent(CDP_TYPE_BOOK,           "book",           "Generic container of records.", 0);
    system_initiate_agent(CDP_TYPE_LINK,           "list",           "Book with records ordered by how they are added/removed", 0);
    system_initiate_agent(CDP_AGENT_QUEUE,          "queue",          "List that only removes records from its beginning or adds them to its end.", 0);
    system_initiate_agent(CDP_AGENT_STACK,          "stack",          "List that only adds/removes records from its beginning.", 0);
    system_initiate_agent(CDP_AGENT_DICTIONARY,     "dictionary",     "Book of records sorted by their unique name.", 0);

    // Register types
    system_initiate_agent(CDP_TYPE_REGISTER,       "register",       "Generic record that holds data.", 0);
    agent = system_initiate_agent(CDP_AGENT_BOOLEAN, "boolean",        "Boolean value.", sizeof(uint8_t)); {
        value = cdp_book_add_dictionary(agent, CDP_NAME_VALUE, CDP_STO_CHD_ARRAY, CDP_ID_BOOL_COUNT); {
            cdp_book_add_static_text(value, CDP_BOOLEAN_FALSE,   "false");
            cdp_book_add_static_text(value, CDP_BOOLEAN_TRUE,    "true");

            assert(cdp_book_children(value) == CDP_BOOLEAN_COUNT);
            cdp_book_set_auto_id(value, CDP_BOOLEAN_COUNT);
    }   }
    system_initiate_agent(CDP_AGENT_BYTE,           "byte",           "Unsigned integer number of 8 bits.",   sizeof(uint8_t));
    system_initiate_agent(CDP_AGENT_UINT16,         "uint16",         "Unsigned integer number of 16 bits.",  sizeof(uint16_t));
    system_initiate_agent(CDP_AGENT_UINT32,         "uint32",         "Unsigned integer number of 32 bits.",  sizeof(uint32_t));
    system_initiate_agent(CDP_AGENT_UINT64,         "uint64",         "Unsigned integer number of 64 bits.",  sizeof(uint64_t));
    system_initiate_agent(CDP_AGENT_INT16,          "int16",          "Integer number of 16 bits.",           sizeof(int16_t));
    system_initiate_agent(CDP_AGENT_INT32,          "int32",          "Integer number of 32 bits.",           sizeof(int32_t));
    system_initiate_agent(CDP_AGENT_INT64,          "int64",          "Integer number of 64 bits.",           sizeof(int64_t));
    system_initiate_agent(CDP_AGENT_FLOAT32,        "float32",        "Floating point number of 32 bits.",    sizeof(float));
    system_initiate_agent(CDP_AGENT_FLOAT64,        "float64",        "Floating point number of 64 bits.",    sizeof(double));
    //
    system_initiate_agent(CDP_AGENT_ID,             "id",             "Register with the value of an id (name or agent) of records.", sizeof(cdpID));
    agent = system_initiate_agent(CDP_AGENT_NAME_ID, "name_id",        "Id as a text token for creating record paths.", 4); {    // FixMe: variant base size for UTF8?
        NAME = cdp_book_add_dictionary(agent, CDP_NAME_VALUE, CDP_STO_CHD_PACKED_QUEUE, cdp_next_pow_of_two(CDP_NAME_COUNT));
    }
    system_initiate_agent(CDP_AGENT_UTF8,           "utf8",           "Text encoded in UTF8 format.", 0);
    system_initiate_agent(CDP_AGENT_PATCH,          "patch",          "Record that can patch another record.", 0);
    //
    system_initiate_agent(CDP_AGENT_ACTION,       "action",       "Address of a action function.", sizeof(cdpAction));
    agent = system_initiate_agent(CDP_AGENT_EVENT,   "event",          "Object event.", sizeof(uint8_t)); {
        value = cdp_book_add_dictionary(agent, CDP_NAME_VALUE, CDP_STO_CHD_ARRAY, CDP_CALL_COUNT); {
            cdp_book_add_static_text(value, CDP_CALL_CONSTRUCT, "construct");
            cdp_book_add_static_text(value, CDP_CALL_DESTRUCT,  "destruct");
            cdp_book_add_static_text(value, CDP_CALL_REFERENCE, "reference");
            cdp_book_add_static_text(value, CDP_CALL_FREE,      "free");
            cdp_book_add_static_text(value, CDP_CALL_APPEND,    "append");
            cdp_book_add_static_text(value, CDP_CALL_PREPEND,   "prepend");
            cdp_book_add_static_text(value, CDP_CALL_INSERT,    "insert");
            cdp_book_add_static_text(value, CDP_CALL_SORT,      "sort");
            cdp_book_add_static_text(value, CDP_CALL_COPY,      "copy");
            cdp_book_add_static_text(value, CDP_CALL_MOVE,      "move");
            cdp_book_add_static_text(value, CDP_CALL_LINK,      "link");

            assert(cdp_book_children(value) == CDP_CALL_COUNT);
            cdp_book_set_auto_id(value, CDP_CALL_COUNT);
    }   }

    // Link types
    system_initiate_agent(CDP_TYPE_LINK,           "link",           "Record that points to another record.", 0);

    // Structured types
    system_initiate_agent(CDP_AGENT_AGENT,           "agent",           "Dictionary for describing types.", 0);
    system_initiate_agent(CDP_AGENT_OBJECT,         "action",         "Records structured and ordered by event signals.", 0);

    // Finish core types.
    assert(cdp_book_children(AGENT) == CDP_AGENT_COUNT);
    cdp_book_set_auto_id(AGENT, CDP_AGENT_COUNT);


    /* Initiate name (ID) string interning system:
       *** WARNING: this must be done in the same order as the _cdpNameID
                    enumeration in "cdp_agent.h". ***
    */
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,            "");     // Void text.
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,           "/");     // The root book.
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "agent");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "system");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "user");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "private");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "public");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "data");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "network");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "temp");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "name");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "size");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "value");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "action");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "return");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "error");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "startup");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "shutdown");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "cascade");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "discascade");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "create");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "destroy");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "reset");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "free");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "reference");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "link");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "copy");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "move");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "next");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "previous");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "validate");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "serialize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "unserialize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "textualize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,"untextualize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "read");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "update");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "patch");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,         "add");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "prepend");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "insert");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "first");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "last");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,         "pop");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "search");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "remove");

    assert(cdp_book_get_auto_id(NAME) == CDP_NAME_COUNT);


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
    assert(cdp_book_children(AGENT));
    return cdp_book_traverse(AGENT, system_startup_traverse, NULL, NULL);
}


bool cdp_system_step(void) {
    assert(SYSTEM);
    return true;
}


void cdp_system_shutdown(void) {
    assert(SYSTEM);
    cdp_book_reset(&CDP_ROOT, 64);    // FixMe: maxDepth.
    cdp_record_system_shutdown();
    SYSTEM = NULL;
}




/* Executes the associated signal handler for the specified agent instance.
*/
cdpActionReturn cdp_action(cdpRecord* instance, cdpRecord* signal) {
    assert(instance && signal);
    cdpRecord* agent = cdp_agent(cdp_record_agent(instance));
    cdpRecord* actionReg = cdp_book_find_by_name(agent, cdp_record_id(instance));
    cdpAction* action = cdp_register_read_action(actionReg);
    return action(instance, signal);
}




cdpRecord* cdp_create_book(cdpRecord* instance, cdpID nameID, cdpID agentID, unsigned storage, unsigned baseLength) {
    assert(instance && nameID && agentID && storage < CDP_STO_CHD_COUNT);

    cdpRecord signal = {0};     // ToDo: store signal in a system queue and pause calling task.
    cdp_record_initialize_dictionary(&signal, CDP_NAME_CREATE, CDP_STO_CHD_ARRAY, 5); {
        cdp_book_add_id(&signal, CDP_NAME_NAME, nameID);
        cdp_book_add_id(&signal, CDP_NAME_AGENT, agentID);
        cdp_book_add_id(&signal, CDP_NAME_STORAGE, storage);
        if (baseLength)
            cdp_book_add_uint32(&signal, CDP_NAME_BASE, base);
    }

    cdpActionReturn ret = cdp_action(instance, &signal);
    cdp_record_finalize(&signal);
    if (ret.error) {
        // ToDo: report error.
        return NULL;
    }

    cdpRecord* retReg = cdp_book_find_by_name(&signal, CDP_NAME_RETURN);
    cdpRecord* newBook = cdp_link_read_address(retReg);
    cdp_record_finalize(&ret);
    return newBook;
}


cdpRecord* cdp_create_register(cdpRecord* instance, cdpID nameID, cdpID agentID, void* data, size_t size) {
    assert(instance && nameID && agentID && size);

    cdpRecord signal = {0};     // ToDo: store signal in a system queue and pause calling task.
    cdp_record_initialize_dictionary(&signal, CDP_NAME_CREATE, CDP_STO_CHD_ARRAY, 5); {
        cdp_book_add_id(&signal, CDP_NAME_NAME, nameID);
        cdp_book_add_id(&signal, CDP_NAME_AGENT, agentID);
        cdp_book_add_uint32(&signal, CDP_NAME_SIZE, size);
        if (data)
            cdp_book_add_id(&signal, CDP_NAME_DATA, data);
    }

    cdpRecord* newReg;
    if (cdp_action(instance, &signal)) {
        cdpRecord* retReg = cdp_book_find_by_name(&signal, CDP_NAME_RETURN);
        newReg = cdp_link_read_address(retReg);
    } else {
        newReg = NULL;
    }

    cdp_record_finalize(&signal);
    return newReg;
}


void cdp_destroy(cdpRecord* instance) {
    cdpRecord signal = {0};
    cdp_record_initialize_id(&signal, CDP_NAME_DESTROY, CDP_NAME_DESTROY);
    cdp_action(instance, &signal);
    cdp_record_finalize(&signal);
}


void cdp_reset(cdpRecord* instance) {
    cdpRecord signal = {0};
    cdp_record_initialize_id(&signal, CDP_NAME_RESET, CDP_NAME_RESET);
    cdp_action(instance, &signal);
    cdp_record_finalize(&signal);
}


void cdp_free(cdpRecord* instance) {
    cdpRecord signal = {0};
    cdp_record_initialize_id(&signal, CDP_NAME_FREE, CDP_NAME_FREE);
    cdp_action(instance, &signal);
    cdp_record_finalize(&signal);
}


void cdp_reference(cdpRecord* instance) {
    cdpRecord signal = {0};
    cdp_record_initialize_id(&signal, CDP_NAME_REFERENCE, CDP_NAME_REFERENCE);
    cdp_action(instance, &signal);
    cdp_record_finalize(&signal);
}


cdpRecord* cdp_copy(cdpRecord* instance, cdpRecord* newParent, cdpID nameID) {
    assert(instance && cdp_record_is_book(newParent) && nameID);

    cdpRecord signal = {0};
    cdp_record_initialize_dictionary(&signal, CDP_NAME_COPY, CDP_STO_CHD_ARRAY, 3); {
        cdp_book_add_link(&signal, CDP_NAME_PARENT, newParent);
        cdp_book_add_id(&signal, CDP_NAME_NAME, nameID);
    }

    cdpRecord* newRec;
    if (cdp_action(instance, &signal)) {
        cdpRecord* retReg = cdp_book_find_by_name(&signal, CDP_NAME_RETURN);
        newRec = cdp_link_read_address(retReg);
    } else {
        newRec = NULL;
    }

    cdp_record_finalize(&signal);
    return newRec;
}



cdpRecord* cdp_move(cdpRecord* instance, cdpRecord* newParent, cdpID nameID) {
    assert(instance && cdp_record_is_book(newParent) && nameID);

    cdpRecord signal = {0};
    cdp_record_initialize_dictionary(&signal, CDP_NAME_MOVE, CDP_STO_CHD_ARRAY, 3); {
        cdp_book_add_link(&signal, CDP_NAME_PARENT, newParent);
        cdp_book_add_id(&signal, CDP_NAME_NAME, nameID);
    }

    cdpRecord* newRec;
    if (cdp_action(instance, &signal)) {
        cdpRecord* retReg = cdp_book_find_by_name(&signal, CDP_NAME_RETURN);
        newRec = cdp_link_read_address(retReg);
    } else {
        newRec = NULL;
    }

    cdp_record_finalize(&signal);
    return newRec;
}



cdpRecord* cdp_link(cdpRecord* instance, cdpRecord* newParent, cdpID nameID) {
    assert(instance && cdp_record_is_book(newParent) && nameID);

    cdpRecord signal = {0};
    cdp_record_initialize_dictionary(&signal, CDP_NAME_LINK, CDP_STO_CHD_ARRAY, 3); {
        cdp_book_add_link(&signal, CDP_NAME_PARENT, newParent);
        cdp_book_add_id(&signal, CDP_NAME_NAME, nameID);
    }

    cdpRecord* newRec;
    if (cdp_action(instance, &signal)) {
        cdpRecord* retReg = cdp_book_find_by_name(&signal, CDP_NAME_RETURN);
        newRec = cdp_link_read_address(retReg);
    } else {
        newRec = NULL;
    }

    cdp_record_finalize(&signal);
    return newRec;
}




cdpRecord* cdp_next(cdpRecord* instance) {
    assert(instance);

    cdpRecord signal = {0};
    cdp_record_initialize_link(&signal, CDP_NAME_NEXT, NULL);

    cdpRecord* nextRec;
    if (cdp_action(instance, &signal)) {
        cdpRecord* retReg = cdp_book_find_by_name(&signal, CDP_NAME_RETURN);
        nextRec = cdp_link_read_address(retReg);
    } else {
        nextRec = NULL;
    }

    cdp_record_finalize(&signal);
    return nextRec;
}



cdpRecord* cdp_previous(cdpRecord* instance) {
    assert(instance && cdp_record_is_book(newParent) && nameID);

    cdpRecord signal = {0};
    cdp_record_initialize_dictionary(&signal, CDP_NAME_COPY, CDP_STO_CHD_ARRAY, 3); {
        cdp_book_add_link(&signal, CDP_NAME_PARENT, newParent);
        cdp_book_add_id(&signal, CDP_NAME_NAME, nameID);
    }

    cdpRecord* newRec;
    if (cdp_action(instance, &signal)) {
        cdpRecord* retReg = cdp_book_find_by_name(&signal, CDP_NAME_RETURN);
        newRec = cdp_link_read_address(retReg);
    } else {
        newRec = NULL;
    }

    cdp_record_finalize(&signal);
    return newRec;
}




bool cdp_validate(cdpRecord* instance);

bool cdp_serialize(cdpRecord* instance, void** data, size_t* size);
bool cdp_unserialize(cdpRecord* instance, void* data, size_t size);
bool cdp_textualize(cdpRecord* instance, char** data, size_t* length);
bool cdp_untextualize(cdpRecord* instance, char* data, size_t length);

void* cdp_read(cdpRecord* instance, void** data, size_t* size);
void* cdp_update(cdpRecord* instance, void* data, size_t size);
void* cdp_patch(cdpRecord* instance, void* data, size_t size);

cdpRecord* cdp_add(cdpRecord* instance, cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_prepend(cdpRecord* instance, cdpRecord* book, cdpRecord* record);
cdpRecord* cdp_insert(cdpRecord* instance, cdpRecord* book, cdpRecord* record);

cdpRecord* cdp_first(cdpRecord* instance);
cdpRecord* cdp_last(cdpRecord* instance);

cdpRecord* cdp_pop(cdpRecord* instance, bool last);
cdpRecord* cdp_search(cdpRecord* instance, cdpRecord* book, cdpRecord* key);
cdpRecord* cdp_remove(cdpRecord* instance, cdpRecord* book, cdpRecord* record);

