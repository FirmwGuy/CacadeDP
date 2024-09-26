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

#ifndef CDP_DOM_TEXT_H
#define CDP_DOM_TEXT_H


#include "cdp_record.h"


CDP_ATTRIBUTE_STRUCT(cdpTextAttribute,
    cdpAttribute    type:       3,      // _cdpTextType.
                    formating:  3,      // _cdpTextFormat.
);


enum _cdpTextRole {
    CDP_ROLE_CONTENT,       // Represents text content such as paragraphs, headers, or plain text.
    CDP_ROLE_FORMATING,     // Elements used to apply formatting (e.g., bold, italic, underline).
    CDP_ROLE_BLOCK,         // Structural elements that enclose other elements, such as div, section, blockquote, etc.
    CDP_ROLE_TABLE,         // Represents lists, tables, and other collection structures (e.g., <ul>, <ol>, <table>).
    CDP_ROLE_MEDIA,         // Represents media elements like images, videos, and embedded content (e.g., <img>, <iframe>).
    //CDP_ROLE_METADATA,      // Used for defining metadata, comments, or annotations within the text (e.g., <meta>, comments in markdown).
};


// Initial tag IDs (for a description see cdp_agent.h):
enum _cdpBinaryTagID {
    CDP_TAG_BYTE,
    CDP_TAG_UINT16,
    CDP_TAG_UINT32,
    CDP_TAG_UINT64,
    CDP_TAG_UINT128,
    CDP_TAG_INT16,
    CDP_TAG_INT32,
    CDP_TAG_INT64,
    CDP_TAG_INT128,
    CDP_TAG_DECIMAL32,
    CDP_TAG_DECIMAL64,
    CDP_TAG_DECIMAL128,
    CDP_TAG_FLOAT32,
    CDP_TAG_FLOAT64,
    CDP_TAG_FLOAT128,
    CDP_TAG_COMPLEX32,
    CDP_TAG_COMPLEX64,
    CDP_TAG_COMPLEX128,
    //
    CDP_TAG_TAG,
    CDP_TAG_ID,
    CDP_TAG_PATCH,

    CDP_TAG_BINARY_COUNT
};

enum _cdpTextType {
    CDP_PLAIN,
    CDP_HEADER,
    CDP_TITLE,
    CDP_PARAGRAPH,
    CDP_NOTE,
};

enum _cdpTextFormating {
    CDP_BOLD,
    CDP_ITALIC,
    CDP_UNDERLINE,
    CDP_STRIKEOUT,
};


#endif
