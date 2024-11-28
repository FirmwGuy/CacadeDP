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




static void* agent_stdin(cdpRecord* client, cdpRecord* subject, unsigned verb, cdpRecord* object, cdpValue value) {
    assert(client && subject);
    static cdpRecord* inp;

    switch (verb) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(subject, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("FLOAT64"), (cdpID)0, 0.0));
        return subject->data;
      }
      case CDP_ACTION_STORE_NEW: {
        cdp_record_set_store(subject, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_LINKED_LIST, CDP_INDEX_BY_NAME));
        return subject->store;
      }

      case CDP_ACTION_GET_INLET: {
        assert_uint64(value.id, ==, cdp_text_to_word("tic"));
        return subject;
      }
      case CDP_ACTION_CONNECT: {
        assert_uint64(value.id, ==, cdp_text_to_word("inp"));
        inp = cdp_dict_add_link(subject, value.id, object);
        return inp;
      }
      case CDP_ACTION_UNPLUG: {
        cdp_record_delete_children(subject);
        inp = NULL;
        return subject;
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
                double d = atof(s);
                cdp_cascade_data_update(client, inp, sizeof(d), sizeof(d), CDP_V(d));
            } else if ('q' == tolower(c)) {
                DONE = true;
            }
        }
        return subject;
      }
    }

    return NULL;
}




static void* agent_adder(cdpRecord* client, cdpRecord* subject, unsigned verb, cdpRecord* object, cdpValue value) {
    assert(client && subject);
    static cdpRecord* num;
    static cdpRecord* ans;

    switch (verb) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(subject, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("FLOAT64"), (cdpID)0, 0.0));
        return subject->data;
      }
      case CDP_ACTION_STORE_NEW: {
        cdp_record_set_store(subject, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_ARRAY, CDP_INDEX_BY_NAME, 2));
        return subject->store;
      }

      case CDP_ACTION_GET_INLET: {
        assert_uint64(value.id, ==, cdp_text_to_word("num"));
        num = cdp_dict_add_value(subject, value.id, CDP_ACRON_CDP, CDP_WORD_ADDER, (cdpID)0, 0.0, sizeof(double), sizeof(double));
        return num;
      }
      case CDP_ACTION_CONNECT: {
        assert_uint64(value.id, ==, cdp_text_to_word("ans"));
        ans = cdp_dict_add_link(subject, value.id, object);
        return ans;
      }
      case CDP_ACTION_UNPLUG: {
        cdp_record_remove(num, NULL);
        num = NULL;
        return subject;
      }

      case CDP_ACTION_DATA_UPDATE: {
        cdp_record_update_value(num, sizeof(double), value);

        double d = value.float64 + cdp_record_value(subject).float64;

        cdp_record_update_value(subject, sizeof(d), CDP_V(d));
        cdp_cascade_data_update(client, ans, sizeof(d), sizeof(d), CDP_V(d));
        return subject;
      }
    }

    return NULL;
}




static void* agent_stdout(cdpRecord* client, cdpRecord* subject, unsigned verb, cdpRecord* object, cdpValue value) {
    assert(client && subject);

    switch (verb) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(subject, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("FLOAT64"), (cdpID)0, 0.0));
        return subject->data;
      }

      case CDP_ACTION_GET_INLET: {
        return subject;
      }

      case CDP_ACTION_DATA_UPDATE: {
        cdp_record_update_value(subject, sizeof(value), value);
        printf("%f\n", value.float64);
        return subject;
      }
    }

    return NULL;
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
    cdpRecord* stdin  = cdp_record_append(pipeline, false, cdp_cascade_record_new(cdp_root(), &child, CDP_WORD_STDIN,  CDP_ACRON_CDP, CDP_WORD_STDIN,  NULL, CDP_V(0), NULL, CDP_V(0)));  assert_not_null(stdin);  assert_false(cdp_record_is_empty(stdin));
    cdpRecord* adder  = cdp_record_append(pipeline, false, cdp_cascade_record_new(cdp_root(), &child, CDP_WORD_ADDER,  CDP_ACRON_CDP, CDP_WORD_ADDER,  NULL, CDP_V(0), NULL, CDP_V(0)));  assert_not_null(adder);  assert_false(cdp_record_is_empty(adder));
    cdpRecord* stdout = cdp_record_append(pipeline, false, cdp_cascade_record_new(cdp_root(), &child, CDP_WORD_STDOUT, CDP_ACRON_CDP, CDP_WORD_STDOUT, NULL, CDP_V(0), NULL, CDP_V(0)));  assert_not_null(stdout); assert_false(cdp_record_is_empty(stdout));

    // Link pipeline in reverse (upstream) order
    cdpRecord* in1 = cdp_cascade_get_inlet(cdp_root(), stdout, cdp_text_to_word("in1"));                assert_not_null(in1);
    cdpRecord* num = cdp_cascade_get_inlet(cdp_root(), adder,  cdp_text_to_word("num"));                assert_not_null(num);
    cdpRecord* tic = cdp_cascade_get_inlet(cdp_root(), stdin,  cdp_text_to_word("tic"));                assert_not_null(tic);

    cdpRecord* ans = cdp_cascade_connect(cdp_root(), adder,            cdp_text_to_word("ans"), in1);   assert_not_null(ans);
    cdpRecord* inp = cdp_cascade_connect(cdp_root(), stdin,            cdp_text_to_word("inp"), num);   assert_not_null(inp);
    cdpRecord* stc = cdp_cascade_connect(cdp_root(), cdp_agent_step(), cdp_text_to_word("tic"), tic);   assert_not_null(stc);

    // Execute pipeline
    while (!DONE) {
        cdp_system_step();
    }

    cdp_cascade_unplug(cdp_root(), cdp_agent_step(), stc);
    cdp_cascade_unplug(cdp_root(), stdin, inp);
    cdp_cascade_unplug(cdp_root(), adder, ans);

    cdp_record_remove(pipeline, NULL);

    return MUNIT_OK;
}

