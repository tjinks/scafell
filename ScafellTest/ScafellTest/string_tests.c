//
//  utf8_tests.c
//  ScafellTest
//
//  Created by Tony on 21/06/2025.
//

#include <stdio.h>
#include <scuts.h>
#include "str.h"
#include "ucdb.h"

static SCF_OPERATION(op);

BEGIN_TEST_GROUP(string_tests)
    INIT(init_string_tests)
    CLEANUP(cleanup_string_tests)
    TEST(test_utf8_from_codepoint)
    TEST(test_char_info)
    TEST(test_invalid_char_info)
    TEST(test_string_from_cstr_valid)
    TEST(test_string_from_cstr_invalid)
END_TEST_GROUP

void init_string_tests(void) {
    scf_ucdb_init();
}

void cleanup_string_tests(void) {
    scf_complete(&op);
}

bool test_utf8_from_codepoint(void) {
    return ASSERT_EQ(65, utf8_from_codepoint(65))
    && ASSERT_EQ(0xADD1, utf8_from_codepoint(0x46D))
    && ASSERT_EQ(0xA09AE1, utf8_from_codepoint(0x16A0))
    && ASSERT_TRUE(0x88989FF0 == utf8_from_codepoint(0x1F608)) // ASSERT_EQ fails here due to signed/unsigned issues
    ;
}

static bool check_info(scf_char_info expected) {
    scf_char_info actual;
    bool result = true;
    actual = scf_get_char_info(expected.base);
    result &= ASSERT_EQ(expected.codepoint, actual.codepoint);
    result &= ASSERT_EQ(expected.base, actual.base);
    result &= ASSERT_EQ(expected.title, actual.title);
    result &= ASSERT_EQ(expected.lower, actual.lower);
    result &= ASSERT_EQ(expected.digit_value, actual.digit_value);
    result &= ASSERT_EQ(expected.category, actual.category);
    return result;
}

bool test_char_info(void) {
    bool result = true;
    scf_char_info info1 = {65, 97, 65, 65, 65, -1, UC_LETTER | UC_UPPER};
    result &= check_info(info1);
    
    scf_char_info info2 = {0x9FD0, 0xBFD0, 0x9FD0, 0x9FD0, 0x41F, -1, UC_LETTER | UC_UPPER};
    result &= check_info(info2);
    
    scf_char_info info3 = {0xA8D9, 0xA8D9, 0xA8D9, 0xA8D9, 0x668, 8, UC_DIGIT};
    result &= check_info(info3);
    
    scf_char_info info4 = {0x80AC9FF0, 0x80AC9FF0, 0x80AC9FF0, 0x80AC9FF0, 0x1FB00, -1, UC_OTHER};
    result &= check_info(info4);
    return result;
}

bool test_invalid_char_info(void) {
    scf_char_info info = scf_get_char_info(0xF09FAB97);
    return ASSERT_EQ(UTF8_INVALID, info.base) && ASSERT_EQ(UC_NONE, info.category);
}

bool test_string_from_cstr_valid(void) {
    const char *cstr = "£xyz♚";
    scf_string s = scf_string_from_cstr(&op, cstr);
    bool result = true;
    result &= ASSERT_EQ(5, s.char_count);
    result &= ASSERT_EQ(8, scf_string_byte_count(&s));
    return result;
}

bool test_string_from_cstr_invalid(void) {
    const char cstr[] = {'a', 0xe2, 0x99, 0x0};
    scf_string s = scf_string_from_cstr(&op, cstr);
    bool result = true;
    result &= ASSERT_EQ(-1, s.char_count);
    result &= ASSERT_EQ(3, scf_string_byte_count(&s));
    return result;
}
