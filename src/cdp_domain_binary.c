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





// Initial tag IDs (for a description see cdp_agent.h):
enum _cdpBinaryTagID {
    // Children
    CDP_TAG_BIN_LENGTH,     // Arbitrary length of a vector.
    CDP_TAG_BIN_LENGTH2D,
    CDP_TAG_BIN_LENGTH3D,
    CDP_TAG_BIN_LENGTH4D,

    CDP_TAG_BIN_TENSOR_ORD, // Tensor order (if over 4 dimensions).
    CDP_TAG_BIN_TENSOR_LEN, // A vector with arbitrary dimension lengths for a tensor.
    //

    CDP_TAG_BIN_LINK,           // Link to other record.
    CDP_TAG_BIN_AGENT,          // Address of agent function.
    CDP_TAG_BIN_TAG,
    CDP_TAG_BIN_ID,
    CDP_TAG_BIN_PATCH,

    CDP_TAG_BIN_BOOLEAN,

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
    CDP_TAG_BIN_COMPLEX32,      // Complex number as an array of 2 Float32.
    CDP_TAG_BIN_COMPLEX64,      // Complex number as an array of 2 Float64.
    CDP_TAG_BIN_COMPLEX128,     // Complex number as an array of 2 Float128.
    CDP_TAG_BIN_VECT2D,         // Vector of 2 Float32.
    CDP_TAG_BIN_VECT3D,         // Vector of 3 Float32.
    CDP_TAG_BIN_VECT4D,         // Vector of 4 Float32.

    CDP_TAG_BIN_CRC16,
    CDP_TAG_BIN_CRC32,

    CDP_TAG_BIN_MURMUR64,
    CDP_TAG_BIN_MURMUR128,

    CDP_TAG_BIN_AND,
    CDP_TAG_BIN_OR,
    CDP_TAG_BIN_NOT,

    CDP_TAG_BIN_ADD,
    CDP_TAG_BIN_SUBSTRACT,
    CDP_TAG_BIN_MULTIPLY,
    CDP_TAG_BIN_DIVIDE,

    CDP_TAG_BINARY_COUNT
};
