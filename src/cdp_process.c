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




extern cdpRecord ROOT;

cdpRecord* NONE;
cdpRecord* SYSTEM;
cdpRecord* NAME;
cdpRecord* TYPE;
cdpRecord* INSTANCE;
cdpRecord* USER;
cdpRecord* PUBLIC;
cdpRecord* DATA;
cdpRecord* PROCESS;
cdpRecord* NETWORK;





cdpRecord* cdp_process_load(const char* name,
                            cdpCreate   create,
                            cdpInstance tic,
                            cdpInstance save,
                            cdpInstance restore,
                            cdpInstance destroy) {
}





cdpNameID cdp_system_enter_name(const char* name, size_t length) {
  for (unsigned n=0; n<length; n++) {
      assert(!isupper(name[n]));
  }
}

cdpNameID cdp_system_enter_name_static(const char* name, size_t length) {
}



cdpRecord* cdp_system_name(cdpNameID) {
}




unsigned cdp_system_enter_type(cdpNameID nameID, size_t baseSize) {
}



cdpRecord* cdp_system_type(unsigned typeID) {
}



#define register_name_id(r, str)  cdp_record_add_register(r, CDP_ID_NAME, CDP_ID_UTF8, true, str, strlen(str))

static inline cdpRecord* register_type_id(cdpRecord* r, const char* text, unsigned sizeID, unsigned sizeVal) {
    cdpRecord* type = cdp_record_add_dictionary(r, CDP_ID_TYPE, CDP_ID_TYPE, CDP_STO_CHD_ARRAY, NULL, NULL, 2);
    cdp_record_add_text(type, CDP_ID_NAME, text);
    cdp_record_add_unsigned(type, sizeID, sizeVal);
    return type;
}


void cdp_system_initiate(void) {
    assert(!TYPE);
    cdp_record_system_initiate();
    
    TYPE = cdp_record_add_dictionary(&ROOT, CDP_NAME_TYPE, CDP_TYPE_DICTIONARY, CDP_STO_CHD_ARRAY, NULL, NULL, 2); {
        unsigned idSize;
        
        /*
           The content of NAME and TYPE books must be entered in the exact same order
           of the _CDP_ID enumeration (since each nameID is just an index
           to each book entry).
           WARNING: Please keep it in sync with cdp_process.h!
        */
        
        NAME = cdp_record_add_book(SYSTEM, CDP_ID_NAME, CDP_ID_NAME, CDP_STO_CHD_PACKED_QUEUE, 32); {
            register_name_id(NAME, "none");
            
            register_name_id(NAME, "/");          // Root directory.
            register_name_id(NAME, "system");
            register_name_id(NAME, "name");
            register_name_id(NAME, "type");
            register_name_id(NAME, "instance");
            register_name_id(NAME, "user");
            register_name_id(NAME, "private");
            register_name_id(NAME, "public");
            register_name_id(NAME, "data");
            register_name_id(NAME, "process");
            register_name_id(NAME, "network");
            
            register_name_id(NAME, "boolean");
            register_name_id(NAME, "binary/unsigned; size=8");
            register_name_id(NAME, "binary/unsigned; size=16");
            register_name_id(NAME, "binary/unsigned");
            register_name_id(NAME, "binary/unsigned; size=64");
            register_name_id(NAME, "binary/integer; size=16");
            register_name_id(NAME, "binary/integer");
            register_name_id(NAME, "binary/integer; size=64");
            register_name_id(NAME, "binary/float");
            register_name_id(NAME, "binary/float; size=64");
            register_name_id(NAME, "text/utf");
            
            // The following may be in any order
            idSize = cdp_system_enter_name_string("size");    // Maximum bits per element on each type.
        }
        
        TYPE = cdp_record_add_book(SYSTEM, CDP_ID_TYPE, CDP_ID_TYPE, CDP_STO_CHD_PACKED_QUEUE, 32); {
            NONE = register_type_id(TYPE, CDP_ID_NONE, idSize, 0);
            
            register_type_id(TYPE, CDP_ID_ROOT,     idSize, 0);
            register_type_id(TYPE, CDP_ID_SYSTEM,   idSize, 0);
            register_type_id(TYPE, CDP_ID_NAME,     idSize, 0);
            register_type_id(TYPE, CDP_ID_TYPE,     idSize, 0);
            register_type_id(TYPE, CDP_ID_INSTANCE, idSize, 0);
            register_type_id(TYPE, CDP_ID_USER,     idSize, 0);
            register_type_id(TYPE, CDP_ID_PRIVATE,  idSize, 0);
            register_type_id(TYPE, CDP_ID_PUBLIC,   idSize, 0);
            register_type_id(TYPE, CDP_ID_DATA,     idSize, 0);
            register_type_id(TYPE, CDP_ID_PROCESS,  idSize, 0);
            register_type_id(TYPE, CDP_ID_NETWORK,  idSize, 0);

            register_name_id(NAME, "boolean");
            register_name_id(NAME, "binary/unsigned; size=8");
            register_name_id(NAME, "binary/unsigned; size=16");
            register_name_id(NAME, "binary/unsigned");
            register_name_id(NAME, "binary/unsigned; size=64");
            register_name_id(NAME, "binary/integer; size=16");
            register_name_id(NAME, "binary/integer");
            register_name_id(NAME, "binary/integer; size=64");
            register_name_id(NAME, "binary/float");
            register_name_id(NAME, "binary/float; size=64");
            register_name_id(NAME, "text/utf");

            
            register_type_id(TYPE, CDP_ID_BOOLEAN,  idSize, 1);
            register_type_id(TYPE, CDP_ID_UINT8,    idSize, sizeof(uint8_t));
            register_type_id(TYPE, CDP_ID_UINT16,   idSize, sizeof(uint16_t));
            register_type_id(TYPE, CDP_ID_UINT32,   idSize, sizeof(uint32_t));
            register_type_id(TYPE, CDP_ID_UINT64,   idSize, sizeof(uint64_t));
            register_type_id(TYPE, CDP_ID_SIGN16,   idSize, sizeof(int16_t));
            register_type_id(TYPE, CDP_ID_SIGN32,   idSize, sizeof(int32_t));
            register_type_id(TYPE, CDP_ID_SIGN64,   idSize, sizeof(int64_t));
            register_type_id(TYPE, CDP_ID_FLOAT32,  idSize, sizeof(float));
            register_type_id(TYPE, CDP_ID_FLOAT64,  idSize, sizeof(double));
            register_type_id(TYPE, CDP_ID_UTF8,     idSize, 4 * sizeof(uint8_t));
        }
    }
    
    INSTANCE = cdp_record_add_dictionary(&ROOT, CDP_ID_INSTANCE, CDP_ID_INSTANCE, CDP_STO_CHD_RED_BLACK_T, NULL, NULL);    
    USER     = cdp_record_add_dictionary(&ROOT, CDP_ID_USER,     CDP_ID_USER,     CDP_STO_CHD_RED_BLACK_T, NULL, NULL);    
    PUBLIC   = cdp_record_add_dictionary(&ROOT, CDP_ID_PUBLIC,   CDP_ID_PUBLIC,   CDP_STO_CHD_RED_BLACK_T, NULL, NULL);    
    DATA     = cdp_record_add_dictionary(&ROOT, CDP_ID_DATA,     CDP_ID_DATA,     CDP_STO_CHD_RED_BLACK_T, NULL, NULL);    
    PROCESS  = cdp_record_add_dictionary(&ROOT, CDP_ID_PROCESS,  CDP_ID_PROCESS,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);    
    NETWORK  = cdp_record_add_dictionary(&ROOT, CDP_ID_NETWORK,  CDP_ID_NETWORK,  CDP_STO_CHD_RED_BLACK_T, NULL, NULL);    
    
}


bool cdp_system_tic(void) {
    assert(TYPE);
}



void cdp_system_shutdown(void) {
    assert(TYPE);
    cdp_record_system_shutdown();
}



