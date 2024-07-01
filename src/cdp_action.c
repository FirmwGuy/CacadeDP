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




void cdp_system_initiate_actions(void) {
    extern cdpRecord* NAME;

    /**** WARNING: this must be done in the same order as the
                enumeration in "cdp_action.h". ****/

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,  "storage");
    cdp_book_add_static_text(NAME, CDP_AUTO_ID,     "base");

    cdp_book_add_static_text(NAME, CDP_AUTO_ID,   "record");

}


bool cdp_action_ignore(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_error(cdpRecord* instance, cdpSignal* signal) {
    cdp_record_initialize_list(&signal->condition, CDP_NAME_ERROR, CDP_STO_CHD_LINKED_LIST);
    cdp_book_add_static_text(&signal->condition, CDP_AUTO_ID, "Unsupported action.");
    return false;
}




/*
 *    Record actions
 */


bool cdp_action_initiate_book(cdpRecord* instance, cdpSignal* signal) {
    // ToDo: use cdp_book_get_by_position() to speedup things.
    cdpID    nameID  = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);
    cdpID    agentID = cdp_dict_get_id(&signal->input, CDP_NAME_AGENT);
    unsigned storage = cdp_dict_get_id(&signal->input, CDP_NAME_STORAGE);

    cdpRecord* regBase  = cdp_book_find_by_name(&signal->input, CDP_NAME_BASE);
    if (regBase)
        cdp_record_initialize(instance, CDP_TYPE_BOOK, 0, nameID, agentID, storage, cdp_register_read_uint32(regBase));
    else
        cdp_record_initialize(instance, CDP_TYPE_BOOK, 0, nameID, agentID, storage);

    return true;
}


bool cdp_action_initiate_register(cdpRecord* instance, cdpSignal* signal) {
    cdpID  nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);
    cdpID agentID = cdp_dict_get_id(&signal->input, CDP_NAME_AGENT);

    cdpRecord* data = cdp_book_find_by_name(&signal->input, CDP_NAME_DATA);

    cdp_record_transfer(data, instance);
    instance->metadata.id    = nameID;
    instance->metadata.agent = agentID;

    return true;
}


bool cdp_action_initiate_link(cdpRecord* instance, cdpSignal* signal) {
    cdpID  nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);
    cdpRecord* link = cdp_book_find_by_name(&signal->input, CDP_NAME_LINK);

    cdp_record_transfer(link, instance);
    instance->metadata.id = nameID;

    return true;
}


bool cdp_action_terminate(cdpRecord* instance, cdpSignal* signal) {
    cdp_record_finalize(instance);
    return true;
}


bool cdp_action_reset_book(cdpRecord* instance, cdpSignal* signal) {
    cdp_book_reset(instance);
    return true;
}


bool cdp_action_reset_register(cdpRecord* instance, cdpSignal* signal) {
    cdp_register_reset(instance);
    return true;
}


bool cdp_action_next(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* nextRec = cdp_book_next(NULL, instance);
    if (nextRec)
        cdp_book_add_link(&signal->output, CDP_NAME_OUTPUT, nextRec);
    return true;
}


bool cdp_action_previous(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* prevRec = cdp_book_prev(NULL, instance);
    if (prevRec)
        cdp_book_add_link(&signal->output, CDP_NAME_OUTPUT, prevRec);
    return true;
}


bool cdp_action_validate(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    cdp_book_add_bool(&signal->output, CDP_NAME_OUTPUT, true);
    return true;
}


bool cdp_action_remove(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* record = cdp_book_add_bool(&signal->output, CDP_NAME_OUTPUT, false);   // Create a temporary (bool) record for overwrite.
    cdp_book_remove(instance, record);
    return true;
}




/*
 *    Book actions
 */


bool cdp_action_add(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_prepend(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_insert(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


bool cdp_action_first(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_last(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_take(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_pop(cdpRecord* instance, cdpSignal* signal) {
    return true;
}


bool cdp_action_search(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


bool cdp_action_link(cdpRecord* instance, cdpSignal* signal) {
    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);
    cdpRecord* record = cdp_dict_get_link(&signal->input, CDP_NAME_RECORD);
    cdpRecord* newLink = cdp_book_add_link(instance, nameID, record);
    cdp_book_add_link(&signal->output, CDP_NAME_OUTPUT, newLink);
    return false;
}


bool cdp_action_shadow(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}


bool cdp_action_clone(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}


bool cdp_action_move(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}




/*
 *    Register actions
 */


bool cdp_action_reference(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}


bool cdp_action_unreference(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return true;
}

bool cdp_action_serialize(cdpRecord* instance, cdpSignal* signal) {
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
    //cdp_register_update(instance,);
    return false;
}


bool cdp_action_patch(cdpRecord* instance, cdpSignal* signal) {
    // Pending...
    return false;
}




/*
 *    Register textualization
 */

#define  action_reg_textualize(name)                                   \
    bool cdp_action_textualize_##name(cdpRecord* instance, cdpSignal* signal) {return true;}\
    bool cdp_action_untextualize_##name(cdpRecord* instance, cdpSignal* signal) {return true;}

action_reg_textualize(bool);
action_reg_textualize(byte);
action_reg_textualize(uint16);
action_reg_textualize(uint32);
action_reg_textualize(uint64);
action_reg_textualize(int16);
action_reg_textualize(int32);
action_reg_textualize(int64);
action_reg_textualize(float32);
action_reg_textualize(float64);

