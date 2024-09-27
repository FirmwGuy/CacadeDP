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

#ifndef CDP_DOMAIN_TEXT_H
#define CDP_DOMAIN_TEXT_H


#include "cdp_record.h"


CDP_ATTRIBUTE_STRUCT(cdpTextAttribute,
    cdpAttribute    encoding:   3,      // Text encoding (UTF8, Unicode, ISO90.., etc).
                    type:       3,      // Text type (title, paragraph, etc).
                    formating:  3,      // Text format (bold, italic, etc).
                    alignment:  2,      // Text alignment (left, center, etc).
                    heading:    3,      // Heading level value (H1, H2, etc).
                    listing:    2,      // List/table type (enumerated, definition, etc).
                    language:   6,      // Language of content (including programming language for scripts).
                    media:      2,      // Embedded media type (image, video, etc).
);


enum _cdpTextRole {
    CDP_ROLE_TXT_CONTENT,       // Represents text content such as paragraphs, headers, or plain text.
    CDP_ROLE_TXT_FORMATING,     // Elements used to apply formatting (e.g., bold, italic, underline).
    CDP_ROLE_TXT_BLOCK,         // Structural elements that enclose other elements, such as div, section, blockquote, etc.
    CDP_ROLE_TXT_TABLE,         // Represents lists, tables, and other collection structures (e.g., <ul>, <ol>, <table>).
    CDP_ROLE_TXT_MEDIA,         // Represents media elements like images, videos, and embedded content (e.g., <img>, <iframe>).
    CDP_ROLE_TXT_LINK,          // A link (URI) to other document.
    CDP_ROLE_TXT_METADATA,      // Used for defining metadata, comments, or annotations within the text (e.g., <meta>, comments in markdown).
    CDP_ROLE_TXT_SCRIPT         // The (executable) code part of this document.
};

enum _cdpTextTagID {
    CDP_TAG_TXT_TITLE,
    CDP_TAG_TXT_AUTHOR,
    CDP_TAG_TXT_DATE,
    CDP_TAG_TXT_VERSION,
    CDP_TAG_TXT_COPYRIGHT,
    CDP_TAG_TXT_LICENSE,
    CDP_TAG_TXT_ABSTRACT,
    CDP_TAG_TXT_BODY,
    CDP_TAG_TXT_TOC,
    CDP_TAG_TXT_CHAPTER,
    CDP_TAG_TXT_SECTION,
    CDP_TAG_TXT_PARAGRAPH,
    CDP_TAG_TXT_FOOTNOTE,
    CDP_TAG_TXT_CONCLUSION,
    CDP_TAG_TXT_AKNOWLEDGMENT,
    CDP_TAG_TXT_APPENDICE,
    CDP_TAG_TXT_GLOSSARY,

    CDP_TAG_TEXT_COUNT
};

enum _cdpTextEncoding {
    CDP_TE_UTF8,            // The standard.
    CDP_TE_UNICODE,         // Wide string.
    CDP_TE_ISO8859,         // European (aka Latin1).
    CDP_TE_SHIFT_JIS,       // Japanse.
    CDP_TE_BIG5,            // Cantonese chinese.
    CDP_TE_GB18030,         // Simplified chinese.
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

enum _cdpTextLanguage {
    CDP_LANG_ENGLISH,
    CDP_LANG_SPANISH,
    CDP_LANG_FRENCH,
    CDP_LANG_GERMAN,
    CDP_LANG_PORTUGESE,
    CDP_LANG_ITALIAN,

    CDP_LANG_INDI = 16,
    CDP_LANG_MANDARIN,
    CDP_LANG_CANTONESE,
    CDP_LANG_COREAN,
    CDP_LANG_JAPANESE,

    CDP_LANG_PYTHON = 32,
    CDP_LANG_JAVASCRIPT,
};

enum _cdpTextMedia {
    CDP_TM_IMAGE,
    CDP_TM_AUDIO,
    CDP_TM_VIDEO
};


#endif
