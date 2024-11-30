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


CDP_CHARACTER_STRUCT( cdpBinary,
    pow2:           4,          // Power of 2 floating describing the element scalar size (in bytes).
    sign:           1,          // Is it signed (1) or unsigned (0)?
    floating:       2,          // Floating point type.
    dimension:      3,          // Dimensions of data.

    endianess:      1,          // Little endian (0) is the norm.
    compression:    3,          // Type of compression used to pack content.
    encryption:     3,          // Encryption method.

    _reserved:      47          // ToDo: expand to streams and communication related stuff.
);


enum _cdpBinaryPow2 {
    CDP_BIN_POW2_BYTE1,         // Scalar size is 8 bits.
    CDP_BIN_POW2_BYTE2,         // Scalar is 16 bits.
    CDP_BIN_POW2_BYTE4,         // 32 bits.
    CDP_BIN_POW2_BYTE8,         // 64 bits.
    CDP_BIN_POW2_BYTE16,        // 128 bits.
    CDP_BIN_POW2_BYTE32,        // 256 bits.
    CDP_BIN_POW2_BYTE64,        // 512 bits.

    CDP_BIN_POW2_OTHER = 15
};


enum _cdpBinaryFloating {
    CDP_BIN_FLOAT_NONE,         // Integer.
    CDP_BIN_FLOAT_DECIMAL,      // Decimal floating point.
    CDP_BIN_FLOAT_BINARY,       // Binary floating point.
    CDP_BIN_FLOAT_COMPLEX       // Binary with imaginary part.
};


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

//~ enum cdpIFDevice {
    //~ // Input device
    //~ CDP_IF_DEVICE_KEYBOARD,     // Keyboard (may be virtual).
    //~ CDP_IF_DEVICE_MOUSE,        // Mouse.
    //~ CDP_IF_DEVICE_TOUCHPAD,     // Touchscreen/touchpad.
    //~ CDP_IF_DEVICE_JOYSTICK,     // Joystick.
    //~ CDP_IF_DEVICE_MICROPHONE,   // User voice command.
    //~ CDP_IF_DEVICE_CAMERA,       // User video input.
    //~ CDP_IF_DEVICE_VR,           // User VR input.

    //~ // Output device
    //~ CDP_IF_DEVICE_SCREEN = 8,   // Screen/display.
    //~ CDP_IF_DEVICE_SPEAKER,      // Speaker/headset.
    //~ CDP_IF_DEVICE_VIBRATOR,     // Vibration.
    //~ CDP_IF_DEVICE_PRINTER,      // User printer.
//~ };


// Domain
#define CDP_WORD_BINARY       CDP_ID(0x00092E0CB2000000)      /* "binary"_____ */

// Uses
#define CDP_WORD_BYTE         CDP_ID(0x000B342800000000)      /* "byte"_______ */
#define CDP_WORD_BOOLEAN      CDP_ID(0x0009EF6142E00000)      /* "boolean"____ */
#define CDP_WORD_ADDRESS      CDP_ID(0x0004849167300000)      /* "address"____ */

#define CDP_ACRON_UINT16      CDP_ID(0x0135A6ED11580000)      /* "UINT16"--- */
#define CDP_ACRON_UINT32      CDP_ID(0x0135A6ED13480000)      /* "UINT32"--- */
#define CDP_ACRON_UINT64      CDP_ID(0x0135A6ED16500000)      /* "UINT64"--- */

#define CDP_ACRON_INT16       CDP_ID(0x0129BB4456000000)      /* "INT16"---- */
#define CDP_ACRON_INT32       CDP_ID(0x0129BB44D2000000)      /* "INT32"---- */
#define CDP_ACRON_INT64       CDP_ID(0x0129BB4594000000)      /* "INT64"---- */

#define CDP_ACRON_FLOAT32     CDP_ID(0x0126B2F8744D2000)      /* "FLOAT32"-- */
#define CDP_ACRON_FLOAT64     CDP_ID(0x0126B2F874594000)      /* "FLOAT64"-- */

#define CDP_ACRON_VECTOR2D    CDP_ID(0x0136963D2FC92900)      /* "VECTOR2D"- */
#define CDP_ACRON_VECTOR3D    CDP_ID(0x0136963D2FC93900)      /* "VECTOR3D"- */
#define CDP_ACRON_VECTOR4D    CDP_ID(0x0136963D2FC94900)      /* "VECTOR4D"- */

#define CDP_ACRON_MATRIX2D    CDP_ID(0x012D874CA9E12900)      /* "MATRIX2D"- */
#define CDP_ACRON_MATRIX3D    CDP_ID(0x012D874CA9E13900)      /* "MATRIX3D"- */
#define CDP_ACRON_MATRIX4D    CDP_ID(0x012D874CA9E14900)      /* "MATRIX4D"- */

// Children
#define CDP_ACRON_LENGTH1D    CDP_ID(0x012C96E9F4A11900)      /* "LENGTH1D"- */
#define CDP_ACRON_LENGTH2D    CDP_ID(0x012C96E9F4A12900)      /* "LENGTH2D"- */
#define CDP_ACRON_LENGTH3D    CDP_ID(0x012C96E9F4A13900)      /* "LENGTH3D"- */

// Agents
#define CDP_WORD_LOGIC        CDP_ID(0x0031E748C0000000)      /* "logic"______ */
#define CDP_WORD_BITWISE      CDP_ID(0x000934BA66500000)      /* "bitwise"____ */
#define CDP_WORD_ARITHMETIC   CDP_ID(0x000649A21A5A2460)      /* "arithmetic"_ */
#define CDP_WORD_MATHEMATIC   CDP_ID(0x003434415A1A2460)      /* "mathematic"_ */

// Selectors
#define CDP_WORD_AND          CDP_ID(0x0005C40000000000)      /* "and"________ */
#define CDP_WORD_OR           CDP_ID(0x003E400000000000)      /* "or"_________ */
#define CDP_WORD_NOT          CDP_ID(0x0039F40000000000)      /* "not"________ */
#define CDP_WORD_XOR          CDP_ID(0x0061F20000000000)      /* "xor"________ */

#define CDP_WORD_EQUAL        CDP_ID(0x0016350B00000000)      /* "equal"______ */
#define CDP_ACRON_GT          CDP_ID(0x0127D00000000000)      /* "GT"------- */
#define CDP_ACRON_EGT         CDP_ID(0x01259F4000000000)      /* "EGT"------ */
#define CDP_ACRON_LT          CDP_ID(0x012CD00000000000)      /* "LT"------- */
#define CDP_ACRON_ELT         CDP_ID(0x0125B34000000000)      /* "ELT"------ */

#define CDP_WORD_ADD          CDP_ID(0x0004840000000000)      /* "add"________ */
#define CDP_WORD_SUBTRACT     CDP_ID(0x004EA2A4823A0000)      /* "subtract"___ */
#define CDP_WORD_MULTIPLY     CDP_ID(0x0036ACA260CC8000)      /* "multiply"___ */
#define CDP_WORD_DIVIDE       CDP_ID(0x001136490A000000)      /* "divide"_____ */


// Characters

#define CDP_BINARY_BOOLEAN                                                     \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_WORD_BOOLEAN,                                        \
        .pow2       = CDP_BIN_POW2_BYTE1                                       \
    })

#define CDP_BINARY_UINT32                                                      \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_UINT32,                                        \
        .pow2       = cdp_ctz(sizeof(uint32_t))                                \
    })

#define CDP_BINARY_UINT64                                                      \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_UINT64,                                        \
        .pow2       = cdp_ctz(sizeof(uint64_t))                                \
    })

#define CDP_BINARY_INT32                                                       \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_INT32,                                         \
        .pow2       = cdp_ctz(sizeof(int32_t)),                                \
        .sign       = 1                                                        \
    })

#define CDP_BINARY_INT64                                                       \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_INT64,                                         \
        .pow2       = cdp_ctz(sizeof(int64_t)),                                \
        .sign       = 1                                                        \
    })

#define CDP_BINARY_FLOAT32                                                     \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_FLOAT32,                                       \
        .pow2       = cdp_ctz(sizeof(float)),                                  \
        .sign       = 1,                                                       \
        .floating   = CDP_BIN_FLOAT_BINARY                                     \
    })

#define CDP_BINARY_FLOAT64                                                     \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_FLOAT64,                                       \
        .pow2       = cdp_ctz(sizeof(double)),                                 \
        .sign       = 1,                                                       \
        .floating   = CDP_BIN_FLOAT_BINARY                                     \
    })

#define CDP_BINARY_VECTOR2D                                                    \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_VECTOR2D,                                      \
        .pow2       = cdp_ctz(sizeof(float)),                                  \
        .sign       = 1,                                                       \
        .floating   = CDP_BIN_FLOAT_BINARY,                                    \
        .dimension  = CDP_BIN_DIM_VECTOR2D                                     \
    })

#define CDP_BINARY_VECTOR3D                                                    \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_VECTOR3D,                                      \
        .pow2       = cdp_ctz(sizeof(float)),                                  \
        .sign       = 1,                                                       \
        .floating   = 1,                                                       \
        .dimension  = CDP_BIN_DIM_VECTOR3D                                     \
    })

#define CDP_BINARY_VECTOR4D                                                    \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_VECTOR4D,                                      \
        .pow2       = cdp_ctz(sizeof(float)),                                  \
        .sign       = 1,                                                       \
        .floating   = CDP_BIN_FLOAT_BINARY,                                    \
        .dimension  = CDP_BIN_DIM_VECTOR4D                                     \
    })

#define CDP_BINARY_MATRIX2D                                                    \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_MATRIX2D,                                      \
        .pow2       = cdp_ctz(sizeof(float)),                                  \
        .sign       = 1,                                                       \
        .floating   = CDP_BIN_FLOAT_BINARY,                                    \
        .dimension  = CDP_BIN_DIM_MATRIX2D                                     \
    })

#define CDP_BINARY_MATRIX3D                                                    \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_MATRIX3D,                                      \
        .pow2       = cdp_ctz(sizeof(float)),                                  \
        .sign       = 1,                                                       \
        .floating   = CDP_BIN_FLOAT_BINARY,                                    \
        .dimension  = CDP_BIN_DIM_MATRIX3D                                     \
    })

#define CDP_BINARY_MATRIX4D                                                    \
    ((cdpBinary) {                                                             \
        .domain     = CDP_WORD_BINARY,                                         \
        .tag        = CDP_ACRON_MATRIX4D,                                      \
        .pow2       = cdp_ctz(sizeof(float)),                                  \
        .sign       = 1,                                                       \
        .floating   = CDP_BIN_FLOAT_BINARY,                                    \
        .dimension  = CDP_BIN_DIM_MATRIX4D                                     \
    })


#endif
