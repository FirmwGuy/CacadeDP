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
    cdpAttribute    encoding:   2,      // Text encoding (UTF8, Unicode, ISO90.., etc).
);


enum _cdpnterfaceIRole {
    CDP_ROLE_IF_INPUT,          // Components that capture user input (e.g., buttons, text fields, CLI commands, voice inputs).
    CDP_ROLE_IF_INPUT,          // Components that capture user input (e.g., buttons, text fields, CLI commands, voice inputs).
};

enum _cdpTextTagID {
    CDP_TAG_TXT_BYTE,

    CDP_TAG_TEXT_COUNT
};

enum _cdpTextEncoding {
    CDP_TE_UTF8,
    CDP_TE_UNICODE,
    CDP_TE_ISO90,
};

enum _cdpTextType {
    CDP_TT_HEADER,
    CDP_TT_TITLE,
    CDP_TT_PARAGRAPH,
    CDP_TT_NOTE,
};

enum _cdpTextFormating {
    CDP_TF_BOLD,
    CDP_TF_ITALIC,
    CDP_TF_UNDERLINE,
    CDP_TF_STRIKEOUT,
};

enum _cdpTextAlignment {
    CDP_TA_LEFT,
    CDP_TA_CENTER,
    CDP_TA_RIGHT,
    CDP_TA_JUSTIFIED,
};

enum _cdpTextListing {
    CDP_TL_UNSORTED,
    CDP_TL_ENUMERATED,
    CDP_TL_DEFINITION,
};

enum _cdpTextMedia {
    CDP_TM_IMAGE,
    CDP_TM_AUDIO,
    CDP_TM_VIDEO
};

#endif
