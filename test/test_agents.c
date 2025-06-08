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




bool DONE;




static int agent_stdin(cdpRecord* client, void** returned, cdpRecord* self, unsigned action, cdpRecord* record, cdpValue value) {
    assert(client && self);
    static cdpRecord* inp;

    switch (action) {
      case CDP_ACTION_INSTANCE_NEW: {
        cdp_record_set_data(self, cdp_data_new_value(CDP_ACRO("CDP"), CDP_ACRO("FLOAT64"), (cdpID)0, (cdpID)0, sizeof(double), 0.0));
        cdp_record_set_store(self, cdp_store_new(CDP_ACRO("CDP"), CDP_WORD("list"), CDP_STORAGE_LINKED_LIST, CDP_INDEX_BY_NAME));
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_INSTANCE_INLET: {
        assert_uint64(value.id, ==, CDP_WORD("tic"));
        CDP_PTR_SEC_SET(returned, self);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_INSTANCE_CONNECT: {
        assert_uint64(value.id, ==, CDP_WORD("inp"));
        inp = cdp_dict_add_link(self, value.id, record);
        CDP_PTR_SEC_SET(returned, inp);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_INSTANCE_UNPLUG: {
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
                cdp_instance_data_update(client, inp, sizeof(d), sizeof(d), CDP_V(d));
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
      case CDP_ACTION_INSTANCE_NEW: {
        cdp_record_set_data(self, cdp_data_new_value(CDP_ACRO("CDP"), CDP_ACRO("FLOAT64"), (cdpID)0, (cdpID)0, sizeof(double), 0.0));

        // FixMe! FixMe! FixMe!
        //cdp_record_set_store(self, cdp_store_new(CDP_ACRO("CDP"), CDP_WORD("list"), CDP_STORAGE_ARRAY, CDP_INDEX_BY_NAME, 2));
        cdp_record_set_store(self, cdp_store_new(CDP_ACRO("CDP"), CDP_WORD("list"), CDP_STORAGE_LINKED_LIST, CDP_INDEX_BY_NAME));
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_INSTANCE_INLET: {
        assert_uint64(value.id, ==, cdp_text_to_word("num"));
        num = cdp_dict_add_value(self, value.id, CDP_ACRO("CDP"), CDP_WORD("adder"), (cdpID)0, (cdpID)0, 0.0, sizeof(double), sizeof(double));
        cdp_data_add_agent(num->data, CDP_ACRO("CDP"), CDP_WORD("adder"), cdp_system_agent(CDP_ACRO("CDP"), CDP_WORD("adder")));
        CDP_PTR_SEC_SET(returned, num);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_INSTANCE_CONNECT: {
        assert_uint64(value.id, ==, cdp_text_to_word("ans"));
        ans = cdp_dict_add_link(self, value.id, record);
        CDP_PTR_SEC_SET(returned, ans);
        return CDP_STATUS_SUCCESS;
      }
      case CDP_ACTION_INSTANCE_UNPLUG: {
        cdp_record_remove(num, NULL);
        num = NULL;
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_DATA_UPDATE: {
        cdpRecord* adder = cdp_record_parent(self);
        cdp_record_update_value(num, sizeof(double), value);

        double d = value.float64 + cdp_record_value(adder).float64;

        cdp_record_update_value(adder, sizeof(d), CDP_V(d));
        cdp_instance_data_update(client, ans, sizeof(d), sizeof(d), CDP_V(d));
        return CDP_STATUS_SUCCESS;
      }
    }

    return CDP_STATUS_OK;
}




static int agent_stdout(cdpRecord* client, void** returned, cdpRecord* self, unsigned action, cdpRecord* record, cdpValue value) {
    assert(client && self);

    switch (action) {
      case CDP_ACTION_INSTANCE_NEW: {
        cdp_record_set_data(self, cdp_data_new_value(CDP_ACRO("CDP"), CDP_ACRO("FLOAT64"), (cdpID)0, (cdpID)0, sizeof(double), 0.0));
        return CDP_STATUS_SUCCESS;
      }

      case CDP_ACTION_INSTANCE_INLET: {
        assert_uint64(value.id, ==, CDP_ACRO("IN1"));
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
    cdp_agency_set_agent(CDP_WORD("test"), CDP_WORD("stdin"), CDP_WORD("system-step"), agent_stdin);
    cdp_agency_set_produ(CDP_WORD("test"), CDP_WORD("stdin"), CDP_WORD("number"));

    cdp_agency_set_agent(CDP_WORD("test"), CDP_WORD("adder"), CDP_WORD("operand"), agent_adder);
    cdp_agency_set_produ(CDP_WORD("test"), CDP_WORD("adder"), CDP_WORD("answer"));

    cdp_agency_set_agent(CDP_WORD("test"), CDP_WORD("stdout"), CDP_WORD("number"), agent_stdout);

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
    cdpRecord* instances = cdp_dict_add_list(CASCADE, CDP_AUTOID, CDP_ACRO("CDP"), CDP_WORD("list"), CDP_STORAGE_LINKED_LIST);       assert_not_null(instances);

    cdpRecord* self    = cdp_dict_add_agency_instance(instances, CDP_ACRO("INST00"), CDP_ACRO("CDP"), CDP_WORD("self"),   NULL);     assert_not_null(stdinp);

    cdpRecord* stdinp  = cdp_dict_add_agency_instance(instances, CDP_ACRO("INST01"), CDP_ACRO("CDP"), CDP_WORD("stdin"),  NULL);     assert_not_null(stdinp);
    cdpRecord* adder   = cdp_dict_add_agency_instance(instances, CDP_ACRO("INST02"), CDP_ACRO("CDP"), CDP_WORD("adder"),  NULL);     assert_not_null(adder);
    cdpRecord* stdoutp = cdp_dict_add_agency_instance(instances, CDP_ACRO("INST03"), CDP_ACRO("CDP"), CDP_WORD("stdout"), NULL));    assert_not_null(stdoutp);

    // Link pipeline
    bool status;
    cdpRecord* systep = cdp_system_step_instance();

    cdp_agency_pipeline_create(self, CDP_WORD("my_pipeline"));

    status = cdp_agency_product_connect(self, CDP_WORD("my_pipeline"), systep, CDP_WORD("system-step"), stdinp, CDP_WORD("system-step"));   assert_true(status);
    status = cdp_agency_product_connect(self, CDP_WORD("my_pipeline"), stdinp, CDP_WORD("number"),      adder,  CDP_WORD("operand"));       assert_true(status);
    status = cdp_agency_product_connect(self, CDP_WORD("my_pipeline"), adder,  CDP_WORD("answer"),      stdout, CDP_WORD("number"));        assert_true(status);

    cdp_agency_pipeline_state(self, CDP_WORD("my_pipeline"), CDP_WORD("start"));

    // Execute pipeline
    while (!DONE) {
        cdpRecord* record = cdp_system_step();
        if (!record) {
            //
            break;
        }
    }

    // Terminate instances
    cdp_agency_pipeline_dispose(self, CDP_WORD("my_pipeline"));
    cdp_record_delete(instances);

    return MUNIT_OK;
}

