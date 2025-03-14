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


// Domain
#define CDP_WORD_BINARY             CDP_IDC(0x00092E0CB2000000)     /* "binary"_____ */

// Encodings
#define CDP_WORD_UNSIGNED           CDP_IDC(0x0055D349DC520000)     /* "unsigned"___ */
#define CDP_WORD_SIGNED             CDP_IDC(0x004D277148000000)     /* "signed"_____ */
#define CDP_ACRON_IEEE754           CDP_IDC(0x0129965957554000)     /* "IEEE754"-- (all floats)*/
#define CDP_ACRON_C_COMPLEX         CDP_IDC(0x0123363BEDC2C978)     /* "C-COMPLEX" */
#define CDP_ACRON_GMP               CDP_IDC(0x0127B70000000000)     /* "GMP"------ */
#define CDP_ACRON_MPFR              CDP_IDC(0x012DC26C80000000)     /* "MPFR"----- */
#define CDP_ACRON_MPC               CDP_IDC(0x012DC23000000000)     /* "MPC"------ */
#define CDP_ACRON_MPDECIMAL         CDP_IDC(0x012DC24963A6D86C)     /* "MPDECIMAL" */
#define CDP_ACRON_C_ARRAY           CDP_IDC(0x0123361CB2879000)     /* "C-ARRAY"-- (contiguous memory without meta) */
#define CDP_ACRON_C_MATRIX          CDP_IDC(0x012336D874CA9E00)     /* "C-MATRIX"- (Row-Major Order: C/OpenGL/NumPy/TensorFlow style matrix) */
#define CDP_ACRON_MATRIXCMO         CDP_IDC(0x012D874CA9E23B6F)     /* "MATRIXCMO" (Column-Major Order: Fortran/Matlab/BLAS/LAPACK style matrix) */

 // Uses
#define CDP_ACRON_CDPID             CDP_IDC(0x0123930A64000000)     /* "CDPID"     */
#define CDP_WORD_BOOLEAN            CDP_IDC(0x0009EF6142E00000)     /* "boolean"____ */
#define CDP_WORD_BYTE               CDP_IDC(0x000B342800000000)     /* "byte"_______ */

#define CDP_ACRON_UINT16            CDP_IDC(0x0135A6ED11580000)     /* "UINT16"    */
#define CDP_ACRON_UINT32            CDP_IDC(0x0135A6ED13480000)     /* "UINT32"    */
#define CDP_ACRON_UINT64            CDP_IDC(0x0135A6ED16500000)     /* "UINT64"    */

#define CDP_ACRON_INT16             CDP_IDC(0x0129BB4456000000)     /* "INT16"     */
#define CDP_ACRON_INT32             CDP_IDC(0x0129BB44D2000000)     /* "INT32"     */
#define CDP_ACRON_INT64             CDP_IDC(0x0129BB4594000000)     /* "INT64"     */

#define CDP_ACRON_FLOAT32           CDP_IDC(0x0126B2F8744D2000)     /* "FLOAT32"   */
#define CDP_ACRON_FLOAT64           CDP_IDC(0x0126B2F874594000)     /* "FLOAT64"   */

#define CDP_ACRON_VECTOR2D          CDP_IDC(0x0136963D2FC92900)     /* "VECTOR2D"  (array of 2 floats) */
#define CDP_ACRON_VECTOR3D          CDP_IDC(0x0136963D2FC93900)     /* "VECTOR3D"  */
#define CDP_ACRON_VECTOR4D          CDP_IDC(0x0136963D2FC94900)     /* "VECTOR4D"  */

#define CDP_ACRON_MATRIX2D          CDP_IDC(0x012D874CA9E12900)     /* "MATRIX2D"  (matrix of 2x2 floats) */
#define CDP_ACRON_MATRIX3D          CDP_IDC(0x012D874CA9E13900)     /* "MATRIX3D"  */
#define CDP_ACRON_MATRIX4D          CDP_IDC(0x012D874CA9E14900)     /* "MATRIX4D"  */

// Children
#define CDP_WORD_LENGTH             CDP_IDC(0x0000000000000000)     /* "length"      */
#define CDP_ACRON_LENGTH2D          CDP_IDC(0x0000000000000000)     /* "LENGTH2D"- */
#define CDP_ACRON_LENGTH3D          CDP_IDC(0x0000000000000000)     /* "LENGTH3D"- */
#define CDP_ACRON_LENGTH4D          CDP_IDC(0x0000000000000000)     /* "LENGTH4D"- */
#define CDP_WORD_TENSOR_ORD         CDP_IDC(0x0000000000000000)     /* "tensor-ord"  */
#define CDP_WORD_TENSOR_LEN         CDP_IDC(0x0000000000000000)     /* "tensor-len"  */

// Agencies
#define CDP_WORD_BUFFER             CDP_IDC(0x000AA63164000000)     /* "buffer"      */
#define CDP_WORD_CLONER             CDP_IDC(0x000D8F7164000000)     /* "cloner"      */
#define CDP_WORD_CONVERTER          CDP_IDC(0x000DEEB16542C800)     /* "converter"   */
#define CDP_WORD_MATH               CDP_IDC(0x0034344000000000)     /* "math"        */

    // Modes
    #define CDP_WORD_AUTOMATIC      CDP_IDC(0x0006B47B43448C00)     /* "automatic"   */
    #define CDP_WORD_TRIGGER        CDP_IDC(0x00524939CB200000)     /* "trigger"     */

    // Selectors
    #define CDP_WORD_AND            CDP_IDC(0x0000000000000000)     /* "and"         */
    #define CDP_WORD_OR             CDP_IDC(0x0000000000000000)     /* "or"          */
    #define CDP_WORD_NOT            CDP_IDC(0x0000000000000000)     /* "not"         */

    #define CDP_WORD_BIT_AND        CDP_IDC(0x0000000000000000)     /* "bit-and"     */
    #define CDP_WORD_BIT_OR         CDP_IDC(0x0000000000000000)     /* "bit-or"      */
    #define CDP_WORD_BIT_NOT        CDP_IDC(0x0000000000000000)     /* "bit-not"     */

    #define CDP_WORD_EQUAL          CDP_IDC(0x0000000000000000)     /* "equal"       */
    #define CDP_WORD_GREATER        CDP_IDC(0x0000000000000000)     /* "greater"     */
    #define CDP_WORD_LESSER         CDP_IDC(0x0000000000000000)     /* "lesser"      */

    #define CDP_WORD_ADD            CDP_IDC(0x0000000000000000)     /* "add"         */
    #define CDP_WORD_SUBTRACT       CDP_IDC(0x0000000000000000)     /* "subtract"    */
    #define CDP_WORD_MULTIPLY       CDP_IDC(0x0000000000000000)     /* "multiply"    */
    #define CDP_WORD_DIVIDE         CDP_IDC(0x0000000000000000)     /* "divide"      */

    // Config
    #define CDP_WORD_CAST           CDP_IDC(0x000C33A000000000)     /* "cast"        */


// Event Messages
#define CDP_WORD_PENDING            CDP_IDC(0x0000000000000000)     /* "pending"     */
#define CDP_WORD_WORKING            CDP_IDC(0x0000000000000000)     /* "working"     */
#define CDP_WORD_COMPLETED          CDP_IDC(0x0000000000000000)     /* "completed"   */
#define CDP_WORD_FAILED             CDP_IDC(0x0000000000000000)     /* "failed"      */

#define CDP_WORD_DEBUG              CDP_IDC(0x0000000000000000)     /* "debug"       */
#define CDP_WORD_WARNING            CDP_IDC(0x0000000000000000)     /* "warning"     */
#define CDP_WORD_ERROR              CDP_IDC(0x0000000000000000)     /* "error"       */
#define CDP_WORD_FATAL              CDP_IDC(0x0000000000000000)     /* "fatal"       */


// Data creation
/*
static inline cdpData* cdp_data_new_bin_id(cdpID value) {
    return cdp_data_new_value(
        CDP_WORD_BINARY,
        CDP_ACRON_CDPID,
        (cdpBinary){
            .pow2 = cdp_ctz(sizeof(value))
        },
        sizeof(value),
        value
    );
}

static inline cdpData* cdp_data_new_bin_boolean(uint8_t value) {
    return cdp_data_new_value(
        CDP_WORD_BINARY,
        CDP_WORD_BOOLEAN,
        (cdpBinary){0},
        sizeof(value),
        value
    );
}

static inline cdpData* cdp_data_new_bin_uint32(uint32_t value) {
    return cdp_data_new_value(
        CDP_WORD_BINARY,
        CDP_ACRON_UINT32,
        (cdpBinary){
            .pow2 = cdp_ctz(sizeof(value))
        },
        sizeof(value),
        value
    );
}

static inline cdpData* cdp_data_new_bin_int64(int64_t value) {
    return cdp_data_new_value(
        CDP_WORD_BINARY,
        CDP_ACRON_INT64,
        (cdpBinary){
            .pow2 = cdp_ctz(sizeof(value)),
            .sign = 1
        },
        sizeof(value),
        value
    );
}

static inline cdpData* cdp_data_new_bin_float32(float value) {
    return cdp_data_new_value(
        CDP_WORD_BINARY,
        CDP_ACRON_FLOAT32,
        (cdpBinary){
            .pow2     = cdp_ctz(sizeof(value)),
            .sign     = 1,
            .floating = CDP_BIN_FLOAT_BINARY
        },
        sizeof(value),
        value
    );
}

static inline cdpData* cdp_data_new_bin_float64(double value) {
    return cdp_data_new_value(
        CDP_WORD_BINARY,
        CDP_ACRON_FLOAT64,
        (cdpBinary){
            .pow2     = cdp_ctz(sizeof(value)),
            .sign     = 1,
            .floating = CDP_BIN_FLOAT_BINARY
        },
        sizeof(value),
        value
    );
}

static inline cdpData* cdp_data_new_bin_vector3d(float* value) {
    return cdp_data_new_value(
        CDP_WORD_BINARY,
        CDP_ACRON_FLOAT64,
        (cdpBinary){
            .pow2     = cdp_ctz(sizeof(value)),
            .sign     = 1,
            .floating = CDP_BIN_FLOAT_BINARY,
            .dimension= CDP_BIN_DIM_VECTOR3D
        },
        3 * sizeof(*value),
        value
    );
}
*/

#endif
