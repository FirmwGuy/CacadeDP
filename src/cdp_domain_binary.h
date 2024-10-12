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
    cdpAttribute    pow2:       4,  // Power of 2 exponent describing the element scalar size (in bytes).
                    exponent:   1,  // True (1) if not an integer.
                    sign:       1,  // Is it signed (1) or unsigned (0)?
                    dimension:  3,  // Dimensions of data.

                    endianess:  1,  // Little endian (0) is the norm.
                    encoding:   4,  // Binary representation of content.
                    compression:3,  // Type of compression used to pack content.
                    encryption: 3,  // Encryption method.

                    _reserved:  12; // ToDo: expand to streams and communication related stuff.
);


enum _cdpBinaryDimension {
    CDP_BIN_DIM_SCALAR,         // A single value.
    CDP_BIN_DIM_VECTOR2D,       // Vector of 2 values.
    CDP_BIN_DIM_VECTOR3D,
    CDP_BIN_DIM_VECTOR4D,
    CDP_BIN_DIM_MATRIX2D,       // Squared 2x2 matrix.
    CDP_BIN_DIM_MATRIX3D,
    CDP_BIN_DIM_MATRIX4D,

    CDP_BIN_DIM_OTHER = 7
};

enum _cdpBinaryPow2 {
    CDP_BIN_POW2_BYTE1,         // Scalar size is 1 byte.
    CDP_BIN_POW2_BYTE2,         // Scalar size is 2 bytes...
    CDP_BIN_POW2_BYTE4,
    CDP_BIN_POW2_BYTE8,
    CDP_BIN_POW2_BYTE16,

    CDP_BIN_POW2_OTHER = 15
};


enum _cdpBinaryEncoding {
    CDP_BIN_ENC_BYTE,           // Content is an opaque sequence of bytes.
    CDP_BIN_ENC_UNSIGNED,       // All GCC-supported unsigned sizes.
    CDP_BIN_ENC_INTEGER,        // All GCC-supported signed integer sizes.
    CDP_BIN_ENC_GMP,            // LGPL GMP (bignum) library.
    CDP_BIN_ENC_FLOAT,          // IEEE binary representation of floats.
    CDP_BIN_ENC_MPFR,           // LGPL MPFR (bigfloat) library.
    CDP_BIN_ENC_DECIMAL,        // GCC decimal representation.
    CDP_BIN_ENC_MPDECIMAL,      // BSD MPDecimal library.
    CDP_BIN_ENC_COMPLEX,        // GCC representation of complex floats.
    CDP_BIN_ENC_MPC,            // LGPL MPC (bigcomplex) library.
    CDP_BIN_ENC_MATRIX,         // CDP representation of vector/matrices.
    CDP_BIN_ENC_ARRAYFIRE,      // BSD Arrayfire tensor library.
           _
    CDP_BIN_ENC_OTHER = 15
};

enum _cdpBinaryCompression {
    CDP_BIN_COMPRESS_NONE,      // Uncompressed content.
    CDP_BIN_COMPRESS_RLE,       // Run-length encoding.
    CDP_BIN_COMPRESS_ZIP,       // Zip (deflate) method.
    CDP_BIN_COMPRESS_LZW,       // 7z kind of compression.

    CDP_BIN_COMPRESS_OTHER = 15 // Run-length encoding.
};

enum _cdpBinaryEncryption {
    CDP_BIN_CRYPT_NONE,         // Unencrypted content.
    CDP_BIN_CRYPT_AES,          // Advanced encryption standard.
    CDP_BIN_CRYPT_RSA,          // Rivest-Shamir-Adleman.
    CDP_BIN_CRYPT_SHA,          // Secure hash algorithm.

    CDP_BIN_CRYPT_OTHER = 15    // Secure hash algorithm.
};


enum _cdpBinaryTagID {
    // Children
    CDP_TAG_BIN_LENGTH,     // Arbitrary length of a vector.
    CDP_TAG_BIN_LENGTH2D,
    CDP_TAG_BIN_LENGTH3D,

    CDP_TAG_BIN_TENSOR_ORD, // Tensor order (if over 4 dimensions).
    CDP_TAG_BIN_TENSOR_LEN, // A vector with arbitrary dimension lengths for a tensor.

    // Uses
    CDP_TAG_BIN_LINK,           // Link to other record.
    CDP_TAG_BIN_AGENT,          // Address of agent function.
    CDP_TAG_BIN_TAG,
    CDP_TAG_BIN_ID,
    CDP_TAG_BIN_PATCH,

    CDP_TAG_BIN_BYTE,           // Opaque binary representation.
    CDP_TAG_BIN_VALUE,
    CDP_TAG_BIN_INDEX,
    CDP_TAG_BIN_BOOLEAN,
    CDP_TAG_BIN_BITWISE,        // Bitwise/bitmask.
    CDP_TAG_BIN_ADDRESS,        // Local memory/size/offset.

    CDP_TAG_BIN_CRC,
    CDP_TAG_BIN_HASH,

    CDP_TAG_BINARY_COUNT

    // Agencies
    CDP_TAG_BIN_AND,
    CDP_TAG_BIN_OR,
    CDP_TAG_BIN_NOT,

    CDP_TAG_BIN_ADD,
    CDP_TAG_BIN_SUBSTRACT,
    CDP_TAG_BIN_MULTIPLY,
    CDP_TAG_BIN_DIVIDE,

    CDP_TAG_BINARY_COUNT
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
