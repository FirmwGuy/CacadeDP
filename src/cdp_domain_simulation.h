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

#ifndef CDP_DOMAIN_SIMULATION_H
#define CDP_DOMAIN_SIMULATION_H


#include "cdp_record.h"


CDP_ATTRIBUTE_STRUCT(cdpSimulationAttribute,
    uint16_t      terrain:   4,  // Distinguishes types of physical structures (e.g., wall, floor, ceiling).
                  building:
                  interaction:   4,  // Defines interaction types (e.g., open, close, activate).
                  //--------

                  event:    4,  // Generated interface events.
                  state:    2,  // State of event (doing, committing, etc).
                  device:   4;  // Hardware used for I/O.
    uint16_t      id;           // Numerical identifier (handle) of interface object.
);

enum _cdpSimulationRole {
    CDP_ROLE_SIM_CONTAINER,     // Basic objects or containers that hold other entities.
    CDP_ROLE_SIM_ANCHOR,        // Non-rendering visual aspects, such as the layout of a scene or positional markers.
    CDP_ROLE_SIM_ACTION,        // Actions or operations that entities can perform.
    CDP_ROLE_SIM_INTERACTIVE,   // Entities designed for world interaction, like doors or moving platforms.
    CDP_ROLE_SIM_TERRAIN,       //
    CDP_ROLE_SIM_BUILDING,       //
    CDP_ROLE_SIM_DECORATIVE,       //
    CDP_ROLE_SIM_CHARACTER,     // Any (human or AI-driven) agent/character that interacts within the simulation.
    CDP_ROLE_SIM_TRIGGER,       // Elements that initiate actions or events based on conditions or interactions.
    CDP_ROLE_SIM_SETUP,         // Simulation parameters or system settings that affect the behavior of the simulation.
};

enum _cdpSimulationElement {
    CDP_SIM_PHYSIC_WALL,        // Distinguishes types of physical structures (e.g., wall, floor, ceiling).
    CDP_SIM_PHYSIC_FLOOR,        //
    CDP_SIM_PHYSIC_CEILING,        //
};

enum _cdpSimulationInteraction {
    CDP_SIM_INTERACT_OPEN,        //
    CDP_SIM_INTERACT_CLOSE,        //
    CDP_SIM_INTERACT_ACTIVATE,        //
};

#endif
