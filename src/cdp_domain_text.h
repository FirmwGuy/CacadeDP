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
    cdpAttribute    encoding:   3,      // Text encoding (UTF8, Unicode, Latin1, etc).
                    heading:    3,      // Heading level value for titles (H1, H2, etc).
                    listing:    2,      // Type of listing for table/list (enumerated, definition, etc).
                    formating:  3,      // Text format (bold, italic, etc).
                    font:       3,      // Recommended font family to use for rendering.
                    alignment:  2,      // Recommended text alignment (left, center, etc).
                    language:   6,      // Language of content (including programming language for scripts).
                    media:      2,      // Embedded media type (image, video, etc).

                    _reserved:  8;      // ToDO: expand to include DOM things.
);


enum _cdpTextRole {
    CDP_ROLE_TXT_CONTENT,       // Represents text content such as paragraphs, headers, or plain text.
    CDP_ROLE_TXT_FORMATING,     // Elements used to apply formatting (bold, etc) or decoration (including space/tab padding).
    CDP_ROLE_TXT_BLOCK,         // Structural elements that enclose other elements, such as div, section, blockquote, etc.
    CDP_ROLE_TXT_TABLE,         // Represents lists, tables, and other collection structures (e.g., <ul>, <ol>, <table>).
    CDP_ROLE_TXT_MEDIA,         // Represents media elements like images, videos, and embedded content (e.g., <img>, <iframe>).
    CDP_ROLE_TXT_LINK,          // A link (URI) to other document.
    CDP_ROLE_TXT_METADATA,      // Used for defining metadata, comments, or annotations within the text (e.g., <meta>, comments in markdown).
    CDP_ROLE_TXT_SCRIPT         // The (executable) code part of this document.
};

enum _cdpTextEncoding {
    CDP_ENCOD_UTF8,             // The standard.
    CDP_ENCOD_UNICODE,          // A 2-byte wide string.
    CDP_ENCOD_ISO8859,          // European (aka Latin1).
    CDP_ENCOD_SHIFT_JIS,        // Japanse.
    CDP_ENCOD_BIG5,             // Cantonese chinese.
    CDP_ENCOD_GB18030,          // Simplified chinese.
};

enum _cdpTextListing {
    CDP_LIST_UNSORTED,
    CDP_LIST_ENUMERATION,
    CDP_LIST_DEFINITION,
};

enum _cdpTextFormating {
    CDP_TF_BOLD,
    CDP_TF_ITALIC,
    CDP_TF_UNDERLINE,
    CDP_TF_STRIKEOUT,
};

enum _cdpTextFont {
    CDP_FONT_DEFAULT,           // Use default font.
    CDP_FONT_SERIF,             // Decorative font (Georgia, Times New Roman).
    CDP_FONT_SANS,              // Modern look font (Arial, Helvetica, Verdana).
    CDP_FONT_MONO,              // Monospaced font (Console, Curier New).
    CDP_FONT_HAND,              // Handwriting font (Brush Script, Lucida Handwriting).
    CDP_FONT_TITLE,             // Display font for titles (Impact, Comic Sans).
};

enum _cdpTextAlignment {
    CDP_ALIGN_LEFT,             // The default alignment in western countries.
    CDP_ALIGN_CENTER,
    CDP_ALIGN_RIGHT,
    CDP_ALIGN_JUSTIFIED,
};

enum _cdpTextLanguage {
    CDP_LANG_ENGLISH,           // The lingua franca.
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
    CDP_TM_VIDEO,
};


enum _cdpTextTagID {
    // Children
    CDP_TAG_TXT_LENGTH,     // Non-ASCII text length in characters (NOT in bytes).
    //

    CDP_TAG_TXT_CHARACTER,
    CDP_TAG_TXT_WORD,
    CDP_TAG_TXT_LINE,
    CDP_TAG_TXT_PARAGRAPH,
    CDP_TAG_TXT_TABLE,
    CDP_TAG_TXT_FORMULA,
    CDP_TAG_TXT_FOOTNOTE,
    CDP_TAG_TXT_HEADER,

    CDP_TAG_TXT_TITLE,
    CDP_TAG_TXT_ABSTRACT,
    CDP_TAG_TXT_BODY,
    CDP_TAG_TXT_TOC,
    CDP_TAG_TXT_CHAPTER,
    CDP_TAG_TXT_SECTION,
    CDP_TAG_TXT_CONCLUSION,
    CDP_TAG_TXT_AKNOWLEDGMENT,
    CDP_TAG_TXT_APPENDICE,
    CDP_TAG_TXT_GLOSSARY,

    CDP_TAG_TXT_AUTHOR,
    CDP_TAG_TXT_DATE,
    CDP_TAG_TXT_VERSION,
    CDP_TAG_TXT_COPYRIGHT,
    CDP_TAG_TXT_LICENSE,

    CDP_TAG_TEXT_COUNT
};


#endif
