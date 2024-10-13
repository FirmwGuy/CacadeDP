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

                    group:      1,  // True if it has child objects.
                    anchor:     1,  // If its linked (anchored) to a parent object. Anchored to the world otherwise.
                    xalign:     2,  // Children horizontal (X) alignment.
                    yalign:     2,  // Children vertical (Y) alignment.
                    zalign:     2,  // Children dept (Z) alignment.

                    bounding:   3,  // Type of bounding (used for collisions).
                    expand:     2,  // How (container) object may expand to accomodate children.
                    collision:  3,  // Collision rule (as a child).

                    iflook:     1,  // Object responds to be looked at (eg, in field of view).
                    ifpointed:  1,  // Object responds to be pointed at (eg, mouse pointer).
                    ifclick:    1,  // Object responds to device/virtual clicks.
                    ifread:     1,  // Object responds to written text.
                    ifhear:     1,  // Object responds to sounds/voice.
                    ifgesture:  1,  // Object responds to gestures.

                    _reserved:  4;
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

enum _cdpVirtualAlignment {
    CDP_VIRT_ALG_NONE,
    CDP_VIRT_ALG_FAR,
    CDP_VIRT_ALG_CENTER,
    CDP_VIRT_ALG_NEAR
};

enum _cdpVirtualBounding {
    CDP_VIRT_BOUND_PARTICLE,    // Object is treated as a particle.
    CDP_VIRT_BOUND_SPHERE,      // Object has a bounding sphere.
    CDP_VIRT_BOUND_BOX,         // Object has a bounding volume (3D box).
    CDP_VIRT_BOUND_POLYGON,     // Object has a bounding (3D) polygon.

    CDP_VIRT_BOUND_OTHER = 7
};

enum _cdpVirtualExpand {
    CDP_VIRT_EXPAND_NONE,       // Container object can't expand.
    CDP_VIRT_EXPAND_PARENT,     // Container may expand to parent's available space if needed.
    CDP_VIRT_EXPAND_ALWAYS,     // Container always expands, even without children.

    CDP_VIRT_EXPAND_OTHER = 3
};

enum _cdpVirtualCollision {
    CDP_VIRT_COLLI_NONE,        // Object never collides with other objects.
    CDP_VIRT_COLLI_STACKING,    // Object pushes other objects ignoring parent limits.
    CDP_VIRT_COLLI_FLOWING,     // Object pushes other objects colliding with parent limits.
    CDP_VIRT_COLLI_PHYSICS,     // Object collision is dictated by (2D) physics.
    CDP_VIRT_COLLI_PHYSICS3D,   // Object collision is dictated by a 3D physics engine.

    CDP_VIRT_COLLI_OTHER = 7
};


//~ enum cdpVirtualState {
    //~ CDP_IF_STATE_START,
    //~ CDP_IF_STATE_DOING,
    //~ CDP_IF_STATE_COMMIT,
    //~ CDP_IF_STATE_ABORT,
//~ };

enum _cdpVirtualTag {
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

    CDP_VIRT_TAG_POINTER,       // Pointer guide (eg, mouse cursor).
    CDP_VIRT_TAG_SELECTOR,      // A pointer range selector (eg, mouse drag box).

    CDP_VIRT_TAG_TITLE,         // A (possible) tabbed title and container for frames.
    CDP_VIRT_TAG_FRAME,         // Decorated frame to put interface objects in.
    CDP_VIRT_TAG_BAR,           // A frame status meta-space.
    CDP_VIRT_TAG_SYSTEM,        // An interface customization component (eg, resizer, etc).

    CDP_VIRT_TAG_LABEL,         // Static text.
    CDP_VIRT_TAG_CURSOR,        // Text input (character) prompt.
    CDP_VIRT_TAG_TEXT_INPUT,    // Text input line.
    CDP_VIRT_TAG_PASSWORD,      // Hidden content text input.
    CDP_VIRT_TAG_CONSOLE,       // Log of output messages.

    CDP_VIRT_TAG_DROPDOWN,
    CDP_VIRT_TAG_COMBOBOX,
    CDP_VIRT_TAG_CHECKBOX,

    // High level objects
    CDP_VIRT_TAG_MAP,           // A contextual interface map selector (e.g., menus, radials, etc).
    CDP_VIRT_TAG_CONFIG,        // A system configuration options dialog.

    CDP_VIRT_TAG_FILE,          // Resource (eg, file/folder) selector dialog.
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

    CDP_VIRT_TAG_CLICK_BEEP,    // Click beep.
    CDP_VIRT_TAG_SELECT_BEEP,   // Selection confirmation beep.
    CDP_VIRT_TAG_NOTICE_BEEP,   // On notice event sound.

    CDP_VIRT_TAG_ERROR_BUZZ,    // Tactile error buzz.
    CDP_VIRT_TAG_CONFIRMATION,  // Tactile confirmation vibration.
    CDP_VIRT_TAG_NAV_PULSE,     // Tactile navigation pulse.

    CDP_VIRT_TAG_VIEW_SIZE,     // Current world viewport size.
    CDP_VIRT_TAG_MOUSE_POS,     // Mouse pointer position.

    // Agencies
    CDP_VIRT_TAG_SET_TRANSFORM, // Updates object position, rotation, scale, etc.

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

    //
    CDP_VIRT_TAG_INI_COUNT
};


#endif
