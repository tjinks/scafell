//
//  ucs_string_tests.c
//  scf-ucs-tests
//
//  Created by Tony on 10/08/2025.
//

#include <string.h>
#include "scuts.h"
#include "ucs_string.h"
#include "mmgt.h"
#include "machine_info.h"

// UTFF8 encodings of £ (0xa3) and ALAF (0x800)
#define POUND "\xc2\xa3"
#define ALAF "\xe0\xa0\x80"

static SCF_OPERATION(op);

static const char data[] = {0x41, 0xEF, 0xBF, 0xBD, 0xF0, 0x90,0x8E, 0xBD, 0xC2, 0xA3, 0};
static size_t datalen;

static void init(void) {
    scf_initialise_machine_info();
    datalen = strlen(data);
}

static void cleanup(void) {
    scf_complete(&op);
}

static bool test_from_bytes(void) {
    ucs_string s = ucs_from_bytes(&op, data, datalen, UCS_UTF8);
    return ASSERT_EQ(4, s.char_count);
}

static bool test_from_cstr(void) {
    ucs_string s = ucs_from_cstr(&op, "1" POUND);
    bool result = ASSERT_EQ(2, s.char_count);
    result &= ASSERT_EQ(4, s.bytes.size);
    result &= ASSERT_EQ(49, s.bytes.data[0]);
    result &= ASSERT_EQ(0xc2, s.bytes.data[1]);
    result &= ASSERT_EQ(0xa3, s.bytes.data[2]);
    result &= ASSERT_EQ(0, s.bytes.data[3]);
    return result;
}

static bool test_from_wstr(void) {
    static const unsigned char expected[] = {0x31, 0xC2, 0xA3, 0xF0, 0x90, 0x81, 0x8D, 0x00};

/*
 * The following assignment results in different code in Visual Studio depending on the encoding of
 * the source file, so we provide an explicit UTF16 encoding below.
 */
#ifndef _MSC_VER
    static const wchar_t input[] = L"1£𐁍";
#else
    static const wchar_t input[] = { 0x31, 0xa3, 0xd800, 0xdc4d, 0 };
#endif
    ucs_string s = ucs_from_wstr(&op, input);
    bool result = ASSERT_EQ(3, s.char_count);
    result &= ASSERT_EQ(8, s.bytes.size);
    result &= ASSERT_EQ(0, memcmp(expected, s.bytes.data, s.bytes.size));
    return result;
}

static bool test_iterator(void) {
    ucs_string s = ucs_from_bytes(&op, data, datalen, UCS_UTF8);
    ucs_iterator iter = ucs_get_iterator(&s);
    bool result = true;
    ucs_utf8_char ch;
    for (int i = 0 ; i < 4; i++) {
        result = result && ASSERT_TRUE(ucs_next(&iter, &ch));
        switch (i) {
            case 0:
                result = result && ASSERT_EQ(0x41, ch);
                break;
            case 1:
                result = result && ASSERT_EQ(0xEFBFBD, ch);
                break;
            case 2:
                result = result && ASSERT_EQ(0xF0908EBD, ch);
                break;
            case 3:
                result = result && ASSERT_EQ(0xC2A3, ch);
                break;
        }
    }
    
    result = result && ASSERT_FALSE(ucs_next(&iter, &ch));
    return result;
}

static bool test_append(void) {
    ucs_string s1 = ucs_from_cstr(&op, "1" POUND);
    ucs_string s2 = ucs_from_cstr(&op, ALAF);
    ucs_string expected = ucs_from_cstr(&op, "1" POUND ALAF);
    
    ucs_string_append(&s1, &s2);
    bool result = ASSERT_EQ(3, s1.char_count);
    result &= ASSERT_EQ(0, ucs_compare(&s1, &expected));
    return result;
}

static bool test_substring(void) {
    ucs_string s1 = ucs_from_cstr(&op, "ABC" POUND "34");
    ucs_iterator iter = ucs_get_iterator_at(&s1, 2);
    ucs_string s2 = ucs_substring(&iter, 3);
    ucs_string expected = ucs_from_cstr(&op, "C" POUND "3");
    bool result = ASSERT_EQ(3, s2.char_count);
    result &= ASSERT_EQ(0, ucs_compare(&expected, &s2));
    return result;
}

static bool test_overlength_substring(void) {
    ucs_string s1 = ucs_from_cstr(&op, "ABC" POUND "34");
    ucs_iterator iter = ucs_get_iterator_at(&s1, 2);
    ucs_string s2 = ucs_substring(&iter, 999);
    ucs_string expected = ucs_from_cstr(&op, "C" POUND "34");
    bool result = ASSERT_EQ(4, s2.char_count);
    result &= ASSERT_EQ(0, ucs_compare(&expected, &s2));
    return result;
}

BEGIN_TEST_GROUP(ucs_string_tests)
INIT(init)
CLEANUP(cleanup)
TEST(test_from_bytes)
TEST(test_iterator)
TEST(test_from_cstr)
TEST(test_from_wstr)
TEST(test_append)
TEST(test_substring)
TEST(test_overlength_substring)
END_TEST_GROUP

