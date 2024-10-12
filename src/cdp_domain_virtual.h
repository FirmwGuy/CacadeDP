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

#ifndef CDP_DOMAIN_VIRTUAL_H
#define CDP_DOMAIN_VIRTUAL_H


#include "cdp_record.h"


CDP_METADATA_STRUCT(cdpVirtual,
    cdpAttribute    visible:    2,  // How virtual object may be seen.
                    audible:    2,  // How object may be heard.
                    tactil:     2,  // How object may be felt.
                    anchor:     1,  // If its linked (anchored) to a parent object.
                    group:      1,  // True if it has child objects.
                    bounding:   3,  // Type of bounding (used for collisions).
                    collision:  3,  // Collision rule (as a child).
                    expand:     2,  // How container object may expand to accomodate children.
                    halign:     2,  // Horizontal alignment.
                    valign:     2,  // Vertical alignment.
                    interface:  2,

                    _reserved:    9;
);


enum _cdpVirtualVisible {
    CDP_VIRT_VISIB_NONE,        // Invisible.
    CDP_VIRT_VISIB_ORIENTED,    // Standard visibility.
    CDP_VIRT_VISIB_BILLBOARDED, // Always facing the screen.

    CDP_VIRT_VISIB_OTHER = 3
};

enum _cdpVirtualAudible {
    CDP_VIRT_AUDIB_NONE,        // Silent.
    CDP_VIRT_AUDIB_POSITIONED,  // Standard audio positioning.
    CDP_VIRT_AUDIB_OMNIPRESENT, // Object audio is independent of position.

    CDP_VIRT_AUDIB_OTHER = 3
};

enum _cdpVirtualTactil {
    CDP_VIRT_TACTIL_NONE,       // Object doesn't generate vibrations.
    CDP_VIRT_TACTIL_VARIABLE,   // Tactil interface depends on distance.
    CDP_VIRT_TACTIL_CONSTANT,   // Tactil interface is independent of distance.

    CDP_VIRT_TACTIL_OTHER = 3
};

enum _cdpVirtualBounding {
    CDP_VIRT_BOUND_PARTICLE,    // Object is treated as a particle.
    CDP_VIRT_BOUND_SPHERE,      // Object has a bounding sphere.
    CDP_VIRT_BOUND_BOX,         // Object has a bounding volume (3D box).
    CDP_VIRT_BOUND_POLYGON,     // Object has a bounding (3D) polygon.

    CDP_VIRT_BOUND_OTHER = 7
};

enum _cdpVirtualCollision {
    CDP_VIRT_COLLI_NONE,        // Object never collides with other objects.
    CDP_VIRT_COLLI_STACKING,    // Object pushes other objects, while parent limits are ignored.
    CDP_VIRT_COLLI_FLOWING,     // Object pushes other objects, colliding with parent limits.
    CDP_VIRT_COLLI_PHYSICS2D,   // Object collision is dictated by 2D physics in the parent's XY plane.
    CDP_VIRT_COLLI_PHYSICS,     // Object collision is dictated by a 3D physics engine.

    CDP_VIRT_COLLI_OTHER = 7
};

enum _cdpVirtualExpand {
    CDP_VIRT_EXPAND_NONE,       // Container object can't expand.
    CDP_VIRT_EXPAND_PARENT,     // Container may expand to parent's available space if needed.
    CDP_VIRT_EXPAND_ALWAYS,     // Container always expands, even without children.

    CDP_VIRT_EXPAND_OTHER = 3
};

enum _cdpVirtualHorizAlign {
    CDP_VIRT_HALG_NONE,
    CDP_VIRT_HALG_LEFT,
    CDP_VIRT_HALG_CENTER,
    CDP_VIRT_HALG_RIGHT
};

enum _cdpVirtualVertAlign {
    CDP_VIRT_VALG_NONE,
    CDP_VIRT_VALG_TOP,
    CDP_VIRT_VALG_CENTER,
    CDP_VIRT_VALG_BOTTOM
};

enum _cdpVirtualInterface {
    CDP_VIRT_STATIC,            // Object can't be interacted with.
    CDP_VIRT_STATIC,            // Object can't be interacted with.
};

//~ enum cdpVirtualState {
    //~ CDP_IF_STATE_START,
    //~ CDP_IF_STATE_DOING,
    //~ CDP_IF_STATE_COMMIT,
    //~ CDP_IF_STATE_ABORT,
//~ };

enum _cdpVirtualTagID {
    // Uses

    // Low level objects
    CDP_VIRT_TAG_OBJECT,        // Basic virtual object.
    CDP_VIRT_TAG_AUDIBLE,       // A pure audible object.
    CDP_VIRT_TAG_VISUAL,        // A pure visual object.
    CDP_VIRT_TAG_TACTILE,       // A pure tactile object.

    CDP_VIRT_TAG_BUTTON,        // A touchable object.
    CDP_VIRT_TAG_TOOGLE,        // A button that can be turned on or off.
    CDP_VIRT_TAG_LIST,          // List of (possible multiple selectable) objects.
    CDP_VIRT_TAG_SLIDE,         // A range selector.

    CDP_VIRT_TAG_CURSOR,        // Text input prompt.
    CDP_VIRT_TAG_POINTER,       // Pointer guide (eg, mouse cursor).
    CDP_VIRT_TAG_SELECTOR,      // A pointer range selector (eg, mouse drag box).

    CDP_VIRT_TAG_TITLE,         // A (possible) tabbed title and container for frames.
    CDP_VIRT_TAG_FRAME,         // Frame to put interface objects on.
    CDP_VIRT_TAG_BAR,           // A frame status meta-space.

    CDP_VIRT_TAG_LABEL,
    CDP_VIRT_TAG_DROPDOWN,
    CDP_VIRT_TAG_COMBOBOX,
    CDP_VIRT_TAG_CHECKBOX,

    CDP_VIRT_TAG_TEXT_INPUT,
    CDP_VIRT_TAG_PASSWORD,
    CDP_VIRT_TAG_CONSOLE,       // Log of output messages.

    // High level objects
    CDP_VIRT_TAG_MAP,           // A contextual interface map selector (e.g., menus, mind map, etc).
    CDP_VIRT_TAG_CONFIG,        // An interface configuration and customization component.
    CDP_VIRT_TAG_SYSTEM,        // A system options dialog.
    CDP_VIRT_TAG_FILE,          // Resource (eg, file, folder) selector dialog.
    CDP_VIRT_TAG_FONT,          // Font selector dialog.
    CDP_VIRT_TAG_COLOR,         // Color picker dialog.
    CDP_VIRT_TAG_CALENDAR,      // Date selector dialog.

    // Children
    CDP_VIRT_TAG_POSITION,      // Position vector with respect to anchor.
    CDP_VIRT_TAG_ANCHOR,        // Anchor position with respect to parent.
    CDP_VIRT_TAG_LAYER,         // World layer it belongs to.

    CDP_VIRT_TAG_TRANSFORM,     // Transform from object space to parent space (including translation, scaling/dilatation, etc).
    CDP_VIRT_TAG_INVTRANSF,     // Inverse transform (from parent space to object space).

    CDP_VIRT_TAG_BOUNDING,      // Bounding sphere, volume, etc.
    CDP_VIRT_TAG_SHAPE,         // Shape (contour) of element.

    CDP_VIRT_TAG_COLLISION,     // Rule to resolve collisions and positioning.

    CDP_VIRT_TAG_MOUSE_POS,     // Mouse screen position in pixels (as a 2D vector).
    CDP_VIRT_TAG_SCREEN_SIZE,   // Current screen size (may be a combined virtual desktop size).

    CDP_VIRT_TAG_CLICK_BEEP,    // Click beep.
    CDP_VIRT_TAG_SELECT_BEEP,   // Selection confirmation beep.
    CDP_VIRT_TAG_NOTICE_BEEP,   // On notice event sound.

    CDP_VIRT_TAG_ERROR_BUZZ,    // Tactile error buzz.
    CDP_VIRT_TAG_CONFIRMATION,  // Tactile confirmation vibration.
    CDP_VIRT_TAG_NAV_PULSE,     // Tactile navigation pulse.

    // Agencies
    CDP_VIRT_TAG_SET_TRANSFORM,
    CDP_VIRT_TAG_MOVE_TO,
    CDP_VIRT_TAG_ARC_TO,

    // Events
    // Input Event
    CDP_VIRT_TAG_FOCUS,         // Focus gain/lost event.
    CDP_VIRT_TAG_POINT,         // Pointed-at (with mouse or glove) event.
    CDP_VIRT_TAG_LOOK,          // Look-at (with viewport or camera) event.

    CDP_VIRT_TAG_MOUSE_CLICK,
    CDP_VIRT_TAG_MOUSE_MOVE,
    CDP_VIRT_TAG_MOUSE_OVER,

    CDP_VIRT_TAG_CLICK,         // Button/key input event.
    CDP_VIRT_TAG_TACTILE,       // Tactile (touchpad) input event.
    CDP_VIRT_TAG_VOICE,         // Voice input event.
    CDP_VIRT_TAG_GESTURE,       // Gesture input event.

    CDP_VIRT_TAG_UNDO,          // Revert user input.
    CDP_VIRT_TAG_REDO,          // Repeat user input.
    CDP_VIRT_TAG_RESET,         // Reset user input.

    CDP_VIRT_TAG_CUT,           // Cut away (and copy) user input.
    CDP_VIRT_TAG_COPY,          // Copy user input.
    CDP_VIRT_TAG_PASTE,         // Paste user input.

    CDP_VIRT_TAG_SELECT,        // Select all selectable objects.
    CDP_VIRT_TAG_DESELECT,      // Deselect all selected objects.

    // Output Event
    CDP_VIRT_TAG_UPDATE,        // Interface object content was updated by program.

    CDP_VIRT_TAG_BUSY,          // Interface object is busy (can't be interacted with).
    CDP_VIRT_TAG_DISABLE,       // Object disabled (no further user interaction allowed).
    CDP_VIRT_TAG_DELETE,        // Object deleted.
    CDP_VIRT_TAG_HIDE,          // Hide object
    CDP_VIRT_TAG_SHOW,          // Show (restore) object.

    CDP_VIRT_TAG_U_EXPIRE,      // User response took too long.
    CDP_VIRT_TAG_TIMEOUT,       // Program response took too long.


};


#endif
