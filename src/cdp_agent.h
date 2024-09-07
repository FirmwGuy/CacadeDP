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

#ifndef CDP_AGENT_H
#define CDP_AGENT_H


#include "cdp_signal.h"


enum {
    CDP_NAME_STORAGE = CDP_NAME_SIGNAL_COUNT,
    CDP_NAME_BASE,

    CDP_NAME_RECORD,

    CDP_NAME_ID_ACTION_COUNT
};

#define CDP_ACTION_COUNT  (CDP_NAME_ID_ACTION_COUNT - CDP_NAME_STORAGE)


void cdp_system_initiate_actions(void);

bool cdp_agent_ignore(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_error(cdpRecord* instance, cdpTask* signal);


/*
 *    Record actions
 */
bool cdp_agent_connect(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_initiate_book(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_initiate_register(cdpRecord* instance, cdpTask* signa);
bool cdp_agent_initiate_link(cdpRecord* instance, cdpTask* signa);
bool cdp_agent_terminate(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_reset_book(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_reset_register(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_next(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_previous(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_validate(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_remove(cdpRecord* instance, cdpTask* signal);


/*
 *    Book actions
 */
bool cdp_agent_add(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_prepend(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_insert(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_first(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_last(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_take(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_pop(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_search(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_link(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_shadow(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_clone(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_move(cdpRecord* instance, cdpTask* signal);


/*
 *    Register actions.
 */
bool cdp_agent_reference(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_unreference(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_serialize(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_unserialize(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_textualize(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_untextualize(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_read(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_update(cdpRecord* instance, cdpTask* signal);
bool cdp_agent_patch(cdpRecord* instance, cdpTask* signal);


#define  action_reg_def_textualize(name)                               \
    bool cdp_agent_textualize_##name(cdpRecord* instance, cdpTask* signal); \
    bool cdp_agent_untextualize_##name(cdpRecord* instance, cdpTask* signal)


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
