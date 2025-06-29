/*
 *  Copyright (c) 2024-2025 Victor M. Barrientos
 *  (https://github.com/FirmwGuy/CacadeDP)
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

#ifndef CDP_SYSTEM_H
#define CDP_SYSTEM_H


#include "cdp_record.h"


/*
    Domain:
        'CDP'

    Agencies:
        'step'
        'buffer'
        'cloner'
        'converter'
        'step'
        'synchronizer'

        'data-update'
        'store-add'
        'store-append'

    Statuses:
        'pending'
        'working'
        'completed'
        'failed'

    Events:
        'debug'
        'warning'
        'error'
        'fatal'

    Actions:
        CDP_ACTION_DATA_UPDATE,
        CDP_ACTION_DATA_NEW,
        CDP_ACTION_DATA_DELETE,
        //
        CDP_ACTION_STORE_ADD_ITEM,
        CDP_ACTION_STORE_REMOVE_ITEM,
        CDP_ACTION_STORE_NEW,
        CDP_ACTION_STORE_DELETE,
        //
        CDP_ACTION_INSTANCE_INITIATE,
        CDP_ACTION_INSTANCE_VALIDATE,
        CDP_ACTION_INSTANCE_INLET,
        CDP_ACTION_INSTANCE_CONNECT,
        CDP_ACTION_INSTANCE_UNPLUG,
        CDP_ACTION_INSTANCE_CLEAN,
        //
        CDP_ACTION_PIPELINE_ASSEMBLED,
        CDP_ACTION_PIPELINE_STARTING,
        CDP_ACTION_PIPELINE_RUNNING,
        CDP_ACTION_PIPELINE_PAUSED,
        CDP_ACTION_PIPELINE_COMPLETED,
        //
        CDP_ACTION_REMOTE_INPUT,
        CDP_ACTION_REMOTE_CONNECTED,
        CDP_ACTION_REMOTE_WAITING,
        CDP_ACTION_REMOTE_BLOCKED,
        CDP_ACTION_REMOTE_INTERRUPTED,
        CDP_ACTION_REMOTE_ERROR,
        CDP_ACTION_REMOTE_FAILED,

    Status:
        CDP_STATUS_FAIL = -1,
        CDP_STATUS_OK,
        CDP_STATUS_PROGRESS,
        CDP_STATUS_SUCCESS

    Logs:
         CDP_LOG_DEBUG,
        CDP_LOG_LOG,
        CDP_LOG_WARNING,
        CDP_LOG_ERROR,
        CDP_LOG_FATAL


    Core directories:
        'data'
        'network'
        'public'
        'private'
        'system'
            'agent'
            'cascade'
            'domain'
            'library'
        'temp'
        'user'

*/


typedef bool (*cdpAgent)(cdpRecord* instance, cdpRecord* call);


bool  cdp_system_startup(void);
bool  cdp_system_step(void);
void  cdp_system_shutdown(void);


bool cdp_agency_set_agent(cdpDT* agency, cdpDT* input, cdpAgent agent);
bool cdp_agency_set_output(cdpDT* agency, cdpDT* output);

cdpRecord* cdp_record_add_agency_instance(  cdpRecord* record, cdpDT* name, uintptr_t context,
                                            cdpDT* agency, cdpRecord* args, cdpRecord* client   );
#define cdp_dict_add_agency_instance(dict, name, agency, args, client)      cdp_record_add_agency_instance(dict, name, 0, agency, args, client)

bool cdp_agency_instance_message(cdpRecord* instance, cdpDT* input, cdpRecord* message);
void cdp_agency_instance_dispose(cdpRecord* instance);

bool cdp_agency_pipeline_create(cdpRecord* selfI, cdpDT* name);
bool cdp_agency_pipeline_message(cdpRecord* selfI, cdpDT* pipeline, cdpDT* input, cdpRecord* message);
bool cdp_agency_pipeline_dispose(cdpRecord* selfI, cdpDT* pipeline);

bool cdp_agency_client_message(cdpRecord* selfI, cdpDT* input, cdpRecord* message);

bool cdp_agency_output_connect( cdpRecord* selfI, cdpDT* pipeline, 
                                cdpRecord* sourceI, cdpDT* output,
                                cdpRecord* targetI, cdpDT* input    );
bool cdp_agency_output_message(cdpRecord* selfI, cdpDT* output, cdpRecord* message);

#define cdp_agency_instance_valid(instance)     (!cdp_record_is_floating(instance) && cdp_store_is_dictionary(instance))


//static inline cdpRecord* cdp_void(void)  {extern cdpRecord* CDP_VOID; assert(CDP_VOID);  return CDP_VOID;}

//static inline cdpRecord* cdp_agent_step(void)  {extern cdpRecord* CDP_STEP; assert(CDP_STEP);  return CDP_STEP;}


#endif
