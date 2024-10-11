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



void cdp_binary_system_initiate(void) {


enum _cdpBinaryTagID {
    // Children
    CDP_TAG_BIN_LENGTH,     // Arbitrary length of a vector.
    CDP_TAG_BIN_LENGTH2D,
    CDP_TAG_BIN_LENGTH3D,

    CDP_TAG_BIN_TENSOR_ORD, // Tensor order (if over 4 dimensions).
    CDP_TAG_BIN_TENSOR_LEN, // A vector with arbitrary dimension lengths for a tensor.

    // Tags
    CDP_TAG_BIN_LINK,           // Link to other record.
    CDP_TAG_BIN_AGENT,          // Address of agent function.
    CDP_TAG_BIN_TAG,
    CDP_TAG_BIN_ID,
    CDP_TAG_BIN_PATCH,

    CDP_TAG_BIN_BYTE,
    CDP_TAG_BIN_BOOLEAN,
    CDP_TAG_BIN_INDEX,
    CDP_TAG_BIN_NUMBER,

    CDP_TAG_BIN_VECTOR2D,
    CDP_TAG_BIN_VECTOR3D,
    CDP_TAG_BIN_VECTOR4D,
    CDP_TAG_BIN_MATRIX2D,
    CDP_TAG_BIN_MATRIX3D,
    CDP_TAG_BIN_MATRIX4D,

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
}

