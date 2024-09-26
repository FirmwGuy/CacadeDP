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

#ifndef CDP_DOM_BINARY_H
#define CDP_DOM_BINARY_H


#include "cdp_record.h"


CDP_ATTRIBUTE_STRUCT(cdpBinaryAttribute,
    cdpAttribute    shift:      1,      // Role shift.

                    size:       4,      // Power of 2 exponent describing the native byte size.
                    sign:       1,      // Is signed or unsigned.
                    endianess:  1,      // Little endian (0) is the norm.
                    dimension:  2,      // Number of dimensions (scalar, vector, etc).
                    compression:2,      // Type of compression used to pack content.
                    encryption: 2,      // Encryption method.
                    // -----------

                    immediate:  1;      // Register value is inside the data pointer of this very record.
);

typedef struct {
    size_t  size;           // Data size in bytes.
    size_t  capacity;       // Buffer capacity in bytes.
    void*   data;           // Pointer to data.
    cdpDel  destructor;     // Destructor function.
    //size_t  refCount;
} cdpData;


enum _cdpBinaryRole {
    CDP_ROLE_ENUMERATION,   // Indexed enumeration where values translate to meaning.
    CDP_ROLE_BOOLEAN,       // True or false value.
    CDP_ROLE_ADDRESS,       // Local memory pointer, address, size or offset.
    CDP_ROLE_INTEGER,       // Integer value.
    CDP_ROLE_DECIMAL,       // Decimal floating point value.
    CDP_ROLE_FLOAT,         // Binary floating point value.
    CDP_ROLE_COMPLEX,       // Binary complex floating point value.
};

enum _cdpBinaryRoleShited {
    CDP_ROLE_LOGICAL_OP,    // Logical/comparison operation (AND, LT/GT, etc).
    CDP_ROLE_BITWISE_OP,    // Bitwise operation (SHIFT, POPCOUNT, etc).
    CDP_ROLE_MATH_OP,       // Mathematical operation (ADD, COS, etc).

    CDP_ROLE_CONTAINER,     // A memory block, buffer or binary stream.
    CDP_ROLE_DEVICE,        // A hardware device (port, adapter, etc).
    CDP_ROLE_FILE,          // A binary (raw format) file.
};

// Initial tag IDs (for a description see cdp_agent.h):
enum _cdpBinaryTagID {
    CDP_TAG_BYTE,
    CDP_TAG_UINT16,
    CDP_TAG_UINT32,
    CDP_TAG_UINT64,
    CDP_TAG_UINT128,
    CDP_TAG_INT16,
    CDP_TAG_INT32,
    CDP_TAG_INT64,
    CDP_TAG_INT128,
    CDP_TAG_DECIMAL32,
    CDP_TAG_DECIMAL64,
    CDP_TAG_DECIMAL128,
    CDP_TAG_FLOAT32,
    CDP_TAG_FLOAT64,
    CDP_TAG_FLOAT128,
    CDP_TAG_COMPLEX32,
    CDP_TAG_COMPLEX64,
    CDP_TAG_COMPLEX128,
    //
    CDP_TAG_TAG,
    CDP_TAG_ID,
    CDP_TAG_PATCH,

    CDP_TAG_BINARY_COUNT
};

enum _cdpBinaryDimension {
    CDP_DIM_SCALAR,
    CDP_DIM_VECTOR,
    CDP_DIM_MATRIX,
    CDP_DIM_TENSOR
};

enum _cdpBinaryCompression {
    CDP_COMPRESS_NONE,      // Uncompressed content.
    CDP_COMPRESS_ZIP,       // Zip (deflate) method.
    CDP_COMPRESS_RLE,       // Run lenght encoding.
    CDP_COMPRESS_LZW        // 7z kind compression.
};

enum _cdpBinaryEncryption {
    CDP_CRYPT_NONE,         // Unencrypted content.
    CDP_CRYPT_AES,          // Advanced encryption standard.
    CDP_CRYPT_RSA,          // Rivest-Shamir-Adleman.
    CDP_CRYPT_SHA           // Secure hash algorithm.
};


#endif
