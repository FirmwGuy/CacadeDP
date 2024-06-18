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


#include "cdp_action.h"




bool cdp_action_ignore(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_error(cdpRecord* instance, cdpSignal* signal) {
    cdp_record_initialize_list(&signal->error, CDP_NAME_ERROR, CDP_STO_CHD_LINKED_LIST);
    cdp_book_add_text(&signal->error, "Unsupported action.");
    return false;
}




/*
 *    Record actions.
 */


bool cdp_action_create_book(cdpRecord* instance, cdpSignal* signal) {
    cdpID    nameID     = cdp_dict_get_id    (&signal->input, CDP_NAME_NAME);
    cdpID    agentID    = cdp_dict_get_id    (&signal->input, CDP_NAME_AGENT);
    unsigned storage    = cdp_dict_get_id    (&signal->input, CDP_NAME_STORAGE);
    size_t   baseLength = cdp_dict_get_uint32(&signal->input, CDP_NAME_BASE);
    assert(!instance  &&  nameID != CDP_NAME_VOID  &&  agentID  &&  storage < CDP_STO_CHD_COUNT);

    cdp_book_add_book(&signal->output, nameID, agentID, storage, baseLength);

    return true;
}


bool cdp_action_create_register(cdpRecord* instance, cdpSignal* signa) {
    cdpID    nameID     = cdp_dict_get_id    (&signal->input, CDP_NAME_NAME);
    cdpID    agentID    = cdp_dict_get_id    (&signal->input, CDP_NAME_AGENT);
    size_t      size    = cdp_register_size(&signal->input, );
    void*   data = cdp_dict_get_id    (&signal->input, CDP_NAME_BASE);
    assert(instance  &&  nameID != CDP_NAME_VOID &&  agentID  &&  size);
...
    cdp_book_add_register(&signal->output, nameID, agentID, storage, baseLength);

    return true;
}


bool cdp_action_destroy(cdpRecord* instance, cdpSignal* signal) {
    cdp_record_finalize(instance);
    return true;
}


bool cdp_action_reset_book(cdpRecord* instance, cdpSignal* signal) {
    cdp_book_reset(instance, 64);       // FixMe.
    return true;
}


bool cdp_action_reset_register(cdpRecord* instance, cdpSignal* signal) {
    cdp_register_reset(instance);
    return true;
}


bool cdp_action_free(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


bool cdp_action_reference(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


bool cdp_action_copy(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}


bool cdp_action_move(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}


bool cdp_action_remove(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord record;
    cdp_book_remove(NULL, instance, &record);
    cdp_book_add_record(&signal->output, CDP_NAME_RECORD, &record);
    return true;
}


bool cdp_action_link(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}


bool cdp_action_next(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* nextRec = cdp_book_next(NULL, instance);
    if (nextRec)
        cdp_book_add_link(&signal->output, CDP_NAME_NEXT, nextRec);
    return true;
}


bool cdp_action_previous(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* prevRec = cdp_book_prev(NULL, instance);
    if (prevRec)
        cdp_book_add_link(&signal->output, CDP_NAME_PREVIOUS, prevRec);
    return true;
}


bool cdp_action_validate(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}




/*
 *    Register actions.
 */


bool cdp_action_serialize(cdpRecord* instance, cdpSignal* signal) {
    cdp_book_copy(&signal->output, CDP_NAME_SERIALIZE, instance);
    return true;
}


bool cdp_action_unserialize(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


bool cdp_action_textualize(cdpRecord* instance, cdpSignal* signal) {
    // ToDo: encode to base64.
    return true;
}


bool cdp_action_untextualize(cdpRecord* instance, cdpSignal* signal) {
    // ToDO: decode base64.
    return true;
}


bool cdp_action_read(cdpRecord* instance, cdpSignal* signal) {
    // make a copy of data and put it on output.
    return false;
}


bool cdp_action_update(cdpRecord* instance, cdpSignal* signal) {
    cdp_register_update(instance,);
    return false;
}


bool cdp_action_patch(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}




/*
 *    Book actions.
 */


bool cdp_action_add(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* record = cdp_book_find_by_name(&signal->input, CDP_NAME_RECORD);
    record = cdp_book_add_record(instance, record, false);
    cdp_book_add_link(&signal->output, CDP_NAME_RECORD, record);
    return true;
}


bool cdp_action_prepend(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* record = cdp_book_find_by_name(&signal->input, CDP_NAME_RECORD);
    record = cdp_book_add_record(instance, record, true);
    cdp_book_add_link(&signal->output, CDP_NAME_RECORD, record);
    return true;
}


bool cdp_action_insert(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


bool cdp_action_first(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* record = cdp_book_first(instance);
    cdp_book_add_link(&signal->output, CDP_NAME_RECORD, record);
    return true;
}


bool cdp_action_last(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* record = cdp_book_last(instance);
    cdp_book_add_link(&signal->output, CDP_NAME_RECORD, record);
    return true;
}


bool cdp_action_take(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* record = cdp_book_take(instance);
    cdp_book_add_record(&signal->output, CDP_NAME_RECORD, record);
    return true;
}


bool cdp_action_pop(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* record = cdp_book_pop(instance);
    cdp_book_add_record(&signal->output, CDP_NAME_RECORD, record);
    return true;
}


bool cdp_action_search(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


