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
#include <stdio.h>      // getc()
#include <ctype.h>      // isdigit()




bool DONE;




cdpID AGENT_STDIN;


static bool stdin_agent_initiate(cdpRecord* instance, cdpSignal* signal) {
    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);

    cdp_record_initialize_register(instance, nameID, AGENT_STDIN, false, NULL, sizeof(uint32_t));

    return true;
}


static bool stdin_agent_step(cdpRecord* instance, cdpSignal* signal) {
  #ifdef _WIN32
    int c = getchar();
  #elif __linux__
    int c = getc_unlocked(stdin);
  #endif
    if (EOF != c) {
        if (isdigit(c)) {
            char s[] = {(char) c, 0};
            uint32_t value = (unsigned)atoi(s);
            cdpRecord* target = cdp_link_data(instance);
            //cdp_update(target, &value, sizeof(value));
        } else if ("q" == tolower(c)) {
            DONE = true;
        }
    }
    return true;
}




cdpID AGENT_ADDER;
cdpID ADDER_OP1;
cdpID ADDER_OP2;
cdpID ADDER_ANS;


bool adder_agent_initiate(cdpRecord* instance, cdpSignal* signal) {
    ADDER_OP1 = cdp_name_id_add_static("op1");
    ADDER_OP2 = cdp_name_id_add_static("op2");
    ADDER_ANS = cdp_name_id_add_static("ans");

    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);

    // FixMe: define dictionary types by flag instead of agent.
    cdp_record_initialize(instance, CDP_TYPE_BOOK, 0, nameID, AGENT_ADDER, CDP_STO_CHD_ARRAY, 3);

    cdp_book_add_uint32(instance, ADDER_OP1, 5);

    cdpRecord* regOp2 = cdp_book_add_uint32(instance, ADDER_OP2, 0);
    CDP_RECORD_SET_ATTRIB(regOp2, CDP_ATTRIB_BABY);

    return true;
}


bool adder_agent_update(cdpRecord* instance, cdpSignal* signal) {
    cdpRecord* ansLink = cdp_book_find_by_name(instance, ADDER_ANS);
    assert(ansLink);

    uint32_t op2 = cdp_dict_get_uint32(&signal->input, ADDER_OP2);
    cdpRecord regOp2 = cdp_book_find_by_name(instance, ADDER_OP2);
    cdp_register_update_uint32(regOp2, op2);

    uint32_t op1 = cdp_dict_get_uint32(instance, ADDER_OP1);
    uint32_t ans = op1 + op2;
    cdp_update(ansLink, &ans, sizeof(ans));

    return true;
}




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




void* test_agents_setup(const MunitParameter params[], void* user_data) {
    cdpID linkID = cdp_system_get_agent(CDP_AGENT_LINK);
    AGENT_STDIN = cdp_system_set_agent("stdin", 0, 1, &linkID, 1, stdin_agent_initiate, NULL);
    cdp_system_set_action(AGENT_STDIN, "step", stdin_agent_step);

    cdpID bookID = cdp_system_get_agent(CDP_AGENT_BOOK);
    AGENT_ADDER = cdp_system_set_agent("adder", 0, 1, &bookID, 1, adder_agent_initiate, NULL);
    cdp_system_set_action(AGENT_ADDER, "update", stdin_agent_step);

    cdpID uint32ID = cdp_system_get_agent(CDP_AGENT_UINT32);
    AGENT_STDOUT = cdp_system_set_agent("stdout", sizeof(uint32_t), 1, &uint32ID, 1, stdout_agent_initiate, NULL);
    cdp_system_set_action(AGENT_STDOUT, "update", stdout_agent_update);

    cdp_system_startup();
    return NULL;
}


void test_agents_tear_down(void* fixture) {
    cdp_system_shutdown();
}


MunitResult test_agents(const MunitParameter params[], void* user_data_or_fixture) {
    extern cdpRecord* TEMP;

    // Instance initiation
    cdpRecord* cascade = cdp_book_add_dictionary(TEMP, CDP_NAME_BOOK, CDP_STO_CHD_ARRAY, 3);
    cdpRecord* stdinI  = cdp_book_add_instance(cascade, CDP_ID("stdin"), NULL);
    cdpRecord* adderI  = cdp_book_add_instance(cascade, CDP_ID("adder"), NULL);
    cdpRecord* stdoutI = cdp_book_add_instance(cascade, CDP_ID("stdout"), NULL);

    // Connect pipeline
    cdp_system_connect(adderI, CDP_ID("ans"), stdoutI);

    cdpRecord* op2 = cdp_book_find_by_name(adderI, CDP_ID("op2"));
    assert(op2);
    cdp_system_connect(stdinI, cdp_record_get_id(stdinI), op2);

    // Execute pipeline
    while (!DONE) {
        cdp_system_step();
    }

    cdp_book_delete(cascade);

    return MUNIT_OK;
}

