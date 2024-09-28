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


CDP_ATTRIBUTE_STRUCT(cdpBinaryAttribute,
    uint16_t    shift:      1,      // Role shift.

                size:       4,      // Power of 2 exponent describing the native byte size.
                sign:       1,      // Is signed or unsigned.
                endianess:  1,      // Little endian (0) is the norm.
                dimension:  2,      // Number of dimensions (scalar, vector, etc).
                compression:2,      // Type of compression used to pack content.
                encryption: 2,      // Encryption method.
                // -----------

                _reserved:  3;

    uint16_t    _available;         // Available for custom user-defined attribute/value storage.
);




enum _cdpBinaryRole {
    CDP_ROLE_BIN_ENUMERATION,   // Indexed enumeration where values translate to meaning.
    CDP_ROLE_BIN_BOOLEAN,       // True or false value.
    CDP_ROLE_BIN_ADDRESS,       // Local memory pointer, address, size or offset.
    CDP_ROLE_BIN_LENGTH,        // Length of a specific dimension.
    CDP_ROLE_BIN_INTEGER,       // Integer value.
    CDP_ROLE_BIN_DECIMAL,       // Decimal floating point value.
    CDP_ROLE_BIN_FLOAT,         // Binary floating point value.
    CDP_ROLE_BIN_COMPLEX        // Binary complex floating point value.
};

enum _cdpBinaryRoleShited {
    CDP_ROLE_BIN_LOGICAL_OP,    // Logical/comparison operation (AND, LT/GT, etc).
    CDP_ROLE_BIN_BITWISE_OP,    // Bitwise operation (SHIFT, POPCOUNT, etc).
    CDP_ROLE_BIN_MATH_OP,       // Mathematical operation (ADD, COS, etc).

    CDP_ROLE_BIN_CONTAINER = 4, // An opaque memory block, buffer or binary stream.
    CDP_ROLE_BIN_DEVICE,        // A hardware device (port, adapter, etc).
    CDP_ROLE_BIN_FILE,          // A binary (raw format) file.
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


// Initial tag IDs (for a description see cdp_agent.h):
enum _cdpBinaryTagID {
    // Children
    CDP_TAG_BIN_LENGTH,     // Arbitrary length of a vector.
    CDP_TAG_BIN_LENGTH2D,
    CDP_TAG_BIN_LENGTH3D,
    CDP_TAG_BIN_LENGTH4D,
    //

    CDP_TAG_BIN_BYTE,
    CDP_TAG_BIN_UINT16,
    CDP_TAG_BIN_UINT32,
    CDP_TAG_BIN_UINT64,
    CDP_TAG_BIN_UINT128,
    CDP_TAG_BIN_INT16,
    CDP_TAG_BIN_INT32,
    CDP_TAG_BIN_INT64,
    CDP_TAG_BIN_INT128,
    CDP_TAG_BIN_DECIMAL32,
    CDP_TAG_BIN_DECIMAL64,
    CDP_TAG_BIN_DECIMAL128,
    CDP_TAG_BIN_FLOAT32,
    CDP_TAG_BIN_FLOAT64,
    CDP_TAG_BIN_FLOAT128,
    CDP_TAG_BIN_COMPLEX32,      // Complex number as a vector of 2 Float32.
    CDP_TAG_BIN_COMPLEX64,      // Complex number as a vector of 2 Float64.
    CDP_TAG_BIN_COMPLEX128,     // Complex number as a vector of 2 Float128.
    CDP_TAG_BIN_VECT2D,         // Vector of 2 Float32.
    CDP_TAG_BIN_VECT3D,         // Vector of 3 Float32.
    CDP_TAG_BIN_VECT4D,         // Vector of 4 Float32.

    CDP_TAG_BIN_CRC16,
    CDP_TAG_BIN_CRC32,

    CDP_TAG_BIN_MURMUR64,
    CDP_TAG_BIN_MURMUR128,

    CDP_TAG_BIN_TAG,
    CDP_TAG_BIN_ID,
    CDP_TAG_BIN_PATCH,

    CDP_TAG_BINARY_COUNT
};


#endif
