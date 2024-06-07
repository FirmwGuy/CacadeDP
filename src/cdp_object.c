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
    if (cdp_register_size(nameReg) == nid->length
     && 0 == memcmp(name, nid->name, nid->length)) {
        nid->id = entry->record->metadata.id;
        return false;
    }
    return true;
}

cdpID cdp_type_add(const char* name, const char* description, size_t baseSize) {
    assert(name && *name);
    if (!SYSTEM)  system_initiate();

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


cdpID cdp_type_add_object(const char* name, cdpCallable callable, char* description, size_t baseSize) {
    assert(callable);

    cdpID typeID = cdp_type_add(name, description, baseSize);
    cdpRecord* objType = cdp_type(typeID);
    cdp_book_add_callable(objType, CDP_NAME_CALL, callable);
    CDP_RECORD_SET_ATRIBUTE(objType, CDP_ATTRIB_FACTUAL);

    return typeID;
}




/* Constructs a local "floating" object (not associated with any book).
*/
bool cdp_object_construct(cdpRecord* object, cdpID nameID, cdpID typeID, cdpID storage, uint32_t base) {
    cdpCallable callable = cdp_type_object_callable(typeID);

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_CONSTRUCT, CDP_STO_CHD_ARRAY, 4); {  // Arbitrary numbered ID used here.
        cdp_book_add_uint32(&call, CDP_NAME_BASE, base);
        cdp_book_add_id(&call, CDP_NAME_NAME, nameID);
        cdp_book_add_id(&call, CDP_NAME_STORAGE, storage);
        cdp_book_add_id(&call, CDP_NAME_TYPE, typeID);
    }

    bool done = callable(object, &call);

    //cdpRecord* returned = cdp_book_find_by_name(&call, CDP_RETURN);
    //assert(returned);

    return done;
}


void cdp_object_destruct(cdpRecord* object) {
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_DESTRUCT, CDP_STO_CHD_LINKED_LIST);

    return callable(object, &call);
}


void cdp_object_reference(cdpRecord* object) {
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_REFERENCE, CDP_STO_CHD_LINKED_LIST);

    return callable(object, &call);
}


void cdp_object_free(cdpRecord* object) {
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_FREE, CDP_STO_CHD_LINKED_LIST);

    return callable(object, &call);
}


cdpRecord* cdp_object_append(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    assert(!cdp_record_is_void(record));
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_APPEND, CDP_STO_CHD_ARRAY, 4); {
        cdp_book_add_record(&call, CDP_NAME_RECORD, record);
        if (book)
            cdp_book_add_link(&call, CDP_NAME_BOOK, book);
    }

    bool done = callable(object, &call);
    if (!done)  return NULL;

    cdpRecord* retReg = cdp_book_find_by_name(&call, CDP_NAME_RETURN);
    assert(retReg);
    cdpRecord* newObj = cdp_register_read_executable(retReg);
    assert(newObj);
    return newObj;
}


cdpRecord* cdp_object_prepend(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    assert(!cdp_record_is_void(record));
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_PREPEND, CDP_STO_CHD_ARRAY, 4); {
        cdp_book_add_record(&call, CDP_NAME_RECORD, record);
        if (book)
            cdp_book_add_link(&call, CDP_NAME_BOOK, book);
    }

    bool done = callable(object, &call);
    if (!done)  return NULL;

    cdpRecord* retReg = cdp_book_find_by_name(&call, CDP_NAME_RETURN);
    assert(retReg);
    cdpRecord* newObj = cdp_register_read_executable(retReg);
    assert(newObj);
    return newObj;
}


cdpRecord* cdp_object_insert(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    assert(!cdp_record_is_void(record));
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_INSERT, CDP_STO_CHD_ARRAY, 4); {
        cdp_book_add_record(&call, CDP_NAME_RECORD, record);
        if (book)
            cdp_book_add_link(&call, CDP_NAME_BOOK, book);
    }

    bool done = callable(object, &call);
    if (!done)  return NULL;

    cdpRecord* retReg = cdp_book_find_by_name(&call, CDP_NAME_RETURN);
    assert(retReg);
    cdpRecord* newObj = cdp_register_read_executable(retReg);
    assert(newObj);
    return newObj;
}


bool cdp_object_update(cdpRecord* object, cdpRecord* record, void* data, size_t size) {
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_UPDATE, CDP_STO_CHD_ARRAY, 4); {
        cdp_book_add_link(&call, CDP_NAME_RECORD, record);
        cdp_book_add_register(&call,
                              cdp_record_attributes(record),
                              CDP_NAME_REGISTER,
                              cdp_record_type(record),
                              cdp_register_is_borrowed(record),
                              data, size);
    }
    return callable(object, &call);
}


bool cdp_object_remove(cdpRecord* object, cdpRecord* book, cdpRecord* record) {
    cdpCallable callable = cdp_type_object_callable(cdp_record_type(object));

    cdpRecord call = {0};
    cdp_record_initialize_dictionary(&call, CDP_CALL_REMOVE, CDP_STO_CHD_ARRAY, 4); {
        cdp_book_add_link(&call, CDP_NAME_RECORD, record);
        if (book)
            cdp_book_add_link(&call, CDP_NAME_BOOK, book);
    }
    return callable(object, &call);
}


bool cdp_object_validate(cdpRecord* object) {
    return true;
}




#define system_initiate_type(t, name, desc, size)   type_add_type(t, name, desc, size, 0)

static void system_initiate(void) {
    if (SYSTEM) return;

    cdp_record_system_initiate();


    /* Initiate root book structure.
    */
    TYPE    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_TYPE,    CDP_STO_CHD_ARRAY, CDP_TYPE_COUNT);
    SYSTEM  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_SYSTEM,  CDP_STO_CHD_RED_BLACK_T);
    USER    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_USER,    CDP_STO_CHD_RED_BLACK_T);
    PUBLIC  = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_PUBLIC,  CDP_STO_CHD_RED_BLACK_T);
    DATA    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_DATA,    CDP_STO_CHD_RED_BLACK_T);
    NETWORK = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_NETWORK, CDP_STO_CHD_RED_BLACK_T);
    TEMP    = cdp_book_add_dictionary(&CDP_ROOT, CDP_NAME_TEMP,    CDP_STO_CHD_RED_BLACK_T);


    /* Initiate type system. */
    cdpRecord* type, *value;

    // Abstract types
    system_initiate_type(CDP_TYPE_VOID,           "void",           "Type for describing nothingness.", 0);

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
        NAME = cdp_book_add_dictionary(type, CDP_NAME_VALUE, CDP_STO_CHD_PACKED_QUEUE, cdp_next_pow_of_two(CDP_NAME_COUNT));
    }
    system_initiate_type(CDP_TYPE_UTF8,           "utf8",           "Text encoded in UTF8 format.", 0);
    system_initiate_type(CDP_TYPE_PATCH,          "patch",          "Record that can patch another record.", 0);
    //
    system_initiate_type(CDP_TYPE_CALLABLE,       "callable",       "Address of a callable function.", sizeof(cdpCallable));
    type = system_initiate_type(CDP_TYPE_EVENT,   "event",          "Object event.", sizeof(uint8_t)); {
        value = cdp_book_add_dictionary(type, CDP_NAME_VALUE, CDP_STO_CHD_ARRAY, CDP_CALL_COUNT); {
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
    system_initiate_type(CDP_TYPE_LINK,           "link",           "Record that points to another record.", 0);

    // Structured types
    system_initiate_type(CDP_TYPE_TYPE,           "type",           "Dictionary for describing types.", 0);
    system_initiate_type(CDP_TYPE_OBJECT,         "object",         "Records structured and ordered by event signals.", 0);

    // Finish core types.
    assert(cdp_book_children(TYPE) == CDP_TYPE_COUNT);
    cdp_book_set_auto_id(TYPE, CDP_TYPE_COUNT);


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

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "call");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "return");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "error");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "object");

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
    CDP_VOID->metadata.type = CDP_VOID->metadata.primal = CDP_TYPE_VOID;
    CDP_VOID->metadata.id = CDP_NAME_VOID;
    CDP_RECORD_SET_ATRIBUTE(CDP_VOID, CDP_ATTRIB_FACTUAL);
}




static bool system_startup_traverse(cdpBookEntry* entry, unsigned unused, void* unused2) {
    cdpCallable callable = cdp_book_get_property(entry->record, CDP_NAME_CALL);
    if (callable) {
        cdpRecord call = {0};
        cdp_record_initialize_dictionary(&call, CDP_CALL_STARTUP, CDP_STO_CHD_LINKED_LIST);
        return callable(NULL, &call);
    }
    return true;
}

bool cdp_system_startup(void) {
    assert(cdp_book_children(TYPE));
    return cdp_book_traverse(TYPE, system_startup_traverse, NULL, NULL);
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

