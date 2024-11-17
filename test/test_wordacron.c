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


#include "test.h"
#include "cdp_record.h"


static void test_wordacron_text(const char* text) {
}


static inline size_t get_trimmed_length(const char* s) {
    while (*s == ' ')   s++;
    size_t len = strlen(s);
    while (len > 0  &&  s[len - 1] == ' ') {
        len--;
    }
    return len;
}

#define CODABLE_MIN     2
#define PRINTABLE_MIN   5

static void test_wordacron_coding(void) {
    const char* acronysm_tests[] = {
        " ",
        "TOOLONGNAMEEXCEEDS",

        " TEST",
        "SPACE X   ",
        "TRIMMED   ",

        "HELLO",
        "WORLD!",
        "?",
        "ACRONYS()",
        "LONGNAME+"
    };
    const char* word_tests[] = {
        " ",
        "toolongtoencodeproperly",

        " with space",
        "trailing     ",
        "    trimthis   ",

        "hello",
        "world.",
        ":",
        "valid_word",
        "punctu-ated"
    };

    char   decoded[12];
    cdpID  encoded;
    size_t decoded_length;

    for (size_t i = 0;  i < cdp_lengthof(acronysm_tests);  i++) {
        encoded = cdp_text_to_acronysm(acronysm_tests[i]);
        if (encoded) {
            decoded_length = cdp_acronysm_to_text(encoded, decoded);

            assert_size(decoded_length, ==, get_trimmed_length(acronysm_tests[i]));
            if (i < PRINTABLE_MIN)
                assert_string_not_equal(decoded, acronysm_tests[i]);
            else
                assert_string_equal(decoded, acronysm_tests[i]);
        } else {
            assert_size(i, <=, CODABLE_MIN);
        }
    }

    for (size_t i = 0;  i < cdp_lengthof(word_tests);  i++) {
        encoded = cdp_text_to_word(word_tests[i]);
        if (encoded) {
            decoded_length = cdp_word_to_text(encoded, decoded);

            assert_size(decoded_length, ==, get_trimmed_length(word_tests[i]));
            if (i < PRINTABLE_MIN)
                assert_string_not_equal(decoded, word_tests[i]);
            else
                assert_string_equal(decoded, word_tests[i]);
        } else {
            assert_size(i, <=, CODABLE_MIN);
        }
    }
}


MunitResult test_wordacron(const MunitParameter params[], void* user_data_or_fixture) {
    const char* param_value = munit_parameters_get(params, "text");
    if (param_value) {
        test_wordacron_text(param_value);
    } else {
        test_wordacron_coding();
    }

    return MUNIT_OK;
}

