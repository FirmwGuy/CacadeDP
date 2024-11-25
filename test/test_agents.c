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


#include "cdp_system.h"
#include <stdio.h>      // getc()
#include <ctype.h>      // isdigit()



#define CDP_WORD_STDIN      CDP_ID(0x004E844B80000000)      /* "stdin"       */
#define CDP_WORD_ADDER      CDP_ID(0x0004842C80000000)      /* "adder"       */
#define CDP_WORD_STDOUT     CDP_ID(0x004E847D68000000)      /* "stdout"      */

bool DONE;




static void* agent_stdin(cdpRecord* client, cdpRecord* subject, cdpAction verb, cdpRecord* object, cdpValue value) {
    assert(instace && subject);
    static cdpRecord* inp;

    switch (verb) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(subject, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("FLOAT64"), (cdpID)0, 0.0));
        return subject->data;
      }
      case CDP_ACTION_STORE_NEW: {
        cdp_record_set_store(subject, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_ARRAY, CDP_INDEX_BY_NAME, 2));
        return subject->store;
      }

      case CDP_ACTION_GET_JACK: {
        return cdp_dict_add_value(subject, cdp_text_to_word("tic"), CDP_ACRON_CDP, cdp_text_to_acronysm("UINT64"), (cdpID)0, 0);
      }
      case CDP_ACTION_CONNECT: {
        assert_uint64(value.id, ==, cdp_text_to_word("inp"));
        inp = cdp_dict_add_link(subject, value.id, object);
        return inp;
      }

      case CDP_ACTION_DATA_UPDATE: {
      #ifdef _WIN32
        int c = _getchar_nolock();
      #elif __linux__
        int c = getc_unlocked(stdin);
      #endif
        if (EOF != c) {
            if (isdigit(c)) {
                char s[] = {(char) c, 0};
                uint32_t u = (unsigned)atoi(s);
                cdp_cascade_data_update(client, inp, sizeof(u), sizeof(u), CDP_V(u));
            } else if ('q' == tolower(c)) {
                DONE = true;
            }
        }
        return subject;
      }
    }

    return NULL;
}


bool adder_agent_initiate(cdpRecord* instance, cdpTask* signal) {
    ADDER_OP1 = cdp_tag_id_add_static("op1");
    ADDER_OP2 = cdp_tag_id_add_static("op2");
    ADDER_ANS = cdp_tag_id_add_static("ans");

    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);

    // FixMe: define dictionary types by flag instead of agent.
    cdp_record_initialize(instance, CDP_ROLE_BOOK, 0, nameID, AGENT_ADDER, CDP_STORAGE_ARRAY, 3);

    cdp_book_add_uint32(instance, ADDER_OP1, 5);

    cdpRecord* regOp2 = cdp_book_add_uint32(instance, ADDER_OP2, 0);
    CDP_RECORD_SET_ATTRIB(regOp2, CDP_ATTRIB_BABY);

    return true;
}


bool adder_agent_update(cdpRecord* instance, cdpTask* signal) {
    cdpRecord* ansLink = cdp_record_find_by_name(instance, ADDER_ANS);
    assert(ansLink);

    uint32_t op2 = cdp_dict_get_uint32(&signal->input, ADDER_OP2);
    cdpRecord* regOp2 = cdp_record_find_by_name(instance, ADDER_OP2);
    cdp_register_update_uint32(regOp2, op2);

    uint32_t op1 = cdp_dict_get_uint32(instance, ADDER_OP1);
    uint32_t ans = op1 + op2;
    cdp_update(ansLink, &ans, sizeof(ans));

    return true;
}




bool agent_stdout(cdpRecord* client, cdpRecord* subject, cdpAction verb, cdpRecord* object) {
    switch (verb) {
      default:  return true;
    }
    cdpID nameID = cdp_dict_get_id(&signal->input, CDP_NAME_NAME);

    cdp_record_initialize_register(instance, nameID, AGENT_STDOUT, false, NULL, sizeof(uint32_t));

    return true;
}


bool stdout_agent_update(cdpRecord* instance, cdpTask* signal) {
    uint32_t value = cdp_dict_get_uint32(&signal->input, CDP_TAG_REGISTER);
    cdp_register_update_uint32(instance, value);
    printf("%u\n", value);
    return true;
}




void* test_agents_setup(const MunitParameter params[], void* user_data) {
    cdp_system_set_agent(CDP_ACRON_CDP, CDP_WORD_STDIN,  agent_stdin);
    cdp_system_set_agent(CDP_ACRON_CDP, CDP_WORD_ADDER,  agent_adder);
    cdp_system_set_agent(CDP_ACRON_CDP, CDP_WORD_STDOUT, agent_stdout);

    cdp_system_startup();

    return NULL;
}



void test_agents_tear_down(void* fixture) {
    cdp_system_shutdown();
}


MunitResult test_agents(const MunitParameter params[], void* user_data_or_fixture) {
    extern cdpRecord* CASCADE;

    // Instance initiation
    cdpRecord* pipeline = cdp_dict_add_list(CASCADE, CDP_AUTOID, CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_ARRAY, 3);   assert_not_null(pipeline);

    cdpRecord  child  = {0};
    cdpRecord* stdin  = cdp_record_append(pipeline, false, cdp_cascade_record_new(cdp_root(), &child, CDP_WORD_STDIN,  CDP_ACRON_CDP, CDP_WORD_STDIN,  NULL, CDP_V(0), NULL, CDP_V(0));   assert_not_null(stdin);  assert_false(cdp_record_is_empty(stdin));
    cdpRecord* adder  = cdp_record_append(pipeline, false, cdp_cascade_record_new(cdp_root(), &child, CDP_WORD_ADDER,  CDP_ACRON_CDP, CDP_WORD_ADDER,  NULL, CDP_V(0), NULL, CDP_V(0));   assert_not_null(adder);  assert_false(cdp_record_is_empty(adder));
    cdpRecord* stdout = cdp_record_append(pipeline, false, cdp_cascade_record_new(cdp_root(), &child, CDP_WORD_STDOUT, CDP_ACRON_CDP, CDP_WORD_STDOUT, NULL, CDP_V(0), NULL, CDP_V(0));   assert_not_null(stdout); assert_false(cdp_record_is_empty(stdout));

    // Link pipeline in reverse (upstream) order
    cdpRecord* op1 = cdp_cascade_get_input(cdp_root(), adder,  cdp_text_to_word("op1"));    assert_not_null(op1);
    cdpRecord* out = cdp_cascade_get_input(cdp_root(), stdout, cdp_text_to_word("out"));    assert_not_null(out);

    cdpRecord* output;
    output = cdp_cascade_connect(cdp_root(), stdin, op1, cdp_text_to_word("inp"));          assert_true(output);
    output = cdp_cascade_connect(cdp_root(), adder, out, cdp_text_to_word("ans"));          assert_true(output);

    // Execute pipeline
    while (!DONE) {
        cdp_system_step();
    }

    cdp_record_remove(pipeline);

    return MUNIT_OK;
}

