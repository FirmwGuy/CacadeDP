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

#ifndef CDP_INTERFACE_H
#define CDP_INTERFACE_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpInterface,
            group:      1,          // True if it has child objects.
            anchor:     1,          // If its linked (anchored) to a specific parent point (parent's origin otherwise).
            xalign:     2,          // Children horizontal (X) alignment.
            yalign:     2,          // Children vertical (Y) alignment.
            zalign:     2,          // Children dept (Z) alignment.

            expand:     2,          // How (container) object may expand to accommodate children.
            collision:  3,          // Collision treatment (as a child).

            visible:    2,          // How virtual object may be seen.
            audible:    2,          // How object may be heard.
            tactil:     2,          // How object may be felt.

            iflook:     1,          // Object responds to be looked at (eg, in field of view).
            ifpointed:  1,          // Object responds to be pointed at (eg, mouse pointer).
            ifclick:    1,          // Object responds to device/virtual clicks.
            ifread:     1,          // Object responds to written text.
            ifhear:     1,          // Object responds to sounds/voice.
            ifgesture:  1,          // Object responds to gestures.

            _reserved:  25
);


enum _cdpVirtualVisible {
    CDP_VIRT_VISIBLE_NONE,          // Invisible.
    CDP_VIRT_VISIBLE_ORIENTED,      // Standard visibility.
    CDP_VIRT_VISIBLE_BILLBOARDED,   // Always facing the screen.

    CDP_VIRT_VISIBLE_OTHER = 3
};

enum _cdpVirtualAudible {
    CDP_VIRT_AUDIBLE_NONE,          // Silent.
    CDP_VIRT_AUDIBLE_POSITIONED,    // Standard audio positioning.
    CDP_VIRT_AUDIBLE_OMNIPRESENT,   // Object audio is independent of position.

    CDP_VIRT_AUDIBLE_OTHER = 3
};

enum _cdpVirtualTactil {
    CDP_VIRT_TACTIL_NONE,           // Object doesn't generate vibrations.
    CDP_VIRT_TACTIL_VARIABLE,       // Tactil interface depends on distance.
    CDP_VIRT_TACTIL_CONSTANT,       // Tactil interface is independent of distance.

    CDP_VIRT_TACTIL_OTHER = 3
};

enum _cdpVirtualAlignment {
    CDP_VIRT_ALIGN_NONE,
    CDP_VIRT_ALIGN_FAR,
    CDP_VIRT_ALIGN_CENTER,
    CDP_VIRT_ALIGN_NEAR
};

enum _cdpVirtualExpand {
    CDP_VIRT_EXPAND_NONE,           // Container object can't expand.
    CDP_VIRT_EXPAND_PARENT,         // Container may expand to parent's available space if needed.
    CDP_VIRT_EXPAND_ALWAYS,         // Container always expands, even without children.

    CDP_VIRT_EXPAND_OTHER = 3
};

enum _cdpVirtualCollision {
    CDP_VIRT_COLLITION_NONE,        // Object never collides with other objects.
    CDP_VIRT_COLLITION_FLOWING,     // Object pushes other objects colliding with parent limits.
    CDP_VIRT_COLLITION_STACKING,    // Object make stacks (piles) when colliding with other objects.
    CDP_VIRT_COLLITION_GROUPING,    // Object make groups (special markers) when colliding with other objects.
    CDP_VIRT_COLLITION_PHYSICS2D,   // Object collision is dictated by 2D physics.
    CDP_VIRT_COLLITION_PHYSICS3D,   // Object collision is dictated by a 3D physics engine.

    CDP_VIRT_COLLITION_OTHER = 7
};



// Domain
#define CDP_WORD_INTERFACE          DP_IDC(0x0025D42C8C119400)      /* "interface"__ */


// Uses

// Low level objects
#define CDP_WORD_OBJECT             DP_IDC(0x003C4A28E8000000)      /* "object"_____ */
#define CDP_WORD_AUDIBLE            DP_IDC(0x0006A44898500000)      /* "audible"____ (A pure audible object) */
#define CDP_WORD_VISUAL             DP_IDC(0x005933A858000000)      /* "visual"_____ (A pure visual object) */

    //CDP_VIRT_TAG_TACTILE,          // A pure tactile object.

#define CDP_WORD_BUTTON             DP_IDC(0x000AB4A3DC000000)      /* "button"_____ (A touchable object) */
#define CDP_WORD_TOGGLE             DP_IDC(0x0051E73B0A000000)      /* "toggle"_____ (A button that can be turned on or off) */
#define CDP_WORD_CARTE              DP_IDC(0x000C32A140000000)      /* "carte"______ (List of (possible multiple selectable) objects) */
#define CDP_WORD_SLIDE              DP_IDC(0x004D892140000000)      /* "slide"______ (A range selector) */
#define CDP_WORD_POINTER            DP_IDC(0x0041E9750B200000)      /* "pointer"____ (Pointer guide (eg, mouse cursor)) */
#define CDP_WORD_SELECTION          DP_IDC(0x004CAC28E897B800)      /* "selection"__ (A pointer range selection (eg, mouse drag box)) */

    //CDP_VIRT_TAG_TITLE,         // A (possible) tabbed title and container for frames.
    //CDP_VIRT_TAG_FRAME,         // Decorated frame to put interface objects in.
    //CDP_VIRT_TAG_BAR,           // A frame status meta-space.
    //CDP_VIRT_TAG_SYSTEM,        // An interface customization component (eg, resizer, etc).

    //CDP_VIRT_TAG_LABEL,         // Static text.
    //CDP_VIRT_TAG_CURSOR,        // Text input (character) prompt.
    //CDP_VIRT_TAG_TEXT_INPUT,    // Text input line.
    //CDP_VIRT_TAG_PASSWORD,      // Hidden content text input.
    //CDP_VIRT_TAG_CONSOLE,       // Log of output messages.

    //CDP_VIRT_TAG_DROPDOWN,
    //CDP_VIRT_TAG_COMBOBOX,
    //CDP_VIRT_TAG_CHECKBOX,

// High level objects
    //CDP_VIRT_TAG_MAP,           // A contextual interface map selector (e.g., menus, radials, etc).
    //CDP_VIRT_TAG_CONFIG,        // A system configuration options dialog.

    //CDP_VIRT_TAG_FILE,          // Resource (eg, file/folder) selector dialog.
    //CDP_VIRT_TAG_FONT,          // Font selector dialog.
    //CDP_VIRT_TAG_COLOR,         // Color picker dialog.
    //CDP_VIRT_TAG_CALENDAR,      // Date selector dialog.

// Children
#define CDP_WORD_LAYER              CDP_IDC(0x0030392C80000000)     /* "layer"______ */
#define CDP_WORD_POSITION           CDP_IDC(0x0041F34D12F70000)     /* "position"___ */
#define CDP_WORD_CENTER             CDP_IDC(0x000CAEA164000000)     /* "center"_____ */
#define CDP_ACRON_OBJ2WORLD         CDP_IDC(0x012F8AA4B7BF2B24)     /* "OBJ2WORLD" */
#define CDP_ACRON_WORLD2OBJ         CDP_IDC(0x0137BF2B244AF8AA)     /* "WORLD2OBJ" */

    //CDP_VIRT_TAG_BOUNDING,      // Bounding sphere, volume, etc.
    //CDP_VIRT_TAG_SHAPE,         // Shape (contour) of element.

    //CDP_VIRT_TAG_CLICK_BEEP,    // Click beep.
    //CDP_VIRT_TAG_SELECT_BEEP,   // Selection confirmation beep.
    //CDP_VIRT_TAG_NOTICE_BEEP,   // On notice event sound.

    //CDP_VIRT_TAG_ERROR_BUZZ,    // Tactile error buzz.
    //CDP_VIRT_TAG_CONFIRMATION,  // Tactile confirmation vibration.
    //CDP_VIRT_TAG_NAV_PULSE,     // Tactile navigation pulse.

    //CDP_VIRT_TAG_VIEW_SIZE,     // Current world viewport size.
    //CDP_VIRT_TAG_MOUSE_POS,     // Mouse pointer position.

// Agencies
#define CDP_WORD_STAGER             CDP_IDC(0x004E813964000000)     /* "stager"_____ */

// Selectors


// Events
#define CDP_WORD_SPAWN              CDP_IDC(0x004E01BB80000000)     /* "spawn"______ */
#define CDP_WORD_MOVE               CDP_IDC(0x0035F62800000000)     /* "move"_______ */
#define CDP_WORD_REMOVE             CDP_IDC(0x0035F62800000000)     /* "remove"_____ */

// Input Events
    //CDP_VIRT_TAG_FOCUS,         // Focus gain/lost event.
    //CDP_VIRT_TAG_POINT,         // Pointed-at (with mouse or glove) event.
    //CDP_VIRT_TAG_LOOK,          // Look-at (with viewport or camera) event.

    //CDP_VIRT_TAG_MOUSE_CLICK,
    //CDP_VIRT_TAG_MOUSE_MOVE,
    //CDP_VIRT_TAG_MOUSE_OVER,

    //CDP_VIRT_TAG_CLICK,         // Button/key input event.
    //CDP_VIRT_TAG_TACTILE,       // Tactile (touchpad) input event.
    //CDP_VIRT_TAG_VOICE,         // Voice input event.
    //CDP_VIRT_TAG_GESTURE,       // Gesture input event.

    //CDP_VIRT_TAG_UNDO,          // Revert user input.
    //CDP_VIRT_TAG_REDO,          // Repeat user input.
    //CDP_VIRT_TAG_RESET,         // Reset user input.

    //CDP_VIRT_TAG_CUT,           // Cut away (and copy) user input.
    //CDP_VIRT_TAG_COPY,          // Copy user input.
    //CDP_VIRT_TAG_PASTE,         // Paste user input.

    //CDP_VIRT_TAG_SELECT,        // Select all selectable objects.
    //CDP_VIRT_TAG_DESELECT,      // Deselect all selected objects.

// Output Events
    //CDP_VIRT_TAG_UPDATE,        // Interface object content was updated by program.

    //CDP_VIRT_TAG_BUSY,          // Interface object is busy (can't be interacted with).
    //CDP_VIRT_TAG_DISABLE,       // Object disabled (no further user interaction allowed).
    //CDP_VIRT_TAG_DELETE,        // Object deleted.
    //CDP_VIRT_TAG_HIDE,          // Hide object
    //CDP_VIRT_TAG_SHOW,          // Show (restore) object.

    //CDP_VIRT_TAG_U_EXPIRE,      // User response took too long.
    //CDP_VIRT_TAG_TIMEOUT,       // Program response took too long.

    //~ CDP_IF_STATE_START,
    //~ CDP_IF_STATE_DOING,
    //~ CDP_IF_STATE_COMMIT,
    //~ CDP_IF_STATE_ABORT,


#endif
