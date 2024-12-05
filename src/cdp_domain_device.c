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


#include "cdp_domain_binary.h"
#include "cdp_domain_text.h"
#include "cdp_domain_device.h"
#include <raylib.h>



int CREATED;


int agent_window(cdpRecord* client, void** returned, cdpRecord* self, unsigned action, cdpRecord* record, cdpValue value) {
    switch (action) {
      case CDP_ACTION_DATA_NEW: {
        cdp_record_set_data_uint64(self, 0);
        CDP_PTR_SEC_SET(returned, self->data);

        if (!CREATED)
            InitWindow(800, 600, "Test basic window");
        CREATED++;

        return CDP_STATUS_PROGRESS;
      }
      case CDP_ACTION_STORE_NEW: {
        cdp_record_set_store(self, cdp_store_new(CDP_ACRON_CDP, CDP_WORD_WINDOW, CDP_STORAGE_RED_BLACK_T, CDP_INDEX_BY_NAME));
        CDP_PTR_SEC_SET(returned, self->store);

        cdp_dict_add_value(self, cdp_text_to_word(""),

        return CDP_STATUS_PROGRESS;
      }

      case CDP_ACTION_DATA_DELETE: {
        CREATED--;
        if (!CREATED)
            CloseWindow();
        return CDP_STATUS_PROGRESS;
      }

      case CDP_ACTION_DATA_UPDATE: {
        if (!WindowShouldClose())
            return CDP_STATUS_ERROR;

        char counter[16];
        sprintf(counter, "Testing Num: %u", (unsigned));
        BeginDrawing(); {
            ClearBackground(RAYWHITE);
            DrawText(counter, 190, 200, 20, LIGHTGRAY);
        } EndDrawing();
        return CDP_STATUS_SUCCESS;
      }
    }

    return CDP_STATUS_OK;
}


void cdp_device_system_initiate(void) {
    cdp_system_register_agent(CDP_WORD_DEVICE, CDP_WORD_WINDOW, agent_window);
}

