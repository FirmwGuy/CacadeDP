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

#ifndef CDP_SIGNAL_H
#define CDP_SIGNAL_H


#include "cdp_agent.h"


// Signal Name IDs:
enum {
    // System signals
    CDP_NAME_STARTUP = CDP_NAME_FLAG_COUNT,
    CDP_NAME_SHUTDOWN,
    CDP_NAME_CONNECT,
    CDP_NAME_DISCONNECT,

    // Record signals
    CDP_NAME_INITIATE,
    CDP_NAME_TERMINATE,
    CDP_NAME_RESET,
    CDP_NAME_NEXT,
    CDP_NAME_PREVIOUS,
    CDP_NAME_VALIDATE,
    CDP_NAME_REMOVE,

    // Book signals
    CDP_NAME_ADD,
    CDP_NAME_PREPEND,
    CDP_NAME_INSERT,
    CDP_NAME_FIRST,
    CDP_NAME_LAST,
    CDP_NAME_TAKE,
    CDP_NAME_POP,
    CDP_NAME_SEARCH,
    CDP_NAME_LINK,
    CDP_NAME_SHADOW,
    CDP_NAME_CLONE,
    CDP_NAME_MOVE,

    // Register signals
    CDP_NAME_REFERENCE,
    CDP_NAME_UNREFERENCE,
    CDP_NAME_SERIALIZE,
    CDP_NAME_UNSERIALIZE,
    CDP_NAME_TEXTUALIZE,
    CDP_NAME_UNTEXTUALIZE,
    CDP_NAME_READ,
    CDP_NAME_UPDATE,
    CDP_NAME_PATCH,

    CDP_NAME_SIGNAL_COUNT
};

#define CDP_SIGNAL_COUNT  (CDP_NAME_SIGNAL_COUNT - CDP_NAME_STARTUP)


void cdp_system_initiate_signals(void);
void cdp_system_finalize_signals(void);


// Signal handlers
void cdp_signal_initiate(cdpSignal* signal, cdpID nameID, unsigned itemsArg, unsigned itemsRes);
void cdp_signal_finalize(cdpSignal* signal);
cdpSignal* cdp_signal_new(cdpID nameID, unsigned itemsArg, unsigned itemsRes);
void cdp_signal_del(cdpSignal* signal);
void cdp_signal_reset(cdpSignal* signal);


// Record signals
bool cdp_initiate(cdpRecord* instance, cdpID nameID, cdpRecord* bookArgs);
bool cdp_initiate_book(cdpRecord* instance, cdpID nameID, cdpID agentID, unsigned storage, unsigned baseLength);
bool cdp_initiate_register(cdpRecord* instance, cdpID nameID, cdpID agentID, bool borrow, void* data, size_t size);
bool cdp_initiate_link(cdpRecord* instance, cdpID nameID, cdpRecord* record);
void cdp_terminate(cdpRecord* instance);
void cdp_reset(cdpRecord* instance);
void cdp_remove(cdpRecord* instance, cdpRecord* target);
cdpRecord* cdp_next(cdpRecord* instance);
cdpRecord* cdp_previous(cdpRecord* instance);
bool cdp_validate(cdpRecord* instance);

// Book signals
cdpRecord* cdp_add(cdpRecord* instance, cdpRecord* record);
cdpRecord* cdp_prepend(cdpRecord* instance, cdpRecord* record);
cdpRecord* cdp_insert(cdpRecord* instance, size_t position, cdpRecord* record);
cdpRecord* cdp_first(cdpRecord* instance);
cdpRecord* cdp_last(cdpRecord* instance);
bool cdp_take(cdpRecord* instance, cdpRecord* target);
bool cdp_pop(cdpRecord* instance, cdpRecord* target);
cdpRecord* cdp_search(cdpRecord* instance, cdpRecord* key);
cdpRecord* cdp_link(cdpRecord* instance, cdpID nameID, cdpRecord* record);
cdpRecord* cdp_shadow(cdpRecord* instance, cdpID nameID, cdpRecord* record);
cdpRecord* cdp_clone(cdpRecord* instance, cdpID nameID, cdpRecord* record);
cdpRecord* cdp_move(cdpRecord* instance, cdpID nameID, cdpRecord* record);

// Register signals
void cdp_reference(cdpRecord* instance);
void cdp_unreference(cdpRecord* instance);
size_t cdp_serialize(cdpRecord* instance, void* data, size_t size);
bool cdp_unserialize(cdpRecord* instance, void* data, size_t size);
bool cdp_textualize(cdpRecord* instance, char** data, size_t* length);
bool cdp_untextualize(cdpRecord* instance, char* data, size_t length);
void* cdp_read(cdpRecord* instance, void* data, size_t* size);
void cdp_update(cdpRecord* instance, void* data, size_t size);
void* cdp_patch(cdpRecord* instance, void* data, size_t size);


static inline cdpRecord* cdp_book_add_instance(cdpRecord* book, cdpID name, cdpRecord* bookArgs) {
    cdpRecord instance = {0};
    cdp_initiate(&instance, name, bookArgs);
    return cdp_book_add_record(book, &instance, false);
}

static inline cdpRecord* cdp_book_prepend_instance(cdpRecord* book, cdpID name, cdpRecord* bookArgs) {
    cdpRecord instance = {0};
    cdp_initiate(&instance, name, bookArgs);
    return cdp_book_add_record(book, &instance, true);
}


#endif
