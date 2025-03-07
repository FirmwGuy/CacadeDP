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

#ifndef CDP_BODY_H
#define CDP_BODY_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpBody,
            is3d:       1,          // True if it is 3D (has volume), 2D (area only) otherwise.
            primitive:  3,          // Primitive (brick) used for tessellating it.
            concave:    1,          // True if concave polygon, convex otherwise.
            holes:      1,          // It has holes.
            lod:        3,          // Level of detail.

            animated:   1,          // Key-frame interpolated (vertex) animation.
            skeleton:   1,          // Skeletal animation.
            morph:      1,          // Morph target animation.

            physics:    3,          // Physic type.
            ik:         1,          // Inverse kinematics enabled.
            gravity:    1,          // True if gravity affects this body.
            ccd:        1,          // Continuous collision detection.
            sleeping:   1,          // Allow sleeping.
            joint:      3,          // Type of joint to parent.

            _reserved:  28
);


enum _cdpBodyPrimitive {
    CDP_BODY_PRIM_TRIANGLE,         // Composed of Triangles.
    CDP_BODY_PRIM_QUAD,             // Composed of Quads.
    CDP_BODY_PRIM_FAN,              // Composed by a Fan.
    CDP_BODY_PRIM_STRIP,            // Composed by a Strip.
    CDP_BODY_PRIM_MESH,             // Composed by a Mesh.

    CDP_BODY_PRIM_OTHER = 7
};


enum _cdpBodyLOD {
    CDP_BODY_LOD_HIGH,              // High level of detail (all vertices).
    CDP_BODY_LOD_MEDIUM,            // Medium (50%) details.
    CDP_BODY_LOD_LOW,               // Low (25%) details.
    CDP_BODY_LOD_LOWEST,            // Lowest (10%) details possible.

    CDP_BODY_LOD_OTHER = 7
};


enum _cdpBodyPhysics {
    CDP_BODY_PHYSICS_NONE,          // No physics.
    CDP_BODY_PHYSICS_STATIC,        // Rigid body.
    CDP_BODY_PHYSICS_DYNAMIC,       // Soft body.
    CDP_BODY_PHYSICS_KINEMATIC,     // ?

    CDP_BODY_PHYSICS_OTHER = 7
};


enum _cdpBodyJoint {
    CDP_BODY_JOINT_NONE,         // Unconnected to parent.
    CDP_BODY_JOINT_RAGDOLL,      // Connected as ragdoll.
    CDP_BODY_JOINT_BALL,         // Ball joint.
    CDP_BODY_JOINT_HINGE,        // ...

    CDP_BODY_JOINT_OTHER = 7
};


// Domain
#define CDP_WORD_BODY               DP_IDC(__)      /* "body"        */


// Uses


// Children


// Agencies
#define CDP_WORD_CREATOR            CDP_IDC(0x000E450D1F200000)     /* "creator"____ */

// Selectors


// Events

#endif
