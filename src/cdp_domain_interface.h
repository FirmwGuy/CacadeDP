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

#ifndef CDP_DOMAIN_INTERFACE_H
#define CDP_DOMAIN_INTERFACE_H


#include "cdp_record.h"


CDP_ATTRIBUTE_STRUCT(cdpInterfaceAttribute,
    uint16_t      object:   5,  // Interface object element.
                  event:    5,  // Generated interface events.
                  //--------

                  device:   4;  // Hardware used for I/O.
                  state:    2,  // State of event (doing, committing, etc).

    uint16_t      id;           // Numerical identifier (handle) of interface object.
);


enum _cdpInterfaceRole {
    CDP_ROLE_IF_OBJECT,         // Basic interactive interface element (e.g., buttons, sliders, etc).
    CDP_ROLE_IF_CONTROL,        // A control element (e.g., text command, mouse-clicks, mouse-pointing, gesture-based controls).
    CDP_ROLE_IF_TEXT,           // A pure (rich) text component.
    CDP_ROLE_IF_AUDIBLE,        // A pure audible component.
    CDP_ROLE_IF_VISUAL,         // A pure visual component.
    CDP_ROLE_IF_TACTILE,        // A pure tactile component.
    CDP_ROLE_IF_METAOBJ         // An interface configuration and customization component.
};

enum cdpInterfaceObject {
    // Low level components
    CDP_IF_OBJ_BUTTON,          // A touchable object.
    CDP_IF_OBJ_TOOGLE,          // A button that can be turned on or off.
    CDP_IF_OBJ_LIST,            // List of (possible multiple selectable) objects.
    CDP_IF_OBJ_SLIDE,           // A range selector.

    CDP_IF_OBJ_CURSOR,          // Text input prompt.
    CDP_IF_OBJ_POINTER,         // Pointer guide (eg, mouse cursor).
    CDP_IF_OBJ_SELECTOR,        // A pointer range selector (eg, mouse drag box).

    CDP_IF_OBJ_TITLE,           // A (possible) tabbed title and container for frames.
    CDP_IF_OBJ_FRAME,           // Frame to put interface objects on.
    CDP_IF_OBJ_BAR,             // A frame status meta-space.

    // High level components
    CDP_IF_OBJ_MAP = 16,        // A contextual interface map selector (e.g., menus, mind map, etc).
    CDP_IF_OBJ_SYSTEM,          // A system options dialog.
    CDP_IF_OBJ_FILE,            // Resource (eg, file, folder) selector dialog.
    CDP_IF_OBJ_FONT,            // Font selector dialog.
    CDP_IF_OBJ_COLOR,           // Color picker dialog.
    CDP_IF_OBJ_CALENDAR,        // Date selector dialog.
};

enum cdpInterfaceEvent {
    // Input Event
    CDP_IF_EVENT_FOCUS,         // Focus gain/lost event.
    CDP_IF_EVENT_POINT,         // Pointed-at (with mouse or glove) event.
    CDP_IF_EVENT_LOOK,          // Look-at (with viewport or camera) event.

    CDP_IF_EVENT_CLICK,         // Button/key input event.
    CDP_IF_EVENT_TACTILE,       // Tactile (touchpad) input event.
    CDP_IF_EVENT_VOICE,         // Voice input event.
    CDP_IF_EVENT_GESTURE,       // Gesture input event.

    CDP_IF_EVENT_UNDO,          // Revert user input.
    CDP_IF_EVENT_REDO,          // Repeat user input.
    CDP_IF_EVENT_RESET,         // Reset user input.

    CDP_IF_EVENT_CUT,           // Cut away (and copy) user input.
    CDP_IF_EVENT_COPY,          // Copy user input.
    CDP_IF_EVENT_PASTE,         // Paste user input.

    CDP_IF_EVENT_SELECT,        // Select all selectable objects.
    CDP_IF_EVENT_DESELECT,      // Deselect all selected objects.

    // Output Event
    CDP_IF_EVENT_UPDATE = 16,   // Interface object content was updated by program.

    CDP_IF_EVENT_BUSY,          // Interface object is busy (can't be interacted with).
    CDP_IF_EVENT_DISABLE,       // Object disabled (no further user interaction allowed).
    CDP_IF_EVENT_DELETE,        // Object deleted.
    CDP_IF_EVENT_HIDE,          // Hide object
    CDP_IF_EVENT_SHOW,          // Show (restore) object.

    CDP_IF_EVENT_U_EXPIRE,      // User response took too long.
    CDP_IF_EVENT_TIMEOUT,       // Program response took too long.
};

enum cdpInterfaceDevice {
    // Input device
    CDP_IF_DEVICE_KEYBOARD,     // Keyboard (may be virtual).
    CDP_IF_DEVICE_MOUSE,        // Mouse.
    CDP_IF_DEVICE_TOUCHPAD,     // Touchscreen/touchpad.
    CDP_IF_DEVICE_JOYSTICK,     // Joystick.
    CDP_IF_DEVICE_MICROPHONE,   // User voice command.
    CDP_IF_DEVICE_CAMERA,       // User video input.
    CDP_IF_DEVICE_VR,           // User VR input.

    // Output device
    CDP_IF_DEVICE_SCREEN = 8,   // Screen/display.
    CDP_IF_DEVICE_SPEAKER,      // Speaker/headset.
    CDP_IF_DEVICE_VIBRATOR,     // Vibration.
    CDP_IF_DEVICE_PRINTER,      // User printer.
};

enum cdpInterfaceState {
    CDP_IF_STATE_START,
    CDP_IF_STATE_DOING,
    CDP_IF_STATE_COMMIT,
    CDP_IF_STATE_ABORT,
};


enum _cdpInterfaceTagID {
    // Children
    CDP_TAG_IF_MOUSE_POS,       // Mouse screen position in pixels (as a 2D vector).
    CDP_TAG_IF_SCREEN_SIZE,     // Current screen size (may be a combined virtual desktop size).
    //

    CDP_TAG_IF_CONSOLE,         // Log of output messages.
    CDP_TAG_IF_TEXT_INPUT,
    CDP_TAG_IF_PASSWORD,

    CDP_TAG_IF_LABEL,
    CDP_TAG_IF_DROPDOWN,
    CDP_TAG_IF_COMBOBOX,
    CDP_TAG_IF_CHECKBOX,

    CDP_TAG_IF_MOUSE_CLICK,
    CDP_TAG_IF_MOUSE_MOVE,
    CDP_TAG_IF_MOUSE_OVER,

    CDP_TAG_IF_CLICK_BEEP,      // Click beep.
    CDP_TAG_IF_SELECT_BEEP,     // Selection confirmation beep.
    CDP_TAG_IF_NOTICE_BEEP,     // On notice event sound.

    CDP_TAG_IF_ERROR_BUZZ,      // Tactile error buzz.
    CDP_TAG_IF_CONFIRMATION,    // Tactile confirmation vibration.
    CDP_TAG_IF_NAV_PULSE,       // Tactile navigation pulse.

    CDP_TAG_TEXT_COUNT
};


#endif
