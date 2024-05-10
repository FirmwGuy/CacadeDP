/*
 *  Copyright (c) 2024 Victor M. Barrientos <firmw.guy@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
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


#include "cdp_process.h"




cdpRecord* NONE;
extern cdpRecord ROOT;
cdpRecord* SYSTEM;
cdpRecord* NAME;
cdpRecord* TYPE;
cdpRecord* INSTANCE;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* PROCESS;
cdpRecord* NETWORK;


#define NAME_NONE       "None"

#define NAME_ROOT       "/"
#define NAME_SYSTEM     "system"
#define NAME_NAME       "name"
#define NAME_TYPE       "type"
#define NAME_INSTANCE   "instance"
#define NAME_USER       "user"
#define NAME_PRIVATE    "private"
#define NAME_PUBLIC     "public"
#define NAME_DATA       "data"
#define NAME_PROCESS    "process"
#define NAME_NETWORK    "network"

#define NAME_BOOLEAN    "Boolean"
#define NAME_UINT8      "Binary/Unsigned; size=8"
#define NAME_UINT16     "Binary/Unsigned; size=16"
#define NAME_UINT32     "Binary/Unsigned"
#define NAME_UINT64     "Binary/Unsigned; size=64"
#define NAME_UINT128    "Binary/Unsigned; size=128"
#define NAME_SIGN8      "Binary/Integer; size=8"
#define NAME_SIGN16     "Binary/Integer; size=16"
#define NAME_SIGN32     "Binary/Integer"
#define NAME_SIGN64     "Binary/Integer; size=64"
#define NAME_SIGN128    "Binary/Integer; size=128"
#define NAME_FLOAT32    "Binary/Float"
#define NAME_FLOAT64    "Binary/Float; size=64"
#define NAME_FLOAT80    "Binary/Float; size=80; format=x87"
#define NAME_UTF8       "Text/UTF"




cdpRecord* cdp_process_load(const char* name,
                            cdpCreate   create,
                            cdpTic      tic,
                            cdpSave     save,
                            cdpLoad     load,
                            cdpDestroy  destroy) {
}





cdpNameID cdp_system_enter_name(const char* name, size_t length) {
}

cdpNameID cdp_system_enter_name_static(const char* name, size_t length) {
}



cdpRecord* cdp_system_name(cdpNameID) {
}




unsigned cdp_system_enter_type(cdpNameID nameID, size_t baseSize) {
}



cdpRecord* cdp_system_type(unsigned typeID) {
}




void cdp_system_initiate(void) {
    assert(!SYSTEM);
    cdp_record_system_initiate();
    
    SYSTEM = cdp_record_add_dictionary(&ROOT, CDP_ID_SYSTEM, CDP_ID_SYSTEM, CDP_STO_CHD_ARRAY, NULL, NULL, 2); {
        NAME = cdp_record_add_book(SYSTEM, CDP_ID_NAME, CDP_ID_NAME, CDP_STO_CHD_PACKED_QUEUE, 32); {
            /* This must be done in the exact same order of the _CDP_ID enumeration (since each nameID
               is just an index to this book entry).
               WARNING: Please keep it in sync with cdp_process.h!
            */
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_NONE, strlen(NAME_NONE));
            
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_ROOT,        strlen(NAME_ROOT));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_SYSTEM,      strlen(NAME_SYSTEM));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_NAME,        strlen(NAME_NAME));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_TYPE,        strlen(NAME_TYPE));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_INSTANCE,    strlen(NAME_INSTANCE));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_USER,        strlen(NAME_USER));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_PRIVATE,     strlen(NAME_PRIVATE));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_PUBLIC,      strlen(NAME_PUBLIC));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_DATA,        strlen(NAME_DATA));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_PROCESS,     strlen(NAME_PROCESS));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_NETWORK,     strlen(NAME_NETWORK));
            
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_BOOLEAN, strlen(NAME_BOOLEAN));
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
            cdp_record_add_register(NAME, CDP_ID_NAME, CDP_ID_UTF8, true, NAME_, strlen());
                
    // Register types
    CDP_ID_BOOLEAN,
    CDP_ID_UINT8,
    CDP_ID_UINT16,
    CDP_ID_UINT32,
    CDP_ID_UINT64,
    CDP_ID_UINT128,
    CDP_ID_SIGN8,
    CDP_ID_SIGN16,
    CDP_ID_SIGN32,
    CDP_ID_SIGN64,
    CDP_ID_SIGN128,
    CDP_ID_FLOAT32,
    CDP_ID_FLOAT64,
    CDP_ID_FLOAT80,
    CDP_ID_UTF8,
            
        }
        
        TYPE = cdp_record_add_book(SYSTEM, CDP_ID_TYPE, CDP_ID_TYPE, CDP_STO_CHD_PACKED_QUEUE, 8);
    }
    INSTANCE = cdp_record_add_dictionary(&ROOT, CDP_ID_SYSTEM, CDP_ID_SYSTEM, CDP_STO_CHD_ARRAY, NULL, NULL, 2);    
    
}


bool cdp_system_tic(void) {
    assert(SYSTEM);
}



void cdp_system_shutdown(void) {
    assert(SYSTEM);
    cdp_record_system_shutdown();
}



