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


CDP_METADATA_STRUCT(cdpSimulation,
    cdpAttribute    role:       3,
                    element:    4,  // Distinguishes types of structures (e.g., wall, character).
                    interaction:3,  // Type of interaction.

                    _reserved:  22;
);


enum _cdpSimulationRole {
    CDP_SIM_ROLE_ENTITY,        // World entity (anything that occupies space).
    CDP_SIM_ROLE_CONTAINER,     // Basic objects or containers that hold space and other entities.
    CDP_SIM_ROLE_ACTION,        // Actions or operations that entities can perform.
    CDP_SIM_ROLE_INTERACTIVE,   // Entities designed for world interaction, like doors or moving platforms.
    CDP_SIM_ROLE_SETUP,         // Simulation parameters or system settings that affect the behavior of the simulation.
};

enum _cdpSimulationElement {
    CDP_SIM_ELEMENT_CHARACTER,  //
    CDP_SIM_ELEMENT_VEHICLE,    //
    CDP_SIM_ELEMENT_TOOL,       //
    CDP_SIM_ELEMENT_FURNITURE,  //

    CDP_SIM_ELEMENT_WALL,       //
    CDP_SIM_ELEMENT_GROUND,     //
    CDP_SIM_ELEMENT_VEGETATION, //

    CDP_SIM_ELEMENT_PARTICLE,   // Used for effects and overlays.
};

enum _cdpSimulationInteraction {
    CDP_SIM_INTERACT_OPEN,      //
    CDP_SIM_INTERACT_CLOSE,     //
    CDP_SIM_INTERACT_ACTIVATE,  //
};

enum _cdpSimulationTag {

};


#endif
