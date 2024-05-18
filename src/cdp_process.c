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




cdpRecord* NONE;
cdpRecord* TYPE;
cdpRecord* NAME;

extern cdpRecord ROOT;

cdpRecord* SYSTEM;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* SERVICE;
cdpRecord* PROCESS;
cdpRecord* NETWORK;

cdpRecord* TEMP;



struct NIF {const char* name; size_t length; size_t index;}

static bool name_id_find(cdpBookEntry* entry, unsigned depth, struct NIF* nif) {
    name = cdp_register_read(entry->record, 0, NUll, NUll);
    if (entry->record->recData.reg.size == nif->length
     && 0 == memcmp(name, nif->name, nif->length)) {
        nif->index = entry.position;
        return false;
    }
    return true;
}

cdpID cdp_name_id_add(const char* name, bool borrow) {
    assert(name && *name);
    size_t length = strlen(name);
    CDP_DEBUG(for (unsigned n=0; n<length; n++) {assert(!isupper(name[n]));})

    // Find previous
    struct NIF nif = {name, length, 0};
    if (!cdp_book_traverse(NAME, name_id_find, &nif)) {
        return nif.index;
    }

    // Add new
    cdpID id = cdp_book_children(NAME);
    cdp_record_add_register(NAME, CDP_NAME_VALUE, CDP_TYPE_UTF8, borrow, name, length);
    return id;
}


cdpRecord* cdp_name_id_text(cdpID id) {
    assert(id < cdp_book_children(NAME));
    return cdp_book_find_by_position(NAME, id);
}




unsigned cdp_type_add(cdpID id, size_t baseSize) {
    assert(cdp_book_children(TYPE) < CDP_TYPE_MAX_ID);

    // Add factory type descriptions

    // with parent types and minimum fields

    // for each field there must be a dictionary of allowed type names.

    return 0;
}


cdpRecord* cdp_type(unsigned type) {
    assert(type < cdp_book_children(TYPE));
    return cdp_book_find_by_position(TYPE, type);
}


bool cdp_type_validate(cdpRecord* record) {

    // Check minimal fields and types from description

    // Flag it as valid!

    return true;
}



static inline cdpRecord* system_initiate_type(cdpRecord* t, const char* name, const char* description, uint32_t size) {
    unsigned items = 1;
    if (*description) items++;
    if (size) items++;

    /*  /type/ (catalog)
            type (type)/
                name (utf8)
                description (utf8)
                size (uint32)
    */
    cdpRecord* type = cdp_record_add_dictionary(t, CDP_NAME_TYPE, CDP_TYPE_TYPE, CDP_STO_CHD_ARRAY, NULL, NULL, items); {
        cdp_record_add_text(t, CDP_NAME_NAME, name);
        if (*description)
            cdp_record_add_static_text(t, CDP_NAME_DESCRIPTION, description);
        if (size)
            cdp_record_add_uint32(t, CDP_NAME_SIZE, size);
    }

    return type;
}


void cdp_system_initiate(void) {
    assert(!TYPE);
    cdp_record_system_initiate();

    TYPE = cdp_record_add_book(&ROOT, CDP_NAME_TYPE, CDP_TYPE_LOG, CDP_STO_CHD_ARRAY, CDP_TYPE_COUNT); {
        /*
           Each type must be entered in the exact same order of the _CDP_TYPE
           enumeration (since each type is just an index to the TYPE collection).

           *** WARNING: Please keep it in sync with cdp_process.h! ***
        */
        NONE = system_initiate_type(TYPE, "none",   "A type for describing nothingness.", 0);
        system_initiate_type(TYPE, "type",          "Dictionary for describing types.", 0);

        // Book types
        system_initiate_type(TYPE, "book",          "Generic container of records.", 0);
        system_initiate_type(TYPE, "dictionary",    "Book of records sorted by their unique name.", 0);
        system_initiate_type(TYPE, "catalog",       "Book with records ordered by some user-defined criteria.", 0);
        system_initiate_type(TYPE, "list",          "Book with records oredered by how they are added/removed", 0);
        system_initiate_type(TYPE, "set",           "List with records of unique value.", 0);
        system_initiate_type(TYPE, "queue",         "List that only removes records from its beginning or adds them to its end.", 0);
        system_initiate_type(TYPE, "chronicle",     "Book that never removes records.", 0);
        system_initiate_type(TYPE, "encyclopedia",  "Dictionary that never removes records.", 0);
        system_initiate_type(TYPE, "compendium",    "Catalog that never removes records.", 0);
        system_initiate_type(TYPE, "collection",    "Set that never removes records.", 0);
        system_initiate_type(TYPE, "log",           "Queue that never removes records.", 0);

        // Register types
        system_initiate_type(TYPE, "register",      "Generic record that holds data.", 0);
        system_initiate_type(TYPE, "patch",         "Record that can patch other record.", 0);
        cdpRecord* nameid = system_initiate_type(TYPE, "nameid", "Id of text token for creating record paths.", 4); {
            NAME = cdp_record_add_book(nameid, CDP_NAME_VALUE, CDP_TYPE_LOG, CDP_STO_CHD_PACKED_QUEUE, CDP_NAME_COUNT); {
        }
        system_initiate_type(TYPE, "name",          "Register whose only data is its own id.", 0);
        system_initiate_type(TYPE, "utf8",          "Text encoded in UTF8.", 0);

        cdpRecord* boolean = system_initiate_type(TYPE, "boolean", "Boolean value.", 1); {
            cdpRecord* value = cdp_record_add_book(boolean, CDP_NAME_VALUE, CDP_TYPE_SET, CDP_STO_CHD_ARRAY, 2); {
                cdp_record_add_text(value, CDP_NAME_VALUE, "false");
                cdp_record_add_text(value, CDP_NAME_VALUE, "true");
            }
        }
        system_initiate_type(TYPE, "byte",          "Unsigned integer number of 8 bits.",   sizeof(uint8_t));
        system_initiate_type(TYPE, "uint16",        "Unsigned integer number of 16 bits.",  sizeof(uint16_t));
        system_initiate_type(TYPE, "uint32",        "Unsigned integer number of 32 bits.",  sizeof(uint32_t));
        system_initiate_type(TYPE, "uint64",        "Unsigned integer number of 64 bits.",  sizeof(uint64_t));
        system_initiate_type(TYPE, "int16",         "Integer number of 16 bits.",           sizeof(int16_t));
        system_initiate_type(TYPE, "int32"          "Integer number of 32 bits.",           sizeof(int32_t));
        system_initiate_type(TYPE, "int64"          "Integer number of 64 bits.",           sizeof(int64_t));
        system_initiate_type(TYPE, "float32"        "Floating point number of 32 bits.",    sizeof(float));
        system_initiate_type(TYPE, "float64"        "Floating point number of 64 bits.",    sizeof(double));


        /*
           Each name must be entered in the exact same order of the _CDP_NAME_ID
           enumeration (since each id is just an index to the value collection).

           *** WARNING: Please keep it in sync with cdp_process.h! ***
        */
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "");

        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "name");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "value");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "size");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "description");


        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "/");         // Root directory.
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "type");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "system");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "user");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "private");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "public");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "data");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "service");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "process");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "network");
        cdp_record_add_static_text(NAME, CDP_NAME_VALUE, "temp");
    }

    SYSTEM  = cdp_record_add_dictionary(&ROOT, CDP_NAME_SYSTEM,   CDP_TYPE_DICTIONARY,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);
    USER    = cdp_record_add_dictionary(&ROOT, CDP_NAME_USER,     CDP_TYPE_DICTIONARY,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);
    PUBLIC  = cdp_record_add_dictionary(&ROOT, CDP_NAME_PUBLIC,   CDP_TYPE_DICTIONARY,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);
    DATA    = cdp_record_add_dictionary(&ROOT, CDP_NAME_DATA,     CDP_TYPE_DICTIONARY,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);
    SERVICE = cdp_record_add_dictionary(&ROOT, CDP_NAME_SERVICE,  CDP_TYPE_DICTIONARY,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);
    PROCESS = cdp_record_add_dictionary(&ROOT, CDP_NAME_PROCESS,  CDP_TYPE_DICTIONARY,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);
    NETWORK = cdp_record_add_dictionary(&ROOT, CDP_NAME_NETWORK,  CDP_TYPE_DICTIONARY,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);

    TEMP    = cdp_record_add_dictionary(&ROOT, CDP_NAME_TEMP,     CDP_TYPE_BOOK,        CDP_STO_CHD_LINKED_LIST);
}


bool cdp_system_step(void) {
    assert(TYPE);

    return true;
}



void cdp_system_shutdown(void) {
    assert(TYPE);
    cdp_book_reset(&ROOT);
    cdp_record_system_shutdown();
}




cdpID cdp_process_load( const char* name,
                            cdpCreate   create,
                            cdpInstance step,
                            cdpInstance destroy,
                            cdpInstance save,
                            cdpInstance restore) {
    assert(name && *name && create && step);
    static cdpID createNID, stepNID, destroyNID, callbackTID;

    if (!createNID) {
        cdp_system_initiate();
        callbackTID = cdp_system_enter_type(cdp_name_id_add_static("callback"),
                                            "CDP callback address.",
                                            sizeof(void*));
        createNID  = cdp_name_id_add_static("create");
        stepNID    = cdp_name_id_add_static("step");
        destroyNID = cdp_name_id_add_static("destroy");
    }
    cdpID id = cdp_name_id_add_static(name);

    // FixMe: find and report previous.

    cdpRecord* process = cdp_record_add_dictionary(PROCESS, id, CDP_TYPE_DICTIONARY, CDP_STO_CHD_ARRAY, NULL, NULL, 8); {
        cdp_record_add_register(process, createNID, callbackTID, create, sizeof(create));
        cdp_record_add_register(process, stepNID,   callbackTID, step,   sizeof(step));
        if (destroy)
            cdp_record_add_register(process, destroyNID, callbackTID, destroy, sizeof(destroy));
    }

    return id;
}


bool cdp_process_instance_creation_service(void) {
    // look for instance request

    // create instances

    return true;
}


cdpRecord* cdp_process_instantiate(cdpID processNID, ...) {

    return NULL;
}

