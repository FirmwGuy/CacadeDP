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


/*
    Domain:
        'virtual'

    Encodings:
        'GLVBO': OpenGL XYZ per-vertex array.
        'GLVBOTEX': Same as GLVBO, but with added NXNYNZ normals and UV texture coord.
        'GLVBOFULL': Same as GLVBOTEX, but with added RGBA color values.

    Uses

    Properties:
        'center'
        'radius'
        //#define CDP_ACRON_BBOX
        //#define CDP_ACRON_BCUBE


    Agencies:
        'drafter'

    Events
*/


#endif
