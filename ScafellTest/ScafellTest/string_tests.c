//
//  utf8_tests.c
//  ScafellTest
//
//  Created by Tony on 21/06/2025.
//

#include <stdio.h>
#include "scuts.h"
#include "str.h"
#include "ucdb.h"

static SCF_OPERATION(op);

void init_string_tests(void) {
}

void cleanup_string_tests(void) {
    scf_complete(&op);
}

#if XXX
bool test_scf_substring(void) {
    bool result = true;
    scf_string s = scf_string_from_cstr(&op, "A£B€");
    utf8_iterator iter = utf8_iterator_at(&s, 1);
    scf_string ss = scf_substring(iter, 2);
    result &= ASSERT_EQ(2, ss.char_count);
    utf8_iterator ss_iter = scf_string_iterator(&ss);
    result &= ASSERT_EQ(0xa3c2, utf8_current(ss_iter));
    result &= ASSERT_TRUE(utf8_next(&ss_iter));
    result &= ASSERT_EQ(66, utf8_current(ss_iter));
    
    return result;
}

bool test_utf8_iterator_at(void) {
    scf_string s = scf_string_from_cstr(&op, "A£B");
    utf8_iterator iter = utf8_iterator_at(&s, 2);
    utf8_char ch = utf8_current(iter);
    bool result = ASSERT_EQ(66, ch);
    result &= iter.char_index == 2;
    
    iter = utf8_iterator_at(&s, 999);
    result &= ASSERT_TRUE(utf8_is_at_end(iter));
    
    return result;
}

bool test_utf8_current_and_next(void) {
    scf_string s = scf_string_from_cstr(&op, "A£B");
    utf8_iterator iter = scf_string_iterator(&s);
    bool result = true;
    utf8_char ch;
    scf_char_info info;
    
    result &= ASSERT_EQ(0, iter.char_index);
    ch = utf8_current(iter);
    result &= ASSERT_TRUE(utf8_next(&iter));
    info = scf_get_char_info(ch);
    result &= ASSERT_EQ(65, info.codepoint);
    
    result &= ASSERT_EQ(1, iter.char_index);
    ch = utf8_current(iter);
    result &= ASSERT_TRUE(utf8_next(&iter));
    info = scf_get_char_info(ch);
    result &= ASSERT_EQ(0xA3, info.codepoint);
    
    result &= ASSERT_EQ(2, iter.char_index);
    ch = utf8_current(iter);
    result &= ASSERT_FALSE(utf8_next(&iter));
    info = scf_get_char_info(ch);
    result &= ASSERT_EQ(66, info.codepoint);
    
    result &= (ASSERT_TRUE(utf8_is_at_end(iter)));
    
    return result;
}
#endif

bool test_scf_utf8_char_from_codepoint_utf8(void) {
    scf_char ch;
    bool result;
    
    ch = scf_char_from_codepoint(65, SCF_UTF8);
    result = ASSERT_EQ(SCF_UTF8, ch.encoding)
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(65, ch.bytes[0]);
    
    ch = scf_char_from_codepoint(0x46D, SCF_UTF8);
    result = result && ASSERT_EQ(SCF_UTF8, ch.encoding)
    && ASSERT_EQ(2, ch.byte_count)
    && ASSERT_EQ(0xAD, ch.bytes[1])
    && ASSERT_EQ(0xD1, ch.bytes[0]);
    
    ch = scf_char_from_codepoint(0x16A0, SCF_UTF8);
    result = result && ASSERT_EQ(SCF_UTF8, ch.encoding)
    && ASSERT_EQ(3, ch.byte_count)
    && ASSERT_EQ(0xA0, ch.bytes[2])
    && ASSERT_EQ(0x9A, ch.bytes[1])
    && ASSERT_EQ(0xE1, ch.bytes[0]);
    
    ch = scf_char_from_codepoint(0x1F608, SCF_UTF8);
    result = result && ASSERT_EQ(SCF_UTF8, ch.encoding)
    && ASSERT_EQ(4, ch.byte_count)
    && ASSERT_EQ(0x88, ch.bytes[3])
    && ASSERT_EQ(0x98, ch.bytes[2])
    && ASSERT_EQ(0x9F, ch.bytes[1])
    && ASSERT_EQ(0xF0, ch.bytes[0]);
    
    return result;
}

bool test_scf_codepoint_from_utf8_char(void) {
    bool result = true;
    
    {
        scf_char ch = {SCF_UTF8, 1, {65}};
        scf_codepoint cp = scf_codepoint_from_char(ch);
        result = result && ASSERT_EQ(cp, 65);
    }

    {
        scf_char ch = {SCF_UTF8, 2, {0xD1, 0xAD}};
        scf_codepoint cp = scf_codepoint_from_char(ch);
        result = result && ASSERT_EQ(cp, 0x46D);
    }

    {
        scf_char ch = {SCF_UTF8, 3, {0xE1, 0x9A, 0xA0}};
        scf_codepoint cp = scf_codepoint_from_char(ch);
        result = result && ASSERT_EQ(cp, 0x16A0);
    }

    {
        scf_char ch = {SCF_UTF8, 4, {0xF0, 0x9F, 0x98, 0x88}};
        scf_codepoint cp = scf_codepoint_from_char(ch);
        result = result && ASSERT_EQ(cp, 0x1F608);
    }

    return result;
}


static bool check_info(scf_char_info expected) {
    scf_char_info actual;
    bool result = true;
    actual = scf_get_char_info(expected.codepoint);
    result &= ASSERT_EQ(expected.codepoint, actual.codepoint);
    result &= ASSERT_EQ(expected.category, actual.category);
    result &= ASSERT_EQ(expected.tc_codepoint, actual.tc_codepoint);
    result &= ASSERT_EQ(expected.lc_codepoint, actual.lc_codepoint);
    result &= ASSERT_EQ(expected.digit_value, actual.digit_value);
    return result;
}

bool test_char_info(void) {
    bool result = true;
    scf_char_info info1 = {65, UC_LETTER | UC_UPPER, -1, 65, 97, 65};
    result &= check_info(info1);
    
    scf_char_info info2 = {0x1AE, UC_LETTER | UC_UPPER, -1, 0x1AE, 0x288, 0x1AE};
    result &= check_info(info2);
    
    scf_char_info info3 = {0xB6F, UC_DIGIT, 9, 0xB6F, 0xB6F, 0xB6F};
    result &= check_info(info3);
    
    return result;
}

#if XXX

typedef struct {
    int codepoint;
    scf_char_category category;
    int digit_value;
    int uc_codepoint;
    int lc_codepoint;
    int tc_codepoint;
} scf_char_info;


bool test_string_from_cstr_valid(void) {
    const char *cstr = "£xyz♚";
    scf_string s = scf_string_from_cstr(&op, cstr);
    bool result = true;
    result &= ASSERT_TRUE(s.is_utf8);
    result &= ASSERT_EQ(5, s.char_count);
    result &= ASSERT_EQ(8, scf_string_byte_count(&s));
    return result;
}

bool test_string_from_cstr_invalid(void) {
    const char cstr[] = {'a', 0xe2, 0x99, 0x0};
    scf_string s = scf_string_from_cstr(&op, cstr);
    bool result = true;
    result &= ASSERT_FALSE(s.is_utf8);
    result &= ASSERT_EQ(3, scf_string_byte_count(&s));
    return result;
}

#endif

BEGIN_TEST_GROUP(string_tests)
    INIT(init_string_tests)
    CLEANUP(cleanup_string_tests)
    TEST(test_scf_utf8_char_from_codepoint_utf8)
    TEST(test_scf_codepoint_from_utf8_char)
    TEST(test_char_info)
/*
    TEST(test_char_info)
    TEST(test_invalid_char_info)
    TEST(test_string_from_cstr_valid)
    TEST(test_string_from_cstr_invalid)
    TEST(test_utf8_current_and_next)
    TEST(test_utf8_prev)
    TEST(test_scf_substring)
    TEST(test_utf8_iterator_at)
 */
END_TEST_GROUP
