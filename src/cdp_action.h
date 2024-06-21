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


#include "cdp_signal.h"


enum {
    CDP_NAME_STORAGE = CDP_NAME_SIGNAL_COUNT,
    CDP_NAME_BASE,

};


bool cdp_action_ignore(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_error(cdpRecord* instance, cdpSignal* signal);


/*
 *    Record actions.
 */
bool cdp_action_create_book(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_create_register(cdpRecord* instance, cdpSignal* signa);
bool cdp_action_create_link(cdpRecord* instance, cdpSignal* signa);
bool cdp_action_destroy(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_reset_book(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_reset_register(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_reference(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_unreference(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_link(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_shadow(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_clone(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_move(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_remove(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_next(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_previous(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_validate(cdpRecord* instance, cdpSignal* signal);


/*
 *    Register actions.
 */
bool cdp_action_serialize(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_unserialize(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_textualize(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_untextualize(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_read(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_update(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_patch(cdpRecord* instance, cdpSignal* signal);



/*
 *    Book actions.
 */
bool cdp_action_add(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_prepend(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_insert(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_first(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_last(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_take(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_pop(cdpRecord* instance, cdpSignal* signal);
bool cdp_action_search(cdpRecord* instance, cdpSignal* signal);

#define  action_reg_def_textualize(name)                               \
    bool cdp_action_textualize_##name(cdpRecord* instance, cdpSignal* signal); \
    bool cdp_action_untextualize_##name(cdpRecord* instance, cdpSignal* signal)

action_reg_def_textualize(bool);
action_reg_def_textualize(byte);
action_reg_def_textualize(uint16);
action_reg_def_textualize(uint32);
action_reg_def_textualize(uint64);
action_reg_def_textualize(int16);
action_reg_def_textualize(int32);
action_reg_def_textualize(int64);
action_reg_def_textualize(float32);
action_reg_def_textualize(float64);


#endif
