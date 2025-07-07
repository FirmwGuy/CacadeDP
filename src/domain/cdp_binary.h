/*
 *  Copyright (c) 2024-2025 Victor M. Barrientos
 *  (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
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

#ifndef CDP_BINARY_H
#define CDP_BINARY_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpBinary,
            type:       3,      // Binary data type.
            pow2:       4,      // Power of 2 exponent describing the element scalar size (in bytes).
            dimension:  3,      // Dimensions of data.

            endianess:  1,      // Little endian (0) is the norm.
            compression:3,      // Type of compression used to pack content.
            encryption: 3,      // Encryption method.

            _reserved:  33      // ToDo: expand to streams and communication related stuff.
);


enum _cdpBinaryType {
    CDP_BIN_TYPE_UNSIGNED,     // Unsigned integer.
    CDP_BIN_TYPE_INTEGER,      // Signed integer.
    CDP_BIN_TYPE_DECIMAL,      // Decimal floating point.
    CDP_BIN_TYPE_FLOAT,        // Binary floating point.
    CDP_BIN_TYPE_COMPLEX,      // Binary with imaginary part.

    CDP_BIN_TYPE_OTHER = 7
};


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


/*
    Domain:
        'binary'

    Encodings:
        'unsigned'
        'signed': 2's complement.
        'IEEE754: all floats.
        'C-COMPLEX'
        'GMP'
        'MPFR'
        'MPC'
        'MPDECIMAL'
        'C-ARRAY': contiguous memory without meta.
        'C-MATRIX: Row-Major Order: C/OpenGL/NumPy/TensorFlow style matrix).
        'MATRIXCMO': Column-Major Order: Fortran/Matlab/BLAS/LAPACK style matrix).

    Uses:
        'CDPID'
        'agent'
        'boolean'
        'byte'

        'UINT16'
        'UINT32'
        'UINT64'

        'INT16'
        'INT32'
        'INT64'

        'FLOAT32'
        'FLOAT64'

        'VECTOR2D': array of 2 floats.
        'VECTOR3D'
        'VECTOR4D'

        'MATRIX2D': matrix of 2x2 floats.
        'MATRIX3D'
        'MATRIX4D'

    Properties:
        'length': number of elements, *not* bytes.
        'LENGTH2D'
        'LENGTH3D'
        'LENGTH4D'
        'tensor-ord'
        'tensor-len'

    Agencies:
        'math'

        Modes:
            'automatic'
            'trigger'

        Selectors:
            'and'
            'or'
            'not'

            'bit-and'
            'bit-or'
            'bit-not'

            'equal'
            'greater'
            'lesser'

            'add'
            'subtract'
            'multiply'
            'divide'

        Config:
            'cast'


    Event Messages:
        'pending'
        'working'
        'completed'
        'failed'
        '
        'debug'
        'warning'
        'error'
        'fatal'
*/


// Data creation

#define CDP_BINARY(...)   ((cdpBinary){__VA_ARGS__}._id)


static inline cdpData* cdp_data_new_binary_id(cdpID value) {
    return cdp_data_new_value(
        CDP_DTWA("binary", "CDPID"),
        CDP_WORD("unsigned"),
        CDP_BINARY(
            .pow2 = cdp_ctz(sizeof(value))
        ),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_id(record, name, value)         cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_id(value), NULL)


static inline cdpData* cdp_data_new_binary_dt(cdpDT* dt) {
    assert(cdp_dt_valid(dt));
    
    cdpDT value = {.domain = dt->domain, .tag = dt->tag};
    return cdp_data_new_value(
        CDP_DTWA("binary", "CDPDT"),
        CDP_WORD("unsigned"),
        CDP_BINARY(
            .pow2      = cdp_ctz(sizeof(cdpID)),
            .dimension = CDP_BIN_DIM_VECTOR2D
        ),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_dt(record, name, dt)            cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_dt(dt), NULL)


static inline cdpData* cdp_data_new_binary_agent(cdpAgent value) {
    return cdp_data_new_value(
        CDP_DTWW("binary", "agent"),
        CDP_WORD("unsigned"),
        CDP_BINARY(
            .pow2 = cdp_ctz(sizeof(value))
        ),
        value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_agent(record, name, value)      cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_agent(value), NULL)


static inline cdpData* cdp_data_new_binary_boolean(uint8_t value) {
    return cdp_data_new_value(
        CDP_DTWW("binary", "boolean"),
        CDP_WORD("unsigned"),
        CDP_BINARY(0),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_boolean(record, name, value)    cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_boolean(value), NULL)

static inline cdpData* cdp_data_new_binary_uint32(uint32_t value) {
    return cdp_data_new_value(
        CDP_DTWA("binary", "UINT32"),
        CDP_WORD("unsigned"),
        CDP_BINARY(
            .pow2 = cdp_ctz(sizeof(value))
        ),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_uint32(record, name, value)     cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_uint32(value), NULL)

static inline cdpData* cdp_data_new_binary_uint64(uint64_t value) {
    return cdp_data_new_value(
        CDP_DTWA("binary", "UINT64"),
        CDP_WORD("unsigned"),
        CDP_BINARY(
            .pow2 = cdp_ctz(sizeof(value))
        ),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_uint64(record, name, value)     cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_uint64(value), NULL)

static inline cdpData* cdp_data_new_binary_int64(int64_t value) {
    return cdp_data_new_value(
        CDP_DTWA("binary", "INT64"),
        CDP_WORD("signed"),
        CDP_BINARY(
            .pow2 = cdp_ctz(sizeof(value)),
            .type = CDP_BIN_TYPE_INTEGER
        ),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_int64(record, name, value)      cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_int64(value), NULL)

static inline cdpData* cdp_data_new_binary_float32(float value) {
    return cdp_data_new_value(
        CDP_DTWA("binary", "FLOAT32"),
        CDP_ACRO("IEEE754"),
        CDP_BINARY(
            .pow2 = cdp_ctz(sizeof(value)),
            .type = CDP_BIN_TYPE_FLOAT
        ),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_float32(record, name, value)    cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_float32(value), NULL)

static inline cdpData* cdp_data_new_binary_float64(double value) {
    return cdp_data_new_value(
        CDP_DTWA("binary", "FLOAT64"),
        CDP_ACRO("IEEE754"),
        CDP_BINARY(
            .pow2 = cdp_ctz(sizeof(value)),
            .type = CDP_BIN_TYPE_FLOAT
        ),
        &value,
        sizeof(value)
    );
}
#define cdp_dict_add_binary_float64(record, name, value)    cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_float64(value), NULL)

static inline cdpData* cdp_data_new_binary_vector3d(float* value) {
    return cdp_data_new_value(
        CDP_DTWA("binary", "VECTOR3D"),
        CDP_ACRO("C_ARRAY"),
        CDP_BINARY(
            .pow2     = cdp_ctz(sizeof(value)),
            .type     = CDP_BIN_TYPE_FLOAT,
            .dimension= CDP_BIN_DIM_VECTOR3D
        ),
        (void*)value,
        3 * sizeof(*value)
    );
}
#define cdp_dict_add_binary_vector3d(record, name, value)   cdp_record_add_child(record, CDP_TYPE_NORMAL, name, 0, cdp_data_new_binary_vector3d(value), NULL)


#endif
