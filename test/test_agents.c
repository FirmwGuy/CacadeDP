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


#include "test.h"
#include "cdp_agent.h"
//#include <stdio.h>      // sprintf()




cdpID AGENT_STDOUT;


bool stdout_agent_initiate(cdpRecord* instance, cdpSignal* signal) {
    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);

    cdp_record_initialize_register(instance, nameID, AGENT_STDOUT, false, NULL, sizeof(uint32_t));

    return true;
}


bool stdout_agent_update(cdpRecord* instance, cdpSignal* signal) {
    uint32_t value = cdp_dict_get_uint32(&signal->input, CDP_NAME_VALUE);
    cdp_register_update_uint32(instance, value);
    printf("%u\n", value);
    return true;
}





cdpID AGENT_STDIN;


bool stdin_agent_initiate(cdpRecord* instance, cdpSignal* signal) {
    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);

    cdp_record_initialize_register(instance, nameID, AGENT_STDOUT, false, NULL, sizeof(uint32_t));

    return true;
}


bool stdin_agent_step(cdpRecord* instance, cdpSignal* signal) {
    int c = getc_unlocked(stdin);
    if (EOF != c  &&  isdigit(c)) {
        uint32_t value = (unsigned)atoi(c);
        cdpRecord* target = cdp_link_data(instance);
        cdp_update(target, &value, sizeof(value));
    }
    return true;
}





cdpID AGENT_ADDER;
cdpID ADDER_OP1;
cdpID ADDER_OP2;
cdpID ADDER_ANS;


bool adder_agent_initiate(cdpRecord* instance, cdpSignal* signal) {
    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);

    cdp_record_initialize_dictionary(instance, nameID, CDP_STO_CHD_ARRAY, 3);

    ADDER_OP1 = cdp_name_id_add_static("op1");
    ADDER_OP2 = cdp_name_id_add_static("op2");
    ADDER_ANS = cdp_name_id_add_static("ans");

    cdp_book_add_uint32(instance, ADDER_OP1, 0);
    cdp_book_add_uint32(instance, ADDER_OP2, 0);

    //cdp_book_add_link(instance, ADDER_OP2, 0);

    return true;
}


bool adder_agent_update(cdpRecord* instance, cdpSignal* signal) {
    uint32_t op1 = cdp_dict_get_uint32(instance, ADDER_OP1);
    uint32_t op2 = cdp_dict_get_uint32(&signal->input, ADDER_OP2);
    uint32_t ans = op1 + op2;

    cdpRecord regOp2 = cdp_book_find_by_name(instance, ADDER_OP2);
    cdp_register_update_uint32(regOp2, op2);

    cdp_update(cdp_book_find_by_name(instance, ADDER_ANS), ans);

    return true;
}





void input_agent_initiate() {
    cdpID uint32ID = cdp_system_get_agent(CDP_AGENT_UINT32);
    AGENT_STDOUT = cdp_system_set_agent("stdout", sizeof(uint32_t), 1, &uint32ID, 1, stdout_agent_initiate, NULL);
    cdp_system_set_action(AGENT_STDOUT, "update", stdout_agent_update);


    cdpID linkID = cdp_system_get_agent(CDP_AGENT_LINK);
    AGENT_STDIN = cdp_system_set_agent("stdin", 0, 1, &linkID, 1, stdin_agent_initiate, NULL);
    cdp_system_set_action(AGENT_STDIN, "step", stdin_agent_step);


    cdpID bookID = cdp_system_get_agent(CDP_AGENT_BOOK);
    AGENT_ADDER = cdp_system_set_agent("adder", 0, 1, &bookID, 1, adder_agent_initiate, NULL);
    cdp_system_set_action(AGENT_STDIN, "update", stdin_agent_step);
}


MunitResult test_agents(const MunitParameter params[], void* user_data_or_fixture) {

    return MUNIT_OK;
}

