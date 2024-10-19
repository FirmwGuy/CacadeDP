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


CDP_METADATA_STRUCT(cdpText,
    cdpAttribute    encoding:   3,  // Text encoding (UTF8, Unicode, Latin1, etc).
                    heading:    3,  // Heading level value for titles (H1, H2, etc).
                    listing:    2,  // Type of listing for table/list (enumerated, definition, etc).
                    formating:  3,  // Text format (bold, italic, etc).
                    font:       3,  // Recommended font family to use for rendering.
                    alignment:  2,  // Recommended horizontal text alignment (left, center, etc).
                    language:   6,  // Language of content (including programming language for scripts).
                    media:      3,  // Embedded media type (image, video, etc).

                    _reserved:  7;  // ToDO: expand to include DOM things?
);


enum _cdpTextEncoding {
    CDP_TXT_ENCOD_UTF8,         // The standard.
    CDP_TXT_ENCOD_UNICODE,      // A 2-byte wide C string.
    CDP_TXT_ENCOD_ISO8859,      // European (aka Latin1).
    CDP_TXT_ENCOD_SHIFT_JIS,    // Japanse.
    CDP_TXT_ENCOD_BIG5,         // Cantonese chinese.
    CDP_TXT_ENCOD_GB18030,      // Simplified chinese.
    CDP_TXT_ENCOD_ASCII64,      // A subset of ASCII (range 32-95).

    CDP_TXT_ENCOD_OTHER = 7
};

enum _cdpTextHeding {
    CDP_TXT_HEADING_NONE,       // Normal text.
    CDP_TXT_HEADING_1,          // Topmost title level.
    CDP_TXT_HEADING_2,          // Nested title level.
    CDP_TXT_HEADING_3,          // Nested nested title level...

    CDP_TXT_HEADING_OTHER = 7
};

enum _cdpTextListing {
    CDP_TXT_LIST_UNSORTED,
    CDP_TXT_LIST_ENUMERATION,
    CDP_TXT_LIST_DEFINITION,

    CDP_TXT_LIST_OTHER = 3
};

enum _cdpTextFormating {
    CDP_TXT_FMAT_NONE,
    CDP_TXT_FMAT_BOLD,
    CDP_TXT_FMAT_ITALIC,
    CDP_TXT_FMAT_UNDERLINE,
    CDP_TXT_FMAT_STRIKEOUT,

    CDP_TXT_FMAT_OTHER = 7
};

enum _cdpTextFont {
    CDP_TXT_FONT_DEFAULT,       // Use default font.
    CDP_TXT_FONT_SERIF,         // Decorative font (Georgia, Times New Roman).
    CDP_TXT_FONT_SANS,          // Modern look font (Arial, Helvetica, Verdana).
    CDP_TXT_FONT_MONO,          // Monospaced font (Console, Curier New).
    CDP_TXT_FONT_HAND,          // Handwriting font (Brush Script, Lucida Handwriting).
    CDP_TXT_FONT_TITLE,         // Display font for titles (Impact, Comic Sans).

    CDP_TXT_FONT_OTHER = 7
};

enum _cdpTextAlignment {
    CDP_TXT_ALIGN_DEFAULT,      // The default in western countries is "left".
    CDP_TXT_ALIGN_CENTER,
    CDP_TXT_ALIGN_OPPOSITE,     // The opposite alignment in western countries is "right".
    CDP_TXT_ALIGN_JUSTIFIED
};

enum _cdpTextLanguage {
    CDP_TXT_LANG_ENGLISH,       // The lingua franca.
    CDP_TXT_LANG_SPANISH,
    CDP_TXT_LANG_FRENCH,
    CDP_TXT_LANG_GERMAN,
    CDP_TXT_LANG_PORTUGESE,
    CDP_TXT_LANG_ITALIAN,

    CDP_TXT_LANG_INDI = 16,
    CDP_TXT_LANG_MANDARIN,
    CDP_TXT_LANG_CANTONESE,
    CDP_TXT_LANG_COREAN,
    CDP_TXT_LANG_JAPANESE,

    CDP_TXT_LANG_PYTHON = 32,
    CDP_TXT_LANG_JAVASCRIPT,

    CDP_TXT_LANG_OTHER = 63
};

enum _cdpTextMedia {
    CDP_TXT_MEDIA_NONE,
    CDP_TXT_MEDIA_IMAGE,
    CDP_TXT_MEDIA_AUDIO,
    CDP_TXT_MEDIA_VIDEO,

    CDP_TXT_MEDIA_OTHER = 7
};


enum _cdpTextTag {
    // Uses
    CDP_TXT_TAG_URL,
    CDP_TXT_TAG_METADATA,      // Used for defining metadata, comments, or annotations within the text (e.g., <meta>, comments in markdown).
    CDP_TXT_TAG_MEDIA,         // Represents media elements like images, videos, and embedded content (e.g., <img>, <iframe>).
    CDP_TXT_TAG_SCRIPT,        // The (executable) code part of this document.

    CDP_TXT_TAG_CHARACTER,
    CDP_TXT_TAG_WORD,
    CDP_TXT_TAG_LINE,
    CDP_TXT_TAG_PARAGRAPH,
    CDP_TXT_TAG_TABLE,
    CDP_TXT_TAG_FORMULA,
    CDP_TXT_TAG_FOOTNOTE,
    CDP_TXT_TAG_HEADER,

    CDP_TXT_TAG_TITLE,
    CDP_TXT_TAG_ABSTRACT,
    CDP_TXT_TAG_BODY,
    CDP_TXT_TAG_TOC,
    CDP_TXT_TAG_CHAPTER,
    CDP_TXT_TAG_SECTION,
    CDP_TXT_TAG_CONCLUSION,
    CDP_TXT_TAG_AKNOWLEDGMENT,
    CDP_TXT_TAG_APPENDICE,
    CDP_TXT_TAG_GLOSSARY,

    CDP_TXT_TAG_AUTHOR,
    CDP_TXT_TAG_DATE,
    CDP_TXT_TAG_VERSION,
    CDP_TXT_TAG_COPYRIGHT,
    CDP_TXT_TAG_LICENSE,

    // Children
    CDP_TXT_TAG_LENGTH,     // Non-ASCII text length in characters (NOT in bytes).
    CDP_TXT_TAG_HASH,       // Hash value of text content.

    // Agencies
    CDP_TXT_TAG_UPPERCASE,
    CDP_TXT_TAG_LOWERCASE,
    CDP_TXT_TAG_CAPITALIZE,
    CDP_TXT_TAG_TRIM,

    //
    CDP_TAG_TEXT_INI_COUNT
};


#define cdp_text_metadata_word()                                               \
    ((cdpText) {                                                               \
        .domain   = CDP_DOMAIN_TEXT,                                           \
        .tag      = CDP_TXT_TAG_WORD                                           \
    })


#define cdp_text_metadata_paragraph()                                          \
    ((cdpText) {                                                               \
        .domain   = CDP_DOMAIN_TEXT,                                           \
        .tag      = CDP_TXT_TAG_PARAGRAPH                                      \
    })


#endif
