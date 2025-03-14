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
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
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



#define CDP_WORD_STDIN      CDP_IDC(0x004E844B80000000)     /* "stdin"       */
#define CDP_WORD_ADDER      CDP_IDC(0x0004842C80000000)     /* "adder"       */
#define CDP_WORD_STDOUT     CDP_IDC(0x004E847D68000000)     /* "stdout"      */

bool DONE;




static int agent_stdin(cdpRecord* client, void** returned, cdpRecord* self, unsigned action, cdpRecord* record, cdpValue value) {
    assert(client && self);
    static cdpRecord* inp;

    switch (action) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(self, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("FLOAT64"), (cdpID)0, sizeof(double), 0.0));
        CDP_PTR_SEC_SET(returned, self->data);
        return CDP_STATUS_PROGRESS;
      }
      case CDP_ACTION_STORE_NEW: {
        cdp_record_set_store(self, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_LINKED_LIST, CDP_INDEX_BY_NAME));
        CDP_PTR_SEC_SET(returned, self->store);
        return CDP_STATUS_PROGRESS;
      }

      case CDP_ACTION_CONTEXT_INLET: {
        assert_uint64(value.id, ==, cdp_text_to_word("tic"));
        CDP_PTR_SEC_SET(returned, self);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_CONTEXT_CONNECT: {
        assert_uint64(value.id, ==, cdp_text_to_word("inp"));
        inp = cdp_dict_add_link(self, value.id, record);
        CDP_PTR_SEC_SET(returned, inp);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_CONTEXT_UNPLUG: {
        cdp_record_delete_children(self);
        inp = NULL;
        return CDP_STATUS_SUCCESS;
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
        return CDP_STATUS_SUCCESS;
      }
    }

    return CDP_STATUS_OK;
}



static int agent_adder(cdpRecord* client, void** returned, cdpRecord* self, unsigned action, cdpRecord* record, cdpValue value) {
    assert(client && self);

    static cdpRecord* num;
    static cdpRecord* ans;

    switch (action) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(self, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("FLOAT64"), (cdpID)0, sizeof(double), 0.0));
        CDP_PTR_SEC_SET(returned, self->data);
        return CDP_STATUS_PROGRESS;
      }
      case CDP_ACTION_STORE_NEW: {
        // FixMe! FixMe! FixMe!
        //cdp_record_set_store(self, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_ARRAY, CDP_INDEX_BY_NAME, 2));
        cdp_record_set_store(self, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_LINKED_LIST, CDP_INDEX_BY_NAME));
        CDP_PTR_SEC_SET(returned, self->store);
        return CDP_STATUS_PROGRESS;
      }

      case CDP_ACTION_CONTEXT_INLET: {
        assert_uint64(value.id, ==, cdp_text_to_word("num"));
        num = cdp_dict_add_value(self, value.id, CDP_ACRON_CDP, CDP_WORD_ADDER, (cdpID)0, 0.0, sizeof(double), sizeof(double));
        cdp_data_add_agent(num->data, CDP_ACRON_CDP, CDP_WORD_ADDER, cdp_system_agent(CDP_ACRON_CDP, CDP_WORD_ADDER));
        CDP_PTR_SEC_SET(returned, num);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_CONTEXT_CONNECT: {
        assert_uint64(value.id, ==, cdp_text_to_word("ans"));
        ans = cdp_dict_add_link(self, value.id, record);
        CDP_PTR_SEC_SET(returned, ans);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_CONTEXT_UNPLUG: {
        cdp_record_remove(num, NULL);
        num = NULL;
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_DATA_UPDATE: {
        cdpRecord* adder = cdp_record_parent(self);
        cdp_record_update_value(num, sizeof(double), value);

        double d = value.float64 + cdp_record_value(adder).float64;

        cdp_record_update_value(adder, sizeof(d), CDP_V(d));
        cdp_cascade_data_update(client, ans, sizeof(d), sizeof(d), CDP_V(d));
        return CDP_STATUS_SUCCESS;
      }
    }

    return CDP_STATUS_OK;
}




static int agent_stdout(cdpRecord* client, void** returned, cdpRecord* self, unsigned action, cdpRecord* record, cdpValue value) {
    assert(client && self);

    switch (action) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data(self, cdp_data_new_value(CDP_ACRON_CDP, cdp_text_to_acronysm("FLOAT64"), sizeof(double), (cdpID)0, 0.0));
        CDP_PTR_SEC_SET(returned, self->data);
        return CDP_STATUS_PROGRESS;
      }

      case CDP_ACTION_CONTEXT_INLET: {
        assert_uint64(value.id, ==, cdp_text_to_acronysm("IN1"));
        CDP_PTR_SEC_SET(returned, self);
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_DATA_UPDATE: {
        cdp_record_update_value(self, sizeof(value), value);
        printf("%f\n", value.float64);
        return CDP_STATUS_SUCCESS;
      }
    }

    return CDP_STATUS_OK;
}




void* test_agents_setup(const MunitParameter params[], void* user_data) {
    cdp_system_register_agent(CDP_ACRON_CDP, CDP_WORD_STDIN,  agent_stdin);
    cdp_system_register_agent(CDP_ACRON_CDP, CDP_WORD_ADDER,  agent_adder);
    cdp_system_register_agent(CDP_ACRON_CDP, CDP_WORD_STDOUT, agent_stdout);

    cdp_system_startup();

    return NULL;
}


void test_agents_tear_down(void* fixture) {
    cdp_system_shutdown();
}


MunitResult test_agents(const MunitParameter params[], void* user_data_or_fixture) {
    const char* param_value = munit_parameters_get(params, "stdio");
    if (!param_value)
        DONE = true;

    extern cdpRecord* CASCADE;

    // Instance initiation
    cdpRecord* pipeline = cdp_dict_add_list(CASCADE, CDP_AUTOID, CDP_ACRON_CDP, CDP_WORD_LIST, CDP_STORAGE_LINKED_LIST);   assert_not_null(pipeline);

    cdpRecord  child = {0};
    int status;

    cdpRecord* stdinp  = cdp_record_append(pipeline, false, cdp_cascade_record_new_simple(cdp_root(), &child, CDP_WORD_STDIN,  CDP_ACRON_CDP, CDP_WORD_STDIN));     assert_not_null(stdinp);    assert_false(cdp_record_is_empty(stdinp));
    cdpRecord* adder   = cdp_record_append(pipeline, false, cdp_cascade_record_new_simple(cdp_root(), &child, CDP_WORD_ADDER,  CDP_ACRON_CDP, CDP_WORD_ADDER));     assert_not_null(adder);     assert_false(cdp_record_is_empty(adder));
    cdpRecord* stdoutp = cdp_record_append(pipeline, false, cdp_cascade_record_new_simple(cdp_root(), &child, CDP_WORD_STDOUT, CDP_ACRON_CDP, CDP_WORD_STDOUT));    assert_not_null(stdoutp);   assert_false(cdp_record_is_empty(stdoutp));

    // Link pipeline in reverse (upstream) order
    cdpRecord* in1; status = cdp_cascade_context_inlet(cdp_root(), &in1, stdoutp, cdp_text_to_acronysm("IN1"));    assert_int(status, >=, CDP_STATUS_OK);   assert_not_null(in1);
    cdpRecord* num; status = cdp_cascade_context_inlet(cdp_root(), &num, adder,   cdp_text_to_word("num"));        assert_int(status, >=, CDP_STATUS_OK);   assert_not_null(num);
    cdpRecord* tic; status = cdp_cascade_context_inlet(cdp_root(), &tic, stdinp,  cdp_text_to_word("tic"));        assert_int(status, >=, CDP_STATUS_OK);   assert_not_null(tic);

    cdpRecord* ans; status = cdp_cascade_context_connect(cdp_root(), &ans, adder,             cdp_text_to_word("ans"), in1);    assert_int(status, >=, CDP_STATUS_OK);  assert_not_null(ans);
    cdpRecord* inp; status = cdp_cascade_context_connect(cdp_root(), &inp, stdinp,            cdp_text_to_word("inp"), num);    assert_int(status, >=, CDP_STATUS_OK);  assert_not_null(inp);
    cdpRecord* stc; status = cdp_cascade_context_connect(cdp_root(), &stc, cdp_agent_step(),  cdp_text_to_word("tic"), tic);    assert_int(status, >=, CDP_STATUS_OK);  assert_not_null(stc);

    // Execute pipeline
    while (!DONE) {
        cdp_system_step();
    }

    cdp_cascade_context_unplug(cdp_root(), cdp_agent_step(), stc);
    cdp_cascade_context_unplug(cdp_root(), stdinp, inp);
    cdp_cascade_context_unplug(cdp_root(), adder, ans);

    cdp_record_remove(pipeline, NULL);

    return MUNIT_OK;
}

