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

#ifndef CDP_ACTION_H
#define CDP_ACTION_H


#include "cdp_record.h"


bool cdp_action(cdpRecord* instance, cdpSignal* signal);


cdpSignal* cdp_signal_new(cdpID nameID, unsigned itemsArg, unsigned itemsRes);
void cdp_signal_del(cdpSignal* signal);
void cdp_signal_reset(cdpSignal* signal);


bool cdp_action(cdpRecord* instance, cdpSignal* signal);


cdpRecord* cdp_create_book(cdpRecord* instance, cdpID nameID, cdpID agentID, unsigned storage, unsigned baseLength);
cdpRecord* cdp_create_register(cdpRecord* instance, cdpID nameID, cdpID agentID, void* data, size_t size);

void cdp_destroy(cdpRecord* instance);
void cdp_reset(cdpRecord* instance);
void cdp_free(cdpRecord* instance);
void cdp_reference(cdpRecord* instance);

cdpRecord* cdp_link(cdpRecord* instance, cdpRecord* newParent, cdpID nameID);
cdpRecord* cdp_copy(cdpRecord* instance, cdpRecord* newParent, cdpID nameID);
cdpRecord* cdp_move(cdpRecord* instance, cdpRecord* newParent, cdpID nameID);

cdpRecord* cdp_next(cdpRecord* instance);
cdpRecord* cdp_previous(cdpRecord* instance);

bool cdp_validate(cdpRecord* instance);

size_t cdp_serialize(cdpRecord* instance, void* data, size_t size);
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

//void       cdp_sort(cdpRecord* instance);


#endif
