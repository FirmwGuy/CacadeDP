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
    cdpAttribute  pow2:       4,  // Power of 2 exponent describing the element scalar size (in bytes).
                  exponent:   1,  // True (1) if not an integer.
                  sign:       1,  // Is it signed (1) or unsigned (0)?
                  dimension:  2,  // Number of dimensions (scalar, vector, etc).

                  endianess:  1,  // Little endian (0) is the norm.
                  encoding:   4,  // Binary representation of content.
                  compression:3,  // Type of compression used to pack content.
                  encryption: 3,  // Encryption method.

                  _reserved:  13; // ToDo: expand to OS and HW related stuff.
);


enum _cdpBinaryRole {
    CDP_ROLE_BIN_MATH,          // Mathematical meaning (ADD, COS, etc).
    CDP_ROLE_BIN_LOGICAL,       // Logical value/operation meaning (AND, LT/GT, etc).
    CDP_ROLE_BIN_BITWISE,       // Bitwise/bitmask meaning (SHIFT, POPCOUNT, etc).
    CDP_ROLE_BIN_ADDRESS,       // Local memory/size/offset meaning.

    CDP_ROLE_BIN_CONTAINER,     // An opaque memory block, buffer or binary stream.
    CDP_ROLE_BIN_DEVICE,        // A hardware device (port, adapter, etc).
    CDP_ROLE_BIN_FILE,          // A binary (raw format) file.
};

enum _cdpBinaryPow2 {
    CDP_POW2_BYTE1,             // Scalar size is 1 byte.
    CDP_POW2_BYTE2,             // Scalar size is 2 bytes...
    CDP_POW2_BYTE4,
    CDP_POW2_BYTE8,
    CDP_POW2_BYTE16,

    CDP_POW2_OTHER = 15
};

enum _cdpBinaryDimension {
    CDP_DIM_SCALAR,
    CDP_DIM_VECTOR,
    CDP_DIM_MATRIX,

    CDP_DIM_OTHER
};

enum _cdpBinaryEncoding {
    CDP_BINENC_BYTE,            // Content is an opaque sequence of bytes.
    CDP_BINENC_UNSIGNED,        // All GCC-supported unsigned sizes.
    CDP_BINENC_INTEGER,         // All GCC-supported integer sizes.
    CDP_BINENC_GMP,             // LGPL bignum library.
    CDP_BINENC_FLOAT,           // IEEE binary representation of floats.
    CDP_BINENC_MPFR,            // LGPL bigfloat library.
    CDP_BINENC_DECIMAL,         // GCC decimal representation.
    CDP_BINENC_MPDECIMAL,       // MPDecimal library.
    CDP_BINENC_COMPLEX,         // GCC representation of complex floats.
    CDP_BINENC_MPC,             // LGPL bigcomplex library.
    CDP_BINENC_MATRIX,          // CDP representation of vector/matrices.
    CDP_BINENC_ARRAYFIRE,       // Arrayfire BSD-3C tensor library.

    CDP_BINENC_OTHER = 15
};

enum _cdpBinaryCompression {
    CDP_COMPRESS_NONE,          // Uncompressed content.
    CDP_COMPRESS_RLE,           // Run-length encoding.
    CDP_COMPRESS_ZIP,           // Zip (deflate) method.
    CDP_COMPRESS_LZW,           // 7z kind of compression.

    CDP_COMPRESS_OTHER = 15     // Run-length encoding.
};

enum _cdpBinaryEncryption {
    CDP_CRYPT_NONE,             // Unencrypted content.
    CDP_CRYPT_AES,              // Advanced encryption standard.
    CDP_CRYPT_RSA,              // Rivest-Shamir-Adleman.
    CDP_CRYPT_SHA,              // Secure hash algorithm.

    CDP_CRYPT_OTHER = 15        // Secure hash algorithm.
};



static inline size_t cdp_binary_size(cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    REC_DATA_SELECT(record) {
      NONE: {
        assert(cdp_record_has_data(record));  // This shouldn't happen.
        break;
      }
      NEAR: {
        cdpBinary* binary = &record->metadata;
        size_t entries;
        switch (binary->dimension) {
          case CDP_DIM_SCALAR:  entries = 1;  break;
          case CDP_DIM_VECTOR:  entries = 2;  break;    // FixMe: this may be avoided altogether.

          case CDP_DIM_MATRIX:
          case CDP_DIM_SCALAR:
            assert(binary->dimension <= CDP_DIM_VECTOR);
            return 0;
        }
        return (entries * (1 << binary->size));
      }
      DATA:;
      FAR: {
        return record->data.pow2;
      }
    } SELECTION_END;

    return 0;
}


static inline cdpBinary cdp_binary_metadata_link(void) {
    return (cdpBinary) {
        .domain = CDP_DOMAIN_BINARY,
        .tag    = cdp_tag("link"),

        //.abstract =
        //.physical =
        //etc

        .role   = CDP_ROLE_BIN_ADDRESS,
        .pow2   = cdp_ctz(sizeof(void*))    // Same as log2(sizeof(void*)).
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
        .pow2   = cdp_ctz(sizeof(cdpAgent)) // Same as log2(sizeof(cdpAgent)).
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
        .pow2   = cdp_ctz(sizeof(uint8_t)).
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
        .pow2   = cdp_ctz(sizeof(uint32_t)).
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
        .pow2   = cdp_ctz(sizeof(uint64_t)).
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
        .pow2   = cdp_ctz(sizeof(int32_t)),
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
        .pow2   = cdp_ctz(sizeof(int64_t)),
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
        .pow2   = cdp_ctz(sizeof(float)),
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
        .pow2   = cdp_ctz(sizeof(double)),
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
        .pow2  = cdp_ctz(sizeof(double)),
        .sign    = 1
    };
}


#endif
