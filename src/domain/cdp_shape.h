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
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#ifndef CDP_SHAPE_H
#define CDP_SHAPE_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpShape,
            volume:     1,          // True if it has volume (3D), 2D otherwise.
            primitive:  3,          // Type of primitive tessellation.
            concave:    1,          // True if concave polygon, convex otherwise.
            holes:      1,          // It has holes.          
            lod:        3,          // Level of detail index entry for this shape.
              
            _reserved:  22
);


enum _cdpShapePrimitive {
    CDP_SHAPE_PRIM_TRIANGLE,        // Composed of Triangles.
    CDP_SHAPE_PRIM_QUAD,            // Composed of Quads.
    CDP_SHAPE_PRIM_FAN,             // Composed by a Fan.
    CDP_SHAPE_PRIM_STRIP,           // Composed by a Strip.
    CDP_SHAPE_PRIM_MESH ,           // Composed by a Mesh.

    CDP_SHAPE_PRIM_OTHER = 7
};


enum _cdpShapeLOD {
    CDP_SHAPE_LOD_HIGHEST,     // Maximum level of detail (all vertices).
    CDP_SHAPE_LOD_MEDIUM,      // Medium (50%) details.
    CDP_SHAPE_LOD_LOW,         // Low (25%) details.
    CDP_SHAPE_LOD_LOWEST,      // Lowest (10%) details possible.

    CDP_SHAPE_LOD_OTHER = 7
};



// Domain
#define CDP_WORD_VIRTUAL            DP_IDC(0x005932A542C00000)      /* "virtual"____ */
    
    
// Uses 
    

// Children


// Agencies
#define CDP_WORD_CREATOR            CDP_IDC(0x000E450D1F200000)     /* "creator"____ */
                                                                    
// Selectors                                                      
                                                                    
                                                                    
// Events                                                         

#endif
