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
                    sign:       1,  // Is it signed (1) or unsigned (0)?
                    exponent:   1,  // True (1) if not an integer.
                    dimension:  3,  // Dimensions of data.

                    endianess:  1,  // Little endian (0) is the norm.
                    abi:        3,  // Binary representation of content.
                    compression:3,  // Type of compression used to pack content.
                    encryption: 3,  // Encryption method.

                    _reserved:  13; // ToDo: expand to streams and communication related stuff.
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
    CDP_BIN_POW2_BYTE1,         // Scalar size is 8 bits.
    CDP_BIN_POW2_BYTE2,         // Scalar is 16 bits.
    CDP_BIN_POW2_BYTE4,         // 32 bits.
    CDP_BIN_POW2_BYTE8,         // 64.
    CDP_BIN_POW2_BYTE16,        // 128.

    CDP_BIN_POW2_OTHER = 15
};


enum _cdpBinaryABI {            // Aplication Binary Interface:
    CDP_BIN_ENC_BYTE,           // Content is an opaque sequence of bytes.
    CDP_BIN_ENC_UNSIGNED,       // All GCC-supported unsigned sizes.
    CDP_BIN_ENC_INTEGER,        // All GCC-supported signed integer sizes.
    //CDP_BIN_ENC_GMP,            // LGPL GMP (bignum) library.
    CDP_BIN_ENC_FLOAT,          // IEEE binary representation of floats.
    //CDP_BIN_ENC_MPFR,           // LGPL MPFR (bigfloat) library.
    CDP_BIN_ENC_DECIMAL,        // GCC decimal representation.
    //CDP_BIN_ENC_MPDECIMAL,      // BSD MPDecimal library.
    CDP_BIN_ENC_COMPLEX,        // GCC representation of complex floats.
    //CDP_BIN_ENC_MPC,            // LGPL MPC (bigcomplex) library.
    CDP_BIN_ENC_MATRIX,         // CDP representation of vector/matrices.
    //CDP_BIN_ENC_ARRAYFIRE,      // BSD Arrayfire tensor library.

    CDP_BIN_ENC_OTHER = 7
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


enum _cdpBinaryTag {
    // Uses
    CDP_BIN_TAG_LINK,           // Link to other record.
    CDP_BIN_TAG_AGENT,          // Address of agent function.
    CDP_BIN_TAG_TAG,
    CDP_BIN_TAG_NAME,
    CDP_BIN_TAG_PATCH,

    CDP_BIN_TAG_BYTE,           // Opaque binary representation.
    CDP_BIN_TAG_VALUE,
    CDP_BIN_TAG_INDEX,
    CDP_BIN_TAG_BOOLEAN,
    CDP_BIN_TAG_BITWISE,        // Bitwise/bitmask.
    CDP_BIN_TAG_ADDRESS,        // Local memory/size/offset.

    CDP_BIN_TAG_CRC,
    CDP_BIN_TAG_HASH,

    // Children
    CDP_BIN_TAG_LENGTH,         // Arbitrary length of a vector.
    CDP_BIN_TAG_LENGTH2D,       // Arbitrary length of a matrix.
    CDP_BIN_TAG_LENGTH3D,
    CDP_BIN_TAG_LENGTH4D,

    CDP_BIN_TAG_TENSOR_ORD,     // Tensor order (if over 4 dimensions).
    CDP_BIN_TAG_TENSOR_LEN,     // A vector with arbitrary dimension lengths for a tensor.

    // Agencies
    CDP_BIN_TAG_AND,
    CDP_BIN_TAG_OR,
    CDP_BIN_TAG_NOT,

    CDP_BIN_TAG_EQUAL,
    CDP_BIN_TAG_GREATER,
    CDP_BIN_TAG_LESSER,

    CDP_BIN_TAG_ADD,
    CDP_BIN_TAG_SUBSTRACT,
    CDP_BIN_TAG_MULTIPLY,
    CDP_BIN_TAG_DIVIDE,

    //
    CDP_BIN_TAG_INI_COUNT
};


#define cdp_binary_metadata_link()                                             \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_LINK,                                          \
        .pow2     = cdp_ctz(sizeof(void*)), /* Same as log2(sizeof(void*)). */ \
        .encoding = CDP_BIN_ENC_UNSIGNED                                       \
    })

#define cdp_binary_metadata_agent()                                            \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_AGENT,                                         \
        .pow2     = cdp_ctz(sizeof(cdpAgent)),                                 \
        .encoding = CDP_BIN_ENC_UNSIGNED                                       \
    })

#define cdp_binary_metadata_boolean()                                          \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_BOOLEAN,                                       \
        .pow2     = CDP_BIN_POW2_BYTE1,                                        \
        .encoding = CDP_BIN_ENC_BYTE                                           \
    })

#define cdp_binary_metadata_uint32()                                           \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_VALUE,                                         \
        .pow2     = cdp_ctz(sizeof(uint32_t)),                                 \
        .encoding = CDP_BIN_ENC_UNSIGNED                                       \
    })

#define cdp_binary_metadata_uint64()                                           \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_VALUE,                                         \
        .pow2     = cdp_ctz(sizeof(uint64_t)),                                 \
        .encoding = CDP_BIN_ENC_UNSIGNED                                       \
    })

#define cdp_binary_metadata_int32()                                            \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_VALUE,                                         \
        .pow2     = cdp_ctz(sizeof(int32_t)),                                  \
        .sign     = 1,                                                         \
        .encoding = CDP_BIN_ENC_INTEGER                                        \
    })

#define cdp_binary_metadata_int64()                                            \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_VALUE,                                         \
        .pow2     = cdp_ctz(sizeof(int64_t)),                                  \
        .sign     = 1,                                                         \
        .encoding = CDP_BIN_ENC_INTEGER                                        \
    })

#define cdp_binary_metadata_float32()                                          \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_VALUE,                                         \
        .pow2     = cdp_ctz(sizeof(float)),                                    \
        .sign     = 1,                                                         \
        .exponent = 1,                                                         \
        .encoding = CDP_BIN_ENC_FLOAT                                          \
    })

#define cdp_binary_metadata_float64()                                          \
    ((cdpBinary) {                                                             \
        .domain   = CDP_DOMAIN_BINARY,                                         \
        .tag      = CDP_BIN_TAG_VALUE,                                         \
        .pow2     = cdp_ctz(sizeof(double)),                                   \
        .sign     = 1,                                                         \
        .exponent = 1,                                                         \
        .encoding = CDP_BIN_ENC_FLOAT                                          \
    })


void cdp_binary_system_initiate(void);


static inline size_t cdp_binary_size(cdpRecord* record) {
    assert(!cdp_record_is_void(record));

    static const void* const _recData[] = {&&NONE, &&NEAR, &&DATA, &&FAR};
    goto *_recData[(record)->metadata.recdata];
    {
      NONE: {
        assert(cdp_record_has_data(record));  // This shouldn't happen.
        return 0;
      }
      NEAR: {
        cdpBinary* binary = (cdpBinary*) &record->metadata;
        assert(binary->dimension <= CDP_BIN_DIM_VECTOR4D);
        size_t entries = 1 + binary->dimension;
        size_t bsize = (1 << binary->pow2);
        return (entries * bsize);
      }
      DATA:;
      FAR: {
        return record->data->size;
      }
    }

    return 0;
}


#endif
