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


#include "cdp_signal.h"




cdpSignal* SIGNAL_CREATE_BOOK;
cdpSignal* SIGNAL_CREATE_REGISTER;
cdpSignal* SIGNAL_DESTROY;
cdpSignal* SIGNAL_RESET;
cdpSignal* SIGNAL_FREE;
cdpSignal* SIGNAL_REFERENCE;
cdpSignal* SIGNAL_COPY;
cdpSignal* SIGNAL_MOVE;
cdpSignal* SIGNAL_LINK;
cdpSignal* SIGNAL_NEXT;
cdpSignal* SIGNAL_PREVIOUS;
cdpSignal* SIGNAL_VALIDATE;
cdpSignal* SIGNAL_SERIALIZE;
cdpSignal* SIGNAL_UNSERIALIZE;
cdpSignal* SIGNAL_TEXTUALIZE;
cdpSignal* SIGNAL_UNTEXTUALIZE;
cdpSignal* SIGNAL_READ;
cdpSignal* SIGNAL_UPDATE;
cdpSignal* SIGNAL_PATCH;
cdpSignal* SIGNAL_ADD;
cdpSignal* SIGNAL_PREPEND;
cdpSignal* SIGNAL_INSERT;
cdpSignal* SIGNAL_FIRST;
cdpSignal* SIGNAL_LAST;
cdpSignal* SIGNAL_POP;
cdpSignal* SIGNAL_SEARCH;
cdpSignal* SIGNAL_REMOVE;



void cdp_signal_initiate(void) {
    extern cdpRecord* NAME;


    /* Initiate signal name IDs:
       *** WARNING: this must be done in the same order as the
                    enumeration in "cdp_signal.h". ***
    */
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
}


void cdp_signal_shutdown(void) {
    cdp_signal_del(SIGNAL_CREATE_BOOK);
    cdp_signal_del(SIGNAL_CREATE_REGISTER);
    cdp_signal_del(SIGNAL_DESTROY);
    cdp_signal_del(SIGNAL_RESET);
    cdp_signal_del(SIGNAL_FREE);
    cdp_signal_del(SIGNAL_REFERENCE);
    cdp_signal_del(SIGNAL_COPY);
    cdp_signal_del(SIGNAL_MOVE);
    cdp_signal_del(SIGNAL_LINK);
    cdp_signal_del(SIGNAL_NEXT);
    cdp_signal_del(SIGNAL_PREVIOUS);
    cdp_signal_del(SIGNAL_VALIDATE);
    cdp_signal_del(SIGNAL_SERIALIZE);
    cdp_signal_del(SIGNAL_UNSERIALIZE);
    cdp_signal_del(SIGNAL_TEXTUALIZE);
    cdp_signal_del(SIGNAL_UNTEXTUALIZE);
    cdp_signal_del(SIGNAL_READ);
    cdp_signal_del(SIGNAL_UPDATE);
    cdp_signal_del(SIGNAL_PATCH);
    cdp_signal_del(SIGNAL_ADD);
    cdp_signal_del(SIGNAL_PREPEND);
    cdp_signal_del(SIGNAL_INSERT);
    cdp_signal_del(SIGNAL_FIRST);
    cdp_signal_del(SIGNAL_LAST);
    cdp_signal_del(SIGNAL_POP);
    cdp_signal_del(SIGNAL_SEARCH);
    cdp_signal_del(SIGNAL_REMOVE);
}




cdpSignal* cdp_signal_new(cdpID nameID, unsigned itemsArg, unsigned itemsRes) {
    CDP_NEW(cdpSignal, signal);
    cdp_record_initialize_dictionary(&signal->argument, nameID, CDP_STO_CHD_ARRAY, itemsArg);
    cdp_record_initialize_dictionary(&signal->result, CDP_NAME_CREATE, CDP_STO_CHD_ARRAY, itemsRes); {
    return signal;
}


void cdp_signal_del(cdpSignal* signal) {
    assert(signal);
    cdp_record_finalize(&signal->argument);
    cdp_record_finalize(&signal->result);
    if (!cdp_record_is_void(&signal->error))
        cdp_record_finalize(&signal->error);
    cdp_free(signal);
}


void cdp_signal_reset(cdpSignal* signal) {
    cdp_book_reset(&signal->argument);
    cdp_book_reset(&signal->result);
    if (!cdp_record_is_void(&signal->error)) {
        cdp_record_finalize(&signal->error);
        CDP_0(&signal->error);
    }
}




/* Executes the associated signal handler for the specified agent instance.
*/
bool cdp_action(cdpRecord* instance, cdpRecord* signal) {
    assert(instance && signal);
    cdpRecord* agent = cdp_agent(cdp_record_agent(instance));
    cdpRecord* actionReg = cdp_book_find_by_name(agent, cdp_record_id(instance));
    cdpAction* action = cdp_register_read_action(actionReg);
    return action(instance, signal);
}




cdpRecord* cdp_create_book(cdpRecord* instance, cdpID nameID, cdpID agentID, unsigned storage, unsigned baseLength) {
    assert(instance  &&  nameID != CDP_NAME_VOID  &&  agentID  &&  storage < CDP_STO_CHD_COUNT);

    if (!SIGNAL_CREATE_BOOK)    // ToDo: store signal in a system queue and pause calling task.
        SIGNAL_CREATE_BOOK = cdp_signal_new(CDP_NAME_CREATE, 4, 1);

    cdp_book_add_id(&SIGNAL_CREATE_BOOK->argument, CDP_NAME_NAME, nameID);
    cdp_book_add_id(&SIGNAL_CREATE_BOOK->argument, CDP_NAME_AGENT, agentID);
    cdp_book_add_id(&SIGNAL_CREATE_BOOK->argument, CDP_NAME_STORAGE, storage);
    if (baseLength)
        cdp_book_add_uint32(&SIGNAL_CREATE_BOOK->argument, CDP_NAME_BASE, base);

    cdpRecord* newBook;
    if (cdp_action(instance, SIGNAL_CREATE_BOOK)) {
        cdpRecord* retReg = cdp_book_find_by_name(&SIGNAL_CREATE_BOOK->result, CDP_NAME_RETURN);
        newBook = cdp_link_read_address(retReg);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_CREATE_BOOK->error));
        newBook = NULL;
    }
    cdp_signal_reset(SIGNAL_CREATE_BOOK);
    return newBook;
}


cdpRecord* cdp_create_register(cdpRecord* instance, cdpID nameID, cdpID agentID, void* data, size_t size) {
    assert(instance  &&  nameID != CDP_NAME_VOID &&  agentID  &&  size);

    if (!SIGNAL_CREATE_REGISTER)    // ToDo: store signal in a system queue and pause calling task.
        SIGNAL_CREATE_REGISTER = cdp_signal_new(CDP_NAME_CREATE, 4, 1);

    cdp_book_add_id(&SIGNAL_CREATE_REGISTER->argument, CDP_NAME_NAME, nameID);
    cdp_book_add_id(&SIGNAL_CREATE_REGISTER->argument, CDP_NAME_AGENT, agentID);
    cdp_book_add_id(&SIGNAL_CREATE_REGISTER->argument, CDP_NAME_SIZE, size);
    if (data)
        cdp_book_add_link(&SIGNAL_CREATE_REGISTER->argument, CDP_NAME_DATA, data);

    cdpRecord* newReg;
    if (cdp_action(instance, SIGNAL_CREATE_REGISTER)) {
        cdpRecord* retReg = cdp_book_find_by_name(&SIGNAL_CREATE_REGISTER->result, CDP_NAME_RETURN);
        newReg = cdp_link_read_address(retReg);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_CREATE_REGISTER->error));
        newReg = NULL;
    }
    cdp_signal_reset(SIGNAL_CREATE_REGISTER);
    return newReg;
}


void cdp_destroy(cdpRecord* instance) {
    if (!SIGNAL_DESTROY)
        SIGNAL_DESTROY = cdp_signal_new(CDP_NAME_DESTROY, 1, 1);
    cdp_action(instance, SIGNAL_DESTROY);
    //cdp_signal_reset(SIGNAL_DESTROY);
}


void cdp_reset(cdpRecord* instance) {
    if (!SIGNAL_RESET)
        SIGNAL_RESET = cdp_signal_new(CDP_NAME_RESET, 1, 1);
    cdp_action(instance, SIGNAL_RESET);
    //cdp_signal_reset(SIGNAL_RESET);
}


void cdp_free(cdpRecord* instance) {
    if (!SIGNAL_FREE)
        SIGNAL_FREE = cdp_signal_new(CDP_NAME_FREE, 1, 1);
    cdp_action(instance, SIGNAL_FREE);
    //cdp_signal_reset(SIGNAL_FREE);
}


void cdp_reference(cdpRecord* instance) {
    if (!SIGNAL_REFERENCE)
        SIGNAL_REFERENCE = cdp_signal_new(CDP_NAME_REFERENCE, 1, 1);
    cdp_action(instance, SIGNAL_REFERENCE);
    //cdp_signal_reset(SIGNAL_REFERENCE);
}


cdpRecord* cdp_copy(cdpRecord* instance, cdpRecord* newParent, cdpID nameID) {
    assert(instance && cdp_record_is_book(newParent) && nameID != CDP_NAME_VOID);

    if (!SIGNAL_COPY)
        SIGNAL_COPY = cdp_signal_new(CDP_NAME_COPY, 2, 1);

    cdp_book_add_link(&SIGNAL_COPY->argument, CDP_NAME_PARENT, newParent);
    cdp_book_add_id(&SIGNAL_COPY->argument, CDP_NAME_NAME, nameID);

    cdpRecord* newRec;
    if (cdp_action(instance, SIGNAL_COPY)) {
        cdpRecord* retLink = cdp_book_find_by_name(&SIGNAL_COPY->result, CDP_NAME_RETURN);
        newRec = cdp_link_read_address(retLink);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_COPY->error));
        newRec = NULL;
    }
    cdp_signal_reset(SIGNAL_COPY);
    return newRec;
}


cdpRecord* cdp_move(cdpRecord* instance, cdpRecord* newParent, cdpID nameID) {
    assert(instance && cdp_record_is_book(newParent) && nameID != CDP_NAME_VOID);

    if (!SIGNAL_MOVE)
        SIGNAL_MOVE = cdp_signal_new(CDP_NAME_MOVE, 2, 1);

    cdp_book_add_link(&SIGNAL_MOVE->argument, CDP_NAME_PARENT, newParent);
    cdp_book_add_id(&SIGNAL_MOVE->argument, CDP_NAME_NAME, nameID);

    cdpRecord* newRec;
    if (cdp_action(instance, SIGNAL_MOVE)) {
        cdpRecord* retLink = cdp_book_find_by_name(&SIGNAL_MOVE->result, CDP_NAME_RETURN);
        newRec = cdp_link_read_address(retLink);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_MOVE->error));
        newRec = NULL;
    }
    cdp_signal_reset(SIGNAL_MOVE);
    return newRec;
}



cdpRecord* cdp_link(cdpRecord* instance, cdpRecord* newParent, cdpID nameID) {
    assert(instance && cdp_record_is_book(newParent) && nameID != CDP_NAME_VOID);

    if (!SIGNAL_LINK)
        SIGNAL_LINK = cdp_signal_new(CDP_NAME_LINK, 2, 1);

    cdp_book_add_link(&SIGNAL_LINK->argument, CDP_NAME_PARENT, newParent);
    cdp_book_add_id(&SIGNAL_LINK->argument, CDP_NAME_NAME, nameID);

    cdpRecord* newRec;
    if (cdp_action(instance, SIGNAL_LINK)) {
        cdpRecord* retLink = cdp_book_find_by_name(&SIGNAL_LINK->result, CDP_NAME_RETURN);
        newRec = cdp_link_read_address(retLink);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_LINK->error));
        newRec = NULL;
    }
    cdp_signal_reset(SIGNAL_LINK);
    return newRec;
}


cdpRecord* cdp_next(cdpRecord* instance) {
    assert(instance);

    if (!SIGNAL_NEXT)
        SIGNAL_NEXT = cdp_signal_new(CDP_NAME_NEXT, 1, 1);

    cdpRecord* nextRec;
    if (cdp_action(instance, SIGNAL_NEXT)) {
        cdpRecord* retLink = cdp_book_find_by_name(&SIGNAL_NEXT->result, CDP_NAME_RETURN);
        nextRec = cdp_link_read_address(retLink);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_NEXT->error));
        nextRec = NULL;
    }
    //cdp_signal_reset(SIGNAL_NEXT);
    return nextRec;
}


cdpRecord* cdp_previous(cdpRecord* instance) {
    assert(instance);

    if (!SIGNAL_PREVIOUS)
        SIGNAL_PREVIOUS = cdp_signal_new(CDP_NAME_PREVIOUS, 1, 1);

    cdpRecord* prevRec;
    if (cdp_action(instance, SIGNAL_PREVIOUS)) {
        cdpRecord* retLink = cdp_book_find_by_name(&SIGNAL_PREVIOUS->result, CDP_NAME_RETURN);
        prevRec = cdp_link_read_address(retLink);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_PREVIOUS->error));
        prevRec = NULL;
    }
    //cdp_signal_reset(SIGNAL_PREVIOUS);
    return prevRec;
}


bool cdp_validate(cdpRecord* instance) {
    if (!SIGNAL_VALIDATE)
        SIGNAL_VALIDATE = cdp_signal_new(CDP_NAME_VALIDATE, 1, 1);
    cdp_action(instance, SIGNAL_VALIDATE);
    cdpRecord* boolReg = cdp_book_find_by_name(&SIGNAL_VALIDATE->result, CDP_NAME_RETURN);
    bool valid = cdp_register_read_bool(boolReg);
    cdp_signal_reset(SIGNAL_VALIDATE);
    return valid;
}


size_t cdp_serialize(cdpRecord* instance, void* data, size_t size) {
    assert(instance && data && size);

    if (!SIGNAL_SERIALIZE)
        SIGNAL_SERIALIZE = cdp_signal_new(CDP_NAME_SERIALIZE, 1, 1);

    cdp_book_add_static_binary(&SIGNAL_SERIALIZE->argument, CDP_NAME_DATA, data, size);

    size_t serializedSize;
    if (cdp_action(instance, SIGNAL_SERIALIZE)) {
        cdpRecord* serializedReg = cdp_book_find_by_name(&SIGNAL_SERIALIZE->result, CDP_NAME_RETURN);
        serializedSize = cdp_register_size(serializedReg);
    } else {
        // ToDo: report error.
        assert(cdp_record_is_book(&SIGNAL_SERIALIZE->error));
        serializedSize = NULL;
    }
    cdp_signal_reset(SIGNAL_SERIALIZE);
    return serializedSize;
}


bool cdp_unserialize(cdpRecord* instance, void* data, size_t size) {
    return true;
}


bool cdp_textualize(cdpRecord* instance, char** data, size_t* length) {
    return true;
}


bool cdp_untextualize(cdpRecord* instance, char* data, size_t length) {
    return true;
}


void* cdp_read(cdpRecord* instance, void** data, size_t* size) {
    return NULL;
}


void* cdp_update(cdpRecord* instance, void* data, size_t size) {
    return NULL;
}


void* cdp_patch(cdpRecord* instance, void* data, size_t size) {
    return NULL;
}


cdpRecord* cdp_add(cdpRecord* instance, cdpRecord* book, cdpRecord* record) {
    return NULL;
}


cdpRecord* cdp_prepend(cdpRecord* instance, cdpRecord* book, cdpRecord* record) {
    return NULL;
}


cdpRecord* cdp_insert(cdpRecord* instance, cdpRecord* book, cdpRecord* record) {
    return NULL;
}


cdpRecord* cdp_first(cdpRecord* instance) {
    return NULL;
}


cdpRecord* cdp_last(cdpRecord* instance) {
    return NULL;
}


cdpRecord* cdp_pop(cdpRecord* instance, bool last) {
    return NULL;
}


cdpRecord* cdp_search(cdpRecord* instance, cdpRecord* book, cdpRecord* key) {
    return NULL;
}


cdpRecord* cdp_remove(cdpRecord* instance, cdpRecord* book, cdpRecord* record) {
    return NULL;
}

