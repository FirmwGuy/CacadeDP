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

#ifndef CDP_DOMAIN_DEVICE_H
#define CDP_DOMAIN_DEVICE_H


#include "cdp_record.h"


CDP_CHARACTER_STRUCT( cdpDevice,
    os:             3,          // Operating system.
    audio:          3,          // Audio system.
    audiout:        3,          // Audio output.
    window:         3,          // Window system.
    graphics:       3,          // Graphics API.
    mouse:          1,          // Mouse is present.
    keyboard:       1,          // Keyboard present.
    joystick:       1,          // Joystick present.
    touchpad:       1,          // Touchpad/touchscreen available.
    status:         3,          // Device status.

    _reserved:      42          // ToDo: .
);


enum _cdpDeviceOS {
    CDP_DEVICE_OS_LINUX,
    CDP_DEVICE_OS_WINDOWS,
    CDP_DEVICE_OS_ANDROID,
    CDP_DEVICE_OS_IOS,
    CDP_DEVICE_OS_BROWSER,
    CDP_DEVICE_OS_FREERTOS,

    CDP_DEVICE_OS_OTHER = 7
};

enum _cdpDeviceAudio {
    CDP_DEVICE_AUDIO_NONE,
    CDP_DEVICE_AUDIO_NATIVE,
    CDP_DEVICE_AUDIO_OPENAL,
    CDP_DEVICE_AUDIO_WEBAUDIO,

    CDP_DEVICE_AUDIO_OTHER = 7
};

enum _cdpDeviceAudioOutput {
    CDP_DEVICE_AUDIOUT_NONE,
    CDP_DEVICE_AUDIOUT_SPEAKERS,
    CDP_DEVICE_AUDIOUT_HEADPHONES,
    CDP_DEVICE_AUDIOUT_HDMI,

    CDP_DEVICE_AUDIOUT_OTHER = 7
};

enum _cdpDeviceWindow {
    CDP_DEVICE_WINDOW_NONE,
    CDP_DEVICE_WINDOW_X11,
    CDP_DEVICE_WINDOW_WIN32,
    CDP_DEVICE_WINDOW_COCOA,
    CDP_DEVICE_WINDOW_BROWSER,
    CDP_DEVICE_WINDOW_WAYLAND,

    CDP_DEVICE_WINDOW_OTHER = 7
};

enum _cdpDeviceGraphics {
    CDP_DEVICE_GRAPHICS_NONE,
    CDP_DEVICE_GRAPHICS_OPENGL,
    CDP_DEVICE_GRAPHICS_VULKAN,
    CDP_DEVICE_GRAPHICS_DIRECTX,
    CDP_DEVICE_GRAPHICS_METAL,
    CDP_DEVICE_GRAPHICS_WEBGL,

    CDP_DEVICE_GRAPHICS_OTHER = 7
};

enum _cdpDeviceStatus {
    CDP_DEVICE_STATUS_UNKNOWN,
    CDP_DEVICE_STATUS_AVAILABLE,
    CDP_DEVICE_STATUS_UNAVAILABLE,
    CDP_DEVICE_STATUS_UNSUPPORTED,
    CDP_DEVICE_STATUS_ACTIVE,
    CDP_DEVICE_STATUS_INACTIVE,
    CDP_DEVICE_STATUS_ERROR,

    CDP_DEVICE_STATUS_OTHER = 7
};


// Domain
#define CDP_WORD_DEVICE       CDP_ID(0x0010B648CA000000)      /* "device"_____ */

// Uses


// Children


// Agents


// Selectors


#endif
