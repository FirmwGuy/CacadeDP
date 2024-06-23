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
#include "cdp_action.h"




cdpSignal* SIGNAL_INITIATE_BOOK;
cdpSignal* SIGNAL_INITIATE_REGISTER;
cdpSignal* SIGNAL_INITIATE_LINK;
cdpSignal* SIGNAL_FINALIZE;
cdpSignal* SIGNAL_RESET;
cdpSignal* SIGNAL_REFERENCE;
cdpSignal* SIGNAL_UNREFERENCE;
cdpSignal* SIGNAL_NEXT;
cdpSignal* SIGNAL_PREVIOUS;
cdpSignal* SIGNAL_VALIDATE;
cdpSignal* SIGNAL_REMOVE;

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
cdpSignal* SIGNAL_TAKE;
cdpSignal* SIGNAL_POP;
cdpSignal* SIGNAL_SEARCH;
cdpSignal* SIGNAL_LINK;
cdpSignal* SIGNAL_SHADOW;
cdpSignal* SIGNAL_CLONE;
cdpSignal* SIGNAL_MOVE;



void cdp_system_initiate_signals(void) {
    extern cdpRecord* NAME;


    /* Initiate signal name IDs:
       *** WARNING: this must be done in the same order as the
                    enumeration in "cdp_signal.h". ***
    */
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "startup");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "shutdown");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "connect");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "disconnect");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "initiate");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "finalize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "reset");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "reference");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "unreference");
    //
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "link");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "shadow");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "clone");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "move");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "remove");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "next");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "previous");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,    "validate");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "serialize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID, "unserialize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "textualize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,"untextualize");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "read");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "update");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "patch");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,         "add");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "prepend");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "insert");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,       "first");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "last");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,        "take");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,         "pop");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,      "search");
}


void cdp_system_finalize_signals(void) {
    cdp_signal_del(SIGNAL_INITIATE_BOOK);
    cdp_signal_del(SIGNAL_INITIATE_REGISTER);
    cdp_signal_del(SIGNAL_INITIATE_LINK);
    cdp_signal_del(SIGNAL_FINALIZE);
    cdp_signal_del(SIGNAL_RESET);
    cdp_signal_del(SIGNAL_REFERENCE);
    cdp_signal_del(SIGNAL_UNREFERENCE);
    cdp_signal_del(SIGNAL_NEXT);
    cdp_signal_del(SIGNAL_PREVIOUS);
    cdp_signal_del(SIGNAL_VALIDATE);
    cdp_signal_del(SIGNAL_REMOVE);

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
    cdp_signal_del(SIGNAL_TAKE);
    cdp_signal_del(SIGNAL_POP);
    cdp_signal_del(SIGNAL_SEARCH);
    cdp_signal_del(SIGNAL_LINK);
    cdp_signal_del(SIGNAL_SHADOW);
    cdp_signal_del(SIGNAL_CLONE);
    cdp_signal_del(SIGNAL_MOVE);
}




cdpSignal* cdp_signal_new(cdpID nameID, unsigned itemsArg, unsigned itemsRes) {
    assert(itemsArg || itemsRes);

    CDP_NEW(cdpSignal, signal);
    signal->nameID = nameID;
    if (itemsArg)
        cdp_record_initialize_dictionary(&signal->input, nameID, CDP_STO_CHD_ARRAY, itemsArg);
    if (itemsRes)
        cdp_record_initialize_dictionary(&signal->output, nameID, CDP_STO_CHD_ARRAY, itemsRes);

    return signal;
}


void cdp_signal_del(cdpSignal* signal) {
    assert(signal);
    cdp_record_finalize(&signal->input);
    cdp_record_finalize(&signal->output);
    if (!cdp_record_is_void(&signal->condition))
        cdp_record_finalize(&signal->condition);
    cdp_free(signal);
}


void cdp_signal_reset(cdpSignal* signal) {
    if (cdp_record_is_book(&signal->input))
        cdp_book_reset(&signal->input);
    if (cdp_record_is_book(&signal->output))
        cdp_book_reset(&signal->output);
    if (!cdp_record_is_void(&signal->condition)) {
        cdp_record_finalize(&signal->condition);
        CDP_0(&signal->condition);
    }
}




#define signaler_start(name, signal, inArgs, outArgs)                          \
    assert(instance);                                                          \
    if (!signal)                                                               \
        signal = cdp_signal_new(name, inArgs, outArgs)

#define signaler_action(signal, result, get_res, ...)                          \
    if (cdp_system_does_action(instance, signal)) {                            \
        __VA_ARGS__;                                                           \
        result = get_res;                                                      \
    } else {                                                                   \
        /* ToDo: report error. */                                              \
        assert(cdp_record_is_book(&signal->condition));                        \
        result = 0;                                                            \
    }                                                                          \
    cdp_signal_reset(signal)

#define signaler_return(type, signal, read_op)                                 \
    type result;                                                               \
    signaler_action(  signal,                                                  \
                      result,                                                  \
                      read_op(o),                                              \
                      cdpRecord* o = cdp_book_find_by_name( &signal->output,   \
                                                            CDP_NAME_OUTPUT ));\
    return result




bool cdp_initiate_book(cdpRecord* instance, cdpID nameID, cdpID agentID, unsigned storage, unsigned baseLength) {
    assert(nameID != CDP_NAME_VOID  &&  agentID  &&  storage < CDP_STO_CHD_COUNT);
    signaler_start(CDP_NAME_INITIATE, SIGNAL_INITIATE_BOOK, 4, 0);

    cdp_book_add_id(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_NAME, nameID);
    cdp_book_add_id(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_AGENT, agentID);
    cdp_book_add_id(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_STORAGE, storage);
    if (baseLength)
        cdp_book_add_uint32(&SIGNAL_INITIATE_BOOK->input, CDP_NAME_BASE, baseLength);

    bool result;
    signaler_action(SIGNAL_INITIATE_BOOK, result, true);

    return result;
}


bool cdp_initiate_register(cdpRecord* instance, cdpID nameID, cdpID agentID, bool borrow, void* data, size_t size) {
    assert(nameID != CDP_NAME_VOID  &&  agentID  &&  size);
    signaler_start(CDP_NAME_INITIATE, SIGNAL_INITIATE_REGISTER, 3, 0);

    cdp_book_add_id(&SIGNAL_INITIATE_REGISTER->input, CDP_NAME_NAME, nameID);
    cdp_book_add_id(&SIGNAL_INITIATE_REGISTER->input, CDP_NAME_AGENT, agentID);
    cdp_book_add_register(&SIGNAL_INITIATE_REGISTER->input, 0, CDP_NAME_DATA, CDP_AGENT_REGISTER, borrow, data, size);

    bool result;
    signaler_action(SIGNAL_INITIATE_REGISTER, result, true);

    return result;
}


bool cdp_initiate_link(cdpRecord* instance, cdpID nameID, cdpID agentID, cdpRecord* record) {
    assert(nameID != CDP_NAME_VOID  &&  agentID  &&  !cdp_record_is_void(record));
    signaler_start(CDP_NAME_INITIATE, SIGNAL_INITIATE_LINK, 3, 0);

    cdp_book_add_id(&SIGNAL_INITIATE_LINK->input, CDP_NAME_NAME, nameID);
    cdp_book_add_id(&SIGNAL_INITIATE_LINK->input, CDP_NAME_AGENT, agentID);
    cdp_book_add_link(&SIGNAL_INITIATE_LINK->input, CDP_NAME_LINK, record);

    bool result;
    signaler_action(SIGNAL_INITIATE_LINK, result, true);

    return result;
}


#define SIMPLE_SIGNAL(func, name, signal)                              \
    void func(cdpRecord* instance) {                                   \
        signaler_start(name, signal, 0, 0);                            \
        cdp_system_does_action(instance, signal);                      \
    }

SIMPLE_SIGNAL(cdp_finalize,    CDP_NAME_FINALIZE,    SIGNAL_FINALIZE)
SIMPLE_SIGNAL(cdp_reset,       CDP_NAME_RESET,       SIGNAL_RESET)
SIMPLE_SIGNAL(cdp_reference,   CDP_NAME_REFERENCE,   SIGNAL_REFERENCE)
SIMPLE_SIGNAL(cdp_unreference, CDP_NAME_UNREFERENCE, SIGNAL_UNREFERENCE)


cdpRecord* cdp_next(cdpRecord* instance) {
    signaler_start(CDP_NAME_NEXT, SIGNAL_NEXT, 0, 1);
    signaler_return(cdpRecord*, SIGNAL_NEXT, cdp_link_data);
}


cdpRecord* cdp_previous(cdpRecord* instance) {
    signaler_start(CDP_NAME_PREVIOUS, SIGNAL_PREVIOUS, 0, 1);
    signaler_return(cdpRecord*, SIGNAL_PREVIOUS, cdp_link_data);
}


bool cdp_validate(cdpRecord* instance) {
    signaler_start(CDP_NAME_VALIDATE, SIGNAL_VALIDATE, 0, 1);
    signaler_return(bool, SIGNAL_PREVIOUS, cdp_register_read_bool);
}


void cdp_remove(cdpRecord* instance, cdpRecord* target) {
    signaler_start(CDP_NAME_REMOVE, SIGNAL_REMOVE, 0, 1);

    cdp_system_does_action(instance, SIGNAL_REMOVE);

    cdpRecord* moved = cdp_book_find_by_name(&SIGNAL_REMOVE->output, CDP_NAME_OUTPUT);
    cdp_record_remove(moved, target);

    cdp_signal_reset(SIGNAL_REMOVE);
}




size_t cdp_serialize(cdpRecord* instance, void* data, size_t size) {
    return 0;
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


bool cdp_take(cdpRecord* instance, cdpRecord* target) {
    return false;
}


bool cdp_pop(cdpRecord* instance, cdpRecord* target) {
    return false;
}


cdpRecord* cdp_search(cdpRecord* instance, cdpRecord* book, cdpRecord* key) {
    return NULL;
}


cdpRecord* cdp_link(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    assert(cdp_record_is_book(instance) && nameID != CDP_NAME_VOID && !cdp_record_is_void(record));
    signaler_start(CDP_NAME_LINK, SIGNAL_LINK, 2, 1);

    cdp_book_add_id(&SIGNAL_LINK->input, CDP_NAME_NAME, nameID);
    cdp_book_add_link(&SIGNAL_LINK->input, CDP_NAME_RECORD, record);

    signaler_return(cdpRecord*, SIGNAL_LINK, cdp_link_data);
}


cdpRecord* cdp_shadow(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    return NULL;
}


cdpRecord* cdp_clone(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    return NULL;
}


cdpRecord* cdp_move(cdpRecord* instance, cdpID nameID, cdpRecord* record) {
    return NULL;
}

