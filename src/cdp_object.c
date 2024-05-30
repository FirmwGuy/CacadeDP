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


#include "cdp_object.h"
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




static bool name_id_traverse_find_text(cdpBookEntry* entry, unsigned depth, struct NID* nid) {
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




static inline cdpRecord* type_add_type(cdpID typeID, const char* name, const char* description, uint32_t size, cdpID nameID) {
    unsigned items = 1;
    if (*description) items++;
    if (size) items++;

    cdpRecord* type = cdp_book_add_dictionary(TYPE, typeID, CDP_STO_CHD_ARRAY, items); {
        if (name)
            cdp_book_add_static_text(type, CDP_NAME_NAME, name);
        else
            cdp_book_add_id(type, CDP_NAME_NAME, nameID);

        if (*description)
            cdp_book_add_static_text(type, CDP_NAME_DESCRIPTION, description);

        if (size)
            cdp_book_add_uint32(type, CDP_NAME_SIZE, size);
    }

    return type;
}

static bool type_traverse_find_by_text(cdpBookEntry* entry, unsigned depth, struct NID* nid) {
    cdpRecord* nameReg = cdp_book_find_by_name(entry->record, CDP_NAME_NAME);
    if (nameReg.metadata.type == CDP_TYPE_ID)
        nameReg = cdp_name_id_text(cdp_register_read_id(nameReg));
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
    for (unsigned n=0; n<length; n++) {
        if (isupper(name[n])) {
            assert(isupper(name[n]));
            return CDP_TYPE_VOID;
        }
    }
    cdpID nameID = cdp_name_id_add_static(name);

    // Find previous
    struct NID nid = {name, length};
    bool found = !cdp_book_traverse(TYPE, (cdpTraverse)type_traverse_find_by_text, &nid, NULL);
    if (found) {
        assert(!found);     // FixMe.
        return CDP_TYPE_VOID;
    }

    return cdp_record_id(type_add_type(CDP_AUTO_ID, NULL, description, baseSize, nameID));
}


cdpRecord* cdp_type(cdpID typeID) {
    assert(!(typeID & CDP_OBJECT_FLAG) && (typeID < cdp_book_get_auto_id(TYPE)));
    //return cdp_book_find_by_name(TYPE, typeID);
    return cdp_book_find_by_position(TYPE, typeID);     // FixMe: check if entry is disabled.
}




cdpID cdp_object_type_add(const char* name, cdpObject object, char* description, size_t baseSize) {
    assert(name && *name && object);

    if (!SYSTEM)  cdp_system_initiate();

    cdpID i = cdp_type_add(name, description, baseSize);

    cdpRecord* object = system_initiate_type(nameID, NULL, description, baseSize); {
        cdp_book_add_object(object, CDP_NAME_OBJECT, object);
        CDP_RECORD_SET_ATRIBUTE(object, CDP_ATTRIB_FACTUAL);
    }

    return cdp_record_id(object);
}


void cdp_object_construct(cdpRecord* object, primal, attrib, cdpID nameID, cdpID typeID, cdpID storage, uint32_t base) {
    cdpRecord op = {0};
    cdp_record_initialize_dictionary(&op, CDP_NAME_MESSAGE, CDP_STO_CHD_ARRAY, 4);

    cdp_record_initialize(object, primal, attrib, nameID, typeID, );
    cdp_book_add_id(&op, CDP_NAME_EVENT, CDP_SIGNAL_CONSTRUCT);
    cdp_book_add_id(&op, CDP_NAME_EVENT, CDP_SIGNAL_CONSTRUCT);
    cdp_object_send(proc, CDP_SIGNAL, &op);
}


void cdp_object_destruct(cdpRecord* object) {
    assert(cdp_record_is_object(object));

}


void cdp_object_reference(cdpRecord* object) {
    assert(cdp_record_is_object(object));

}


void cdp_object_free(cdpRecord* object) {
    assert(cdp_record_is_object(object));
}


cdpRecord* cdp_object_append(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    assert(cdp_record_is_object(object));

}


cdpRecord* cdp_object_prepend(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    assert(cdp_record_is_object(object));

}


cdpRecord* cdp_object_insert(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    assert(cdp_record_is_object(object));

}


cdpRecord* cdp_object_update(cdpRecord* object, cdpRecord* book, cdpRecord* record, void* data, size_t size) {
    assert(cdp_record_is_object(object));

}

cdpRecord* cdp_object_remove(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    assert(cdp_record_is_object(object));

}


bool cdp_object_validate(cdpRecord* object) {
    return false;
}




#define system_initiate_type(t, name, desc, size)   type_add_type(t, name, desc, size, 0)

static void cdp_system_initiate(void) {
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
    cdpRecord* type, *tvoid, *value;

    // Abstract types
    tvoid = system_initiate_type(CDP_TYPE_VOID,   "void",           "Type for describing nothingness.", 0);
    system_initiate_type(CDP_TYPE_TYPE,           "type",           "Dictionary for describing types.", 0);

    // Book types
    system_initiate_type(CDP_TYPE_BOOK,           "book",           "Generic container of records.", 0);
    system_initiate_type(CDP_TYPE_LINK,           "list",           "Book with records ordered by how they are added/removed", 0);
    system_initiate_type(CDP_TYPE_QUEUE,          "queue",          "List that only removes records from its beginning or adds them to its end.", 0);
    system_initiate_type(CDP_TYPE_STACK,          "stack",          "List that only adds/removes records from its beginning.", 0);
    system_initiate_type(CDP_TYPE_DICTIONARY,     "dictionary",     "Book of records sorted by their unique name.", 0);

    // Register types
    system_initiate_type(CDP_TYPE_REGISTER,       "register",       "Generic record that holds data.", 0);
    type = system_initiate_type(CDP_TYPE_BOOLEAN, "boolean",        "Boolean value.", sizeof(uint8_t)); {
        value = cdp_book_add_dictionary(type, CDP_NAME_VALUE, CDP_STO_CHD_ARRAY, CDP_ID_BOOL_COUNT); {
            cdp_book_add_static_text(value, CDP_BOOLEAN_FALSE,   "false");
            cdp_book_add_static_text(value, CDP_BOOLEAN_TRUE,    "true");

            assert(cdp_book_children(value) == CDP_BOOLEAN_COUNT);
            cdp_book_set_auto_id(value, CDP_BOOLEAN_COUNT);
    }   }
    system_initiate_type(CDP_TYPE_BYTE,           "byte",           "Unsigned integer number of 8 bits.",   sizeof(uint8_t));
    system_initiate_type(CDP_TYPE_UINT16,         "uint16",         "Unsigned integer number of 16 bits.",  sizeof(uint16_t));
    system_initiate_type(CDP_TYPE_UINT32,         "uint32",         "Unsigned integer number of 32 bits.",  sizeof(uint32_t));
    system_initiate_type(CDP_TYPE_UINT64,         "uint64",         "Unsigned integer number of 64 bits.",  sizeof(uint64_t));
    system_initiate_type(CDP_TYPE_INT16,          "int16",          "Integer number of 16 bits.",           sizeof(int16_t));
    system_initiate_type(CDP_TYPE_INT32,          "int32",          "Integer number of 32 bits.",           sizeof(int32_t));
    system_initiate_type(CDP_TYPE_INT64,          "int64",          "Integer number of 64 bits.",           sizeof(int64_t));
    system_initiate_type(CDP_TYPE_FLOAT32,        "float32",        "Floating point number of 32 bits.",    sizeof(float));
    system_initiate_type(CDP_TYPE_FLOAT64,        "float64",        "Floating point number of 64 bits.",    sizeof(double));
    //
    system_initiate_type(CDP_TYPE_ID,             "id",             "Register with the value of an id (name or type) of records.", sizeof(cdpID));
    type = system_initiate_type(CDP_TYPE_NAME_ID, "name_id",        "Id as a text token for creating record paths.", 4); {    // FixMe: variant base size for UTF8?
        NAME = cdp_book_add_dictionary(type, CDP_NAME_VALUE, CDP_STO_CHD_PACKED_QUEUE, CDP_NAME_COUNT);
    }
    system_initiate_type(CDP_TYPE_UTF8,           "utf8",           "Text encoded in UTF8 format.", 0);
    system_initiate_type(CDP_TYPE_PATCH,          "patch",          "Record that can patch another record.", 0);
    //
    system_initiate_type(CDP_TYPE_EXECUTABLE,     "executable",     "Address of a object executable entry point.", sizeof(cdpObject));
    type = system_initiate_type(CDP_TYPE_EVENT,   "event",          "Object event.", sizeof(uint8_t)); {
        value = cdp_book_add_dictionary(type, CDP_NAME_VALUE, CDP_STO_CHD_ARRAY, CDP_SIGNAL_COUNT); {
            cdp_book_add_static_text(value, CDP_SIGNAL_CONSTRUCT, "construct");
            cdp_book_add_static_text(value, CDP_SIGNAL_DESTRUCT,  "destruct");
            cdp_book_add_static_text(value, CDP_SIGNAL_REFERENCE, "reference");
            cdp_book_add_static_text(value, CDP_SIGNAL_FREE,      "free");
            cdp_book_add_static_text(value, CDP_SIGNAL_APPEND,    "append");
            cdp_book_add_static_text(value, CDP_SIGNAL_PREPEND,   "prepend");
            cdp_book_add_static_text(value, CDP_SIGNAL_INSERT,    "insert");
            cdp_book_add_static_text(value, CDP_SIGNAL_SORT,      "sort");
            cdp_book_add_static_text(value, CDP_SIGNAL_COPY,      "copy");
            cdp_book_add_static_text(value, CDP_SIGNAL_MOVE,      "move");
            cdp_book_add_static_text(value, CDP_SIGNAL_LINK,      "link");

            assert(cdp_book_children(value) == CDP_SIGNAL_COUNT);
            cdp_book_set_auto_id(value, CDP_SIGNAL_COUNT);
    }   }

    // Link types
    system_initiate_type(CDP_TYPE_LINK,           "link",           "Record that points to another record.", 0);

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
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,            "");     // Void text.
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "name");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "value");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "size");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "description");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "object");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "private");
    //cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "service");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,           "/");     // The root book.
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "type");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "system");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "user");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "public");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "data");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "network");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "temp");

    assert(cdp_book_get_auto_id(NAME) == CDP_NAME_COUNT);


    /* Initiate global records.
    */
    CDP_VOID = cdp_book_add_boolean(TEMP, CDP_NAME_VOID, 0);
    CDP_VOID->metadata.id = CDP_VOID->metadata.primal = CDP_TYPE_VOID;
    CDP_VOID->metadata.type = tvoid->metadata.id;
}




static bool system_step_traverse_instance(cdpBookEntry* entry, unsigned unused, cdpObject object) {
    return object(entry->record, CDP_ACTION_STEP);
}

static bool system_step_traverse(cdpBookEntry* entry, unsigned unused, void* unused2) {
    cdpRecord* procReg = cdp_book_get_property(entry->record, CDP_NAME_PROCESS);
    cdpObject object = cdp_register_read_executable(procReg);
    assert(object);
    return cdp_book_traverse(entry->record, (cdpTraverse)system_step_traverse_instance, object, NULL);
}

bool cdp_system_step(void) {
    assert(SYSTEM);
    return cdp_book_traverse(SYSTEM, system_step_traverse, NULL, NULL);
}


void cdp_system_shutdown(void) {
    assert(SYSTEM);
    cdp_book_reset(&CDP_ROOT, 64);    // FixMe: maxDepth.
    cdp_record_system_shutdown();
}

