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

#ifndef CDP_PHYSICS_H
#define CDP_PHYSICS_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpPhysics,
            type:       3,          // Physic type.
            gravity:    1,          // True if gravity affects this body.
            ccd:        1,          // Continuous collision detection.
            sleeping:   1,          // Allow sleeping.
            joint:      3,          // Type of joint to parent.
            animation:  3,          // Type of body animation (if any).
            ik:         1,          // Inverse kinematics is enabled.

    
            _reserved:  37
);


enum _cdpPhysicsType {
    CDP_PHYSICS_TYPE_NONE,          // No physics.
    CDP_PHYSICS_TYPE_STATIC,        // Rigid body.
    CDP_PHYSICS_TYPE_DYNAMIC,       // Soft body.
    CDP_PHYSICS_TYPE_KINEMATIC,     // ?

    CDP_PHYSICS_TYPE_OTHER = 7
};

enum _cdpPhysicsJoint {
    CDP_PHYSICS_JOINT_NONE,         // Unconnected to parent.
    CDP_PHYSICS_JOINT_RAGDOLL,      // Connected as ragdoll.
    CDP_PHYSICS_JOINT_BALL,         // Ball joint.
    CDP_PHYSICS_JOINT_HINGE,        // ...

    CDP_PHYSICS_JOINT_OTHER = 7
};

enum _cdpPhysicsAnimation {
    CDP_PHYSICS_ANIMATION_NONE,     // Not animated (other than physics).
    CDP_PHYSICS_ANIMATION_SKELETAL, // Skeletal (key vertices) animation.
    CDP_PHYSICS_ANIMATION_MORPH,    // ...

    CDP_PHYSICS_ANIMATION_OTHER = 7
};


// Domain
#define CDP_WORD_PHYSICS            DP_IDC(0x00_____00000)      /* "physics"____ */
    
    
// Uses 

// Children
// mass
// damping (drag factor)
//friction
//elasticity

// Agencies
#define CDP_WORD_CREATOR            CDP_IDC(0x000E450D1F200000)     /* "creator"____ */
                                                                    
// Selectors                                                      
                                                                    
                                                                    
// Events                                                         


#endif
