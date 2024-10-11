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

#ifndef CDP_DOMAIN_BINARY_H
#define CDP_DOMAIN_BINARY_H


#include "cdp_record.h"


CDP_METADATA_STRUCT(cdpBinary,
    uint16_t    shift:      1,      // Role shift.

                size:       4,      // Power of 2 exponent describing the machine value byte size.
                sign:       1,      // Is it signed (1) or unsigned {0}?
                endianess:  1,      // Little endian (0) is the norm.
                dimension:  2,      // Number of dimensions (scalar, vector, etc).
                // -----------

                compression:2,      // Type of compression used to pack content.
                encryption: 2,      // Encryption method.

                _reserved:  3;      // ToDo: expand to OS and HW related stuff.
    uint16_t    _reserved2;
);


enum _cdpBinaryRole {
    CDP_ROLE_BIN_BOOLEAN,       // True or false value.
    CDP_ROLE_BIN_INDEX,         // Index/enumeration where values translate to meaning.
    CDP_ROLE_BIN_ADDRESS,       // Local memory pointer, address, size or offset.
    CDP_ROLE_BIN_INTEGER,       // Integer value.
    CDP_ROLE_BIN_DECIMAL,       // Decimal floating point value.
    CDP_ROLE_BIN_FLOAT,         // Binary floating point value.
    CDP_ROLE_BIN_COMPLEX,       // Binary complex floating point value.
};

enum _cdpBinaryRoleShited {
    CDP_ROLE_BIN_LOGICAL_OP,    // Logical/comparison operation (AND, LT/GT, etc).
    CDP_ROLE_BIN_BITWISE_OP,    // Bitwise operation (SHIFT, POPCOUNT, etc).
    CDP_ROLE_BIN_MATH_OP,       // Mathematical operation (ADD, COS, etc).

    CDP_ROLE_BIN_CONTAINER = 4, // An opaque memory block, buffer or binary stream.
    CDP_ROLE_BIN_DEVICE,        // A hardware device (port, adapter, etc).
    CDP_ROLE_BIN_FILE,          // A binary (raw format) file.
};

enum _cdpBinarySize {
    CDP_BITESIZE_1,
    CDP_BITESIZE_2,
    CDP_BITESIZE_4,
    CDP_BITESIZE_8,
    CDP_BITESIZE_16,
};

enum _cdpBinaryDimension {
    CDP_DIM_SCALAR,
    CDP_DIM_VECTOR,
    CDP_DIM_MATRIX,
    CDP_DIM_TENSOR
};

enum _cdpBinaryCompression {
    CDP_COMPRESS_NONE,      // Uncompressed content.
    CDP_COMPRESS_RLE,       // Run-length encoding.
    CDP_COMPRESS_ZIP,       // Zip (deflate) method.
    CDP_COMPRESS_LZW        // 7z kind of compression.
};

enum _cdpBinaryEncryption {
    CDP_CRYPT_NONE,         // Unencrypted content.
    CDP_CRYPT_AES,          // Advanced encryption standard.
    CDP_CRYPT_RSA,          // Rivest-Shamir-Adleman.
    CDP_CRYPT_SHA           // Secure hash algorithm.
};


static inline cdpBinary cdp_binary_metadata_link(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_LINK,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_ADDRESS,
        .size   = cdp_ctz(sizeof(void*))    // Same as log2(sizeof(void*)).
    };
}

static inline cdpBinary cdp_binary_metadata_agent(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_AGENT,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_ADDRESS,
        .size   = cdp_ctz(sizeof(cdpAgent)) // Same as log2(sizeof(cdpAgent)).
    };
}

static inline cdpBinary cdp_binary_metadata_boolean(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_BOOLEAN,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_BOOLEAN,
        .size   = cdp_ctz(sizeof(uint8_t)).
    };
}

static inline cdpBinary cdp_binary_metadata_uint32(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_UINT32,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_INTEGER,
        .size   = cdp_ctz(sizeof(uint32_t)).
    };
}

static inline cdpBinary cdp_binary_metadata_uint64(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_UINT64,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_INTEGER,
        .size   = cdp_ctz(sizeof(uint64_t)).
    };
}

static inline cdpBinary cdp_binary_metadata_int32(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_INT32,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_INTEGER,
        .size   = cdp_ctz(sizeof(int32_t)),
        .sign   = 1
    };
}

static inline cdpBinary cdp_binary_metadata_int64(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_INT64,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_INTEGER,
        .size   = cdp_ctz(sizeof(int64_t)),
        .sign   = 1
    };
}

static inline cdpBinary cdp_binary_metadata_float32(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_FLOAT32,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_FLOAT,
        .size   = cdp_ctz(sizeof(float)),
        .sign   = 1
    };
}

static inline cdpBinary cdp_binary_metadata_float64(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_FLOAT64,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_FLOAT,
        .size   = cdp_ctz(sizeof(double)),
        .sign   = 1
    };
}


static inline cdpBinary cdp_binary_metadata_float64(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = CDP_TAG_BIN_FLOAT64,

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_FLOAT,
        .size   = cdp_ctz(sizeof(double)),
        .sign   = 1
    };
}


#endif
