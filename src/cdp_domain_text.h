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
                    media:      2,  // Embedded media type (image, video, etc).

                    _reserved:  8;  // ToDO: expand to include DOM things.
);


enum _cdpTextEncoding {
    CDP_TXT_ENCOD_UTF8,         // The standard.
    CDP_TXT_ENCOD_UNICODE,      // A 2-byte wide C string.
    CDP_TXT_ENCOD_ISO8859,      // European (aka Latin1).
    CDP_TXT_ENCOD_SHIFT_JIS,    // Japanse.
    CDP_TXT_ENCOD_BIG5,         // Cantonese chinese.
    CDP_TXT_ENCOD_GB18030,      // Simplified chinese.

    CDP_TXT_ENCOD_OTHER = 15
};

enum _cdpTextHeding {
    CDP_TXT_HEADING_1,          // Topmost title level.
    CDP_TXT_HEADING_2,          // Secondary title level...
    CDP_TXT_HEADING_3,
    CDP_TXT_HEADING_4,

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
    CDP_TEXT_ALIGN_DEFAULT,     // The default in western countries is "left".
    CDP_TEXT_ALIGN_CENTER,
    CDP_TEXT_ALIGN_OPPOSITE,    // The opposite alignment in western countries is "right".
    CDP_TEXT_ALIGN_JUSTIFIED
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
    CDP_TEXT_MEDIA_IMAGE,
    CDP_TEXT_MEDIA_AUDIO,
    CDP_TEXT_MEDIA_VIDEO,

    CDP_TEXT_MEDIA_OTHER = 3
};


enum _cdpTextTagID {
    // Children
    CDP_TAG_TXT_LENGTH,     // Non-ASCII text length in characters (NOT in bytes).

    // Uses
    CDP_TAG_TXT_URL,
    CDP_TAG_TXT_METADATA,      // Used for defining metadata, comments, or annotations within the text (e.g., <meta>, comments in markdown).
    CDP_TAG_TXT_MEDIA,         // Represents media elements like images, videos, and embedded content (e.g., <img>, <iframe>).
    CDP_TAG_TXT_SCRIPT         // The (executable) code part of this document.

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

    // Agencies
    CDP_TAG_TXT_UPPERCASE,
    CDP_TAG_TXT_LOWERCASE,
    CDP_TAG_TXT_CAPITALIZE,
    CDP_TAG_TXT_TRIM,


    CDP_TAG_TEXT_COUNT
};


static inline cdpText cdp_text_metadata_paragraph() {
    return (cdpText) {
        ,
    };
}


#endif
