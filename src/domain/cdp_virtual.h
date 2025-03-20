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

#ifndef CDP_VIRTUAL_H
#define CDP_VIRTUAL_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpVirtual,
            bounding:   3,          // Type of bounding (using for indexing).
            primitive:  3,          // Primitive (brick) used for tessellating the contour.
            is3d:       1,          // True if it is 3D (has volume), 2D (area only) otherwise.
            concave:    1,          // True if concave polygon, convex otherwise.
            holes:      1,          // It has holes.

            _reserved:  41
);


enum _cdpVirtualBounding {
    CDP_VIRT_BOUND_BOX,             // Object has a bounding volume (3D box).
    CDP_VIRT_BOUND_SPHERE,          // Object has a bounding sphere.
    CDP_VIRT_BOUND_POLYGON,         // Object has a bounding (3D) polygon.
    CDP_VIRT_BOUND_PARTICLE,        // Object is treated as a particle (single point).

    CDP_VIRT_BOUND_OTHER = 7
};


enum _cdpBodyPrimitive {
    CDP_VIRTUAL_PRIM_TRIANGLE,      // Composed of Triangles.
    CDP_VIRTUAL_PRIM_QUAD,          // Composed of Quads.
    CDP_VIRTUAL_PRIM_FAN,           // Composed by a Fan.
    CDP_VIRTUAL_PRIM_STRIP,         // Composed by a Strip.
    CDP_VIRTUAL_PRIM_MESH,          // Composed by a Mesh.

    CDP_VIRTUAL_PRIM_OTHER = 7
};


// Domain
#define CDP_WORD_VIRTUAL            DP_IDC(0x005932A542C00000)      /* "virtual"____ */

// Encodings
#define CDP_ACRON_GLVBO             CDP_IDC(0x0127B368AF000000)     /* "GLVBO"---- (OpenGL XYZ per-vertex array) */
#define CDP_ACRON_GLVBOTEX          CDP_IDC(0x0127B368AFD25E00)     /* "GLVBOTEX"- (Same as GLVBO, but with added NXNYNZ normals and UV texture coord) */
#define CDP_ACRON_GLVBOFULL         CDP_IDC(0x0127B368AF9B5B2C)     /* "GLVBOFULL" (Same as GLVBOTEX, but with added RGBA color values) */

// Uses

// Properties
#define CDP_WORD_CENTER             CDP_IDC(0x000CAEA164000000)     /* "center"_____ */
#define CDP_WORD_RADIUS             CDP_IDC(0x0048244D66000000)     /* "radius"_____ */
//#define CDP_ACRON_BBOX          CDP_IDC(0x01228AFE00000000)     /* "BBOX"----- */
//#define CDP_ACRON_BCUBE         CDP_IDC(0x01228F58A5000000)     /* "BCUBE"---- */


// Agencies
#define CDP_WORD_DRAFTER            CDP_IDC(0x001241350B200000)     /* "drafter"____ */


// Selectors


// Events

#endif
