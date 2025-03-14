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

#ifndef CDP_PHYSICS_H
#define CDP_PHYSICS_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpPhysics,
            physics:    3,          // Physic type.
            ik:         1,          // Inverse kinematics enabled.
            gravity:    1,          // True if gravity affects this body.
            ccd:        1,          // Continuous collision detection.
            sleeping:   1,          // Allow sleeping.
            joint:      3,          // Type of joint to parent.

            animated:   1,          // Key-frame interpolated (vertex) animation.
            skeleton:   1,          // Skeletal animation.
            morph:      1,          // Morph target animation.

            _reserved:  37
);


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
#define CDP_WORD_VIRTUAL         DP_IDC(0x005932A542C00000)      /* "virtual"____ */

// Encodings

// Uses


// Agencies
#define CDP_WORD_ORDERER         CDP_IDC(0x003E442C8B200000)     /* "orderer"____ */

// Selectors


// Events

#endif
