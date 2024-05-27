/*
 *  Copyright (c) 2024 Victor M. Barrientos <firmw.guy@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
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


#include "cdp_process.h"
#include <ctype.h>        // isupper()



extern cdpRecord CDP_ROOT;

cdpRecord* CDP_VOID;
cdpRecord* CDP_TRUE;
cdpRecord* CDP_FALSE;

cdpRecord* TYPE;
cdpRecord* SYSTEM;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* NETWORK;
cdpRecord* TEMP;

cdpRecord* NAME;

struct NID {const char* name; size_t length; cdpID id;};




static bool name_id_find_text(cdpBookEntry* entry, unsigned depth, struct NID* nid) {
    const char* name = cdp_register_read_utf8(entry->record);
    if (entry->record->recData.reg.size == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = entry->record->metadata.id;
        return false;
    }
    return true;
}

cdpID cdp_name_id_add(const char* name, bool borrow) {
    assert(name && *name);
    size_t length = strlen(name);
    CDP_DEBUG(for (unsigned n=0; n<length; n++) {assert(!isupper(name[n]));});

    // Find previous
    struct NID nid = {name, length};
    if (!cdp_book_traverse(NAME, (cdpTraverse)name_id_find_text, &nid, NULL))
        return nid.id;

    // Add new
    return cdp_record_id(cdp_book_add_text(NAME, borrow? CDP_ATTRIB_FACTUAL: 0, CDP_AUTO_ID, borrow, name));
}


cdpRecord* cdp_name_id_text(cdpID id) {
    assert(id < cdp_book_get_auto_id(NAME));
    return cdp_book_find_by_name(NAME, id);
}




static inline cdpRecord* system_initiate_type(cdpID typeID, const char* name, const char* description, uint32_t size) {
    unsigned items = 1;
    if (*description) items++;
    if (size) items++;

    cdpRecord* type = cdp_book_add_dictionary(TYPE, typeID, CDP_STO_CHD_ARRAY, items); {
        if (cdp_record_is_named(type))
            cdp_book_add_static_text(type, CDP_NAME_NAME, name);
        if (*description)
            cdp_book_add_static_text(type, CDP_NAME_DESCRIPTION, description);
        if (size)
            cdp_book_add_uint32(type, CDP_NAME_SIZE, size);
    }

    return type;
}

static bool type_find_by_text(cdpBookEntry* entry, unsigned depth, struct NID* nid) {
    if (cdp_record_is_object(entry->record))
        return true;
    cdpRecord* nameReg = cdp_book_find_by_name(entry->record, CDP_NAME_NAME);
    const char* name = cdp_register_read_utf8(nameReg);
    if (nameReg->recData.reg.size == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = entry->record->metadata.id;
        return false;
    }
    return true;
}

cdpID cdp_type_add(const char* name, const char* description, size_t baseSize) {
    assert(name && *name);
    size_t length = strlen(name);
    CDP_DEBUG(for (unsigned n=0; n<length; n++) {assert(!isupper(name[n]));});

    // Find previous
    struct NID nid = {name, length};
    if (!cdp_book_traverse(TYPE, (cdpTraverse)type_find_by_text, &nid, NULL)) {
        assert(!"Type already present.");  // FixMe.
        return CDP_TYPE_VOID;
    }

    return cdp_record_id(system_initiate_type(CDP_AUTO_ID, name, description, baseSize));
}


cdpRecord* cdp_type(cdpID typeID) {
    assert(typeID < cdp_book_get_auto_id(TYPE));
    return cdp_book_find_by_name(TYPE, typeID);
}




cdpID cdp_object_add(cdpID nameID, cdpID processID, char* description, size_t baseSize) {
    assert((nameID & CDP_NAME_FLAG) && (processID & CDP_NAME_FLAG));

    // Find previous
    if (cdp_book_find_by_name(TYPE, nameID)) {
        assert(!"Object already present.");  // FixMe.
        return CDP_TYPE_VOID;
    }

    cdpRecord* object = system_initiate_type(nameID, NULL, description, baseSize); {
        cdp_book_add_id(object, CDP_NAME_PROCESS, processID);
    }

    return cdp_record_id(object);
}


bool cdp_object_validate(cdpRecord* object) {
    return false;
}




void cdp_system_initiate(void) {
    assert(!SYSTEM);
    cdp_record_system_initiate();


    /* Initiate root book structure.
    */
    TYPE    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_TYPE,    CDP_STO_CHD_ARRAY, CDP_TYPE_COUNT + CDP_OBJECT_COUNT);
    SYSTEM  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_SYSTEM,  CDP_STO_CHD_RED_BLACK_T);
    USER    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_USER,    CDP_STO_CHD_RED_BLACK_T);
    PUBLIC  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_PUBLIC,  CDP_STO_CHD_RED_BLACK_T);
    DATA    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_DATA,    CDP_STO_CHD_RED_BLACK_T);
    NETWORK = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_NETWORK, CDP_STO_CHD_RED_BLACK_T);
    TEMP    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_TEMP,    CDP_STO_CHD_RED_BLACK_T);


    /* Initiate type system. */

    // Abstract types
    cdpRecord* tvoid = system_initiate_type(CDP_TYPE_VOID, "void", "Type for describing nothingness.", 0);
    system_initiate_type(CDP_TYPE_TYPE,         "type",         "Dictionary for describing types.", 0);

    // Book types
    system_initiate_type(CDP_TYPE_BOOK,         "book",         "Generic container of records.", 0);
    system_initiate_type(CDP_TYPE_LINK,         "list",         "Book with records ordered by how they are added/removed", 0);
    system_initiate_type(CDP_TYPE_QUEUE,        "queue",        "List that only removes records from its beginning or adds them to its end.", 0);
    system_initiate_type(CDP_TYPE_STACK,        "stack",        "List that only adds/removes records from its beginning.", 0);
    system_initiate_type(CDP_TYPE_DICTIONARY,   "dictionary",   "Book of records sorted by their unique name.", 0);

    // Register types
    system_initiate_type(CDP_TYPE_REGISTER,     "register",     "Generic record that holds data.", 0);
    cdpRecord* boolean = system_initiate_type(CDP_TYPE_BOOLEAN, "boolean", "Boolean value.", sizeof(uint8_t)); {
        cdpRecord* value = cdp_book_add_dictionary(boolean, CDP_NAME_VALUE, CDP_STO_CHD_ARRAY, 2); {
            CDP_FALSE = cdp_book_add_static_text(value, CDP_AUTO_ID, "false");
            CDP_TRUE  = cdp_book_add_static_text(value, CDP_AUTO_ID, "true");
        }
    }
    system_initiate_type(CDP_TYPE_BYTE,         "byte",         "Unsigned integer number of 8 bits.",   sizeof(uint8_t));
    system_initiate_type(CDP_TYPE_UINT16,       "uint16",       "Unsigned integer number of 16 bits.",  sizeof(uint16_t));
    system_initiate_type(CDP_TYPE_UINT32,       "uint32",       "Unsigned integer number of 32 bits.",  sizeof(uint32_t));
    system_initiate_type(CDP_TYPE_UINT64,       "uint64",       "Unsigned integer number of 64 bits.",  sizeof(uint64_t));
    system_initiate_type(CDP_TYPE_INT16,        "int16",        "Integer number of 16 bits.",           sizeof(int16_t));
    system_initiate_type(CDP_TYPE_INT32,        "int32",        "Integer number of 32 bits.",           sizeof(int32_t));
    system_initiate_type(CDP_TYPE_INT64,        "int64",        "Integer number of 64 bits.",           sizeof(int64_t));
    system_initiate_type(CDP_TYPE_FLOAT32,      "float32",      "Floating point number of 32 bits.",    sizeof(float));
    system_initiate_type(CDP_TYPE_FLOAT64,      "float64",      "Floating point number of 64 bits.",    sizeof(double));
    //
    system_initiate_type(CDP_TYPE_ID,           "id",           "Register with the value of an id (name or type) of records.", sizeof(cdpID));
    cdpRecord* nameid = system_initiate_type(CDP_TYPE_NAME_ID, "name_id", "Id as a text token for creating record paths.", 4); {
        NAME = cdp_book_add_dictionary(nameid, CDP_NAME_VALUE, CDP_STO_CHD_PACKED_QUEUE, CDP_NAME_COUNT);
    }
    system_initiate_type(CDP_TYPE_UTF8,         "utf8",         "Text encoded in UTF8 format.", 0);
    system_initiate_type(CDP_TYPE_PATCH,        "patch",        "Record that can patch another record.", 0);
    system_initiate_type(CDP_TYPE_EXECUTABLE,   "executable",   "Address of a process executable entry point.", sizeof(cdpProcess));

    // Link types
    system_initiate_type(CDP_TYPE_LINK,         "link",         "Record that points to another record.", 0);

    // Finish core types.
    assert(cdp_book_children(TYPE) == CDP_TYPE_COUNT);
    cdp_book_set_auto_id(TYPE, CDP_TYPE_COUNT);

    // Object types
    //system_initiate_type(CDP_OBJECT_OBJECT,     "object",       "Book with records structured and ordered by some user-defined criteria.", 0);
    //system_initiate_type(CDP_OJECT_PROCESS,     "patch",        "Record that can patch another record.", 0);


    /* Initiate name (ID) interning system:
       *** WARNING: this must be done in the same order as the _cdpNameID
                    enumeration in "cdp_record.h". ***
    */
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,            "");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "name");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "value");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "size");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "description");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,           "/");       // The root book.
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "type");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "system");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "user");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "private");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "public");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "data");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "service");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "process");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "network");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "temp");

    assert(cdp_book_get_auto_id(NAME) == CDP_NAME_COUNT);


    /* Initiate global records.
    */
    CDP_VOID = cdp_book_add_boolean(TEMP, CDP_NAME_VOID, 0);
    CDP_VOID->metadata.id = CDP_VOID->metadata.primal = CDP_TYPE_VOID;
    CDP_VOID->metadata.type = tvoid->metadata.id;
}


void cdp_system_shutdown(void) {
    assert(SYSTEM);
    cdp_book_reset(&CDP_ROOT, 64);    // FixMe: maxDepth.
    cdp_record_system_shutdown();
}




cdpID cdp_system_process_add(const char* name, cdpProcess process) {
    assert(SYSTEM && name && *name && process);

    cdpID nameID = cdp_name_id_add_static(name);
    cdpRecord* prev = cdp_book_find_by_name(SYSTEM, nameID);
    if (prev) {
        assert(!prev);    // FixMe: find and report previous.
        return CDP_NAME_VOID;
    }

    cdpRecord* procBook = cdp_book_add_dictionary(SYSTEM, nameID, CDP_TYPE_DICTIONARY, CDP_STO_CHD_RED_BLACK_T); {
        cdpRecord procReg = {0};
        cdp_record_initialize(&procReg, CDP_TYPE_REGISTER, CDP_ATTRIB_FACTUAL, CDP_NAME_PROCESS, CDP_TYPE_EXECUTABLE, true, process, sizeof(cdpProcess));
        cdp_book_add_property(procBook, &procReg);
    }

    return nameID;
}


cdpRecord* cdp_system_process(cdpID nameID) {
    assert(nameID & CDP_NAME_FLAG);
    return cdp_book_find_by_name(SYSTEM, nameID);
}


static bool system_step_traverse_instance(cdpBookEntry* entry, unsigned unused, cdpProcess process) {
    return process(entry->record, CDP_ACTION_STEP);
}

static bool system_step_traverse(cdpBookEntry* entry, unsigned unused, void* unused2) {
    cdpRecord* procReg = cdp_book_get_property(entry->record, CDP_NAME_PROCESS);
    cdpProcess process = cdp_register_read_executable(procReg);
    assert(process);
    return cdp_book_traverse(entry->record, (cdpTraverse)system_step_traverse_instance, process, NULL);
}

bool cdp_system_step(void) {
    assert(SYSTEM);

    return cdp_book_traverse(SYSTEM, system_step_traverse, NULL, NULL);
}

