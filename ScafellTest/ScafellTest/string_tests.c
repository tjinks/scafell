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
    
    ch = scf_char_from_codepoint(65, SCF_UTF8, NULL);
    result = ASSERT_EQ(SCF_UTF8, ch.encoding)
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(65, ch.bytes[0]);
    
    ch = scf_char_from_codepoint(0x46D, SCF_UTF8, NULL);
    result = result && ASSERT_EQ(SCF_UTF8, ch.encoding)
    && ASSERT_EQ(2, ch.byte_count)
    && ASSERT_EQ(0xAD, ch.bytes[1])
    && ASSERT_EQ(0xD1, ch.bytes[0]);
    
    ch = scf_char_from_codepoint(0x16A0, SCF_UTF8, NULL);
    result = result && ASSERT_EQ(SCF_UTF8, ch.encoding)
    && ASSERT_EQ(3, ch.byte_count)
    && ASSERT_EQ(0xA0, ch.bytes[2])
    && ASSERT_EQ(0x9A, ch.bytes[1])
    && ASSERT_EQ(0xE1, ch.bytes[0]);
    
    ch = scf_char_from_codepoint(0x1F608, SCF_UTF8, NULL);
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

bool test_string_from_bytes_utf8(void) {
    const char *cstr = "£ABC♚";
    scf_string *s = scf_string_from_bytes(&op, cstr, strlen(cstr), SCF_UTF8, NULL);
    ASSERT_EQ(5, s->char_count);
    ASSERT_EQ(8, s->buf.size);
    int i = 0;
    unsigned char *data = s->buf.data;
    return ASSERT_EQ(0xC2, data[i++])
    && ASSERT_EQ(0xA3, data[i++])
    && ASSERT_EQ(65, data[i++])
    && ASSERT_EQ(66, data[i++])
    && ASSERT_EQ(67, data[i++])
    && ASSERT_EQ(0xE2, data[i++])
    && ASSERT_EQ(0x99, data[i++])
    && ASSERT_EQ(0x9A, data[i++])
    ;
}

bool test_string_from_bytes_utf16(void) {
    const unsigned char bytes[] = {0, 0xA3, 0, 65, 0, 66, 0, 67, 0x26, 0x5A};
    scf_string *s = scf_string_from_bytes(&op, bytes, sizeof(bytes), SCF_UTF16_BE, NULL);
    ASSERT_EQ(5, s->char_count);
    ASSERT_EQ(8, s->buf.size);
    int i = 0;
    unsigned char *data = s->buf.data;
    return ASSERT_EQ(0xC2, data[i++])
    && ASSERT_EQ(0xA3, data[i++])
    && ASSERT_EQ(65, data[i++])
    && ASSERT_EQ(66, data[i++])
    && ASSERT_EQ(67, data[i++])
    && ASSERT_EQ(0xE2, data[i++])
    && ASSERT_EQ(0x99, data[i++])
    && ASSERT_EQ(0x9A, data[i++])
    ;
}

bool test_string_from_bytes_invalid(void) {
    const unsigned char bytes[] = {0xC2, 0xA3, 65, 0x80, 66};
    bool result;
    SCF_TRY(ec) {
        scf_string_from_bytes(&op, bytes, sizeof(bytes), SCF_UTF8, &ec);
        result = ASSERT_FAILURE("Didn't expect to get here!");
    }
    SCF_CATCH {
        result = ASSERT_EQ(SCF_INVALID_STRING_OPERATION, ec.err_info.code);
    }
    SCF_END_TRY

    return result;
}

bool test_iterator_next(void) {
    const char *cstr = "£ABC♚";
    scf_string *s = scf_string_from_bytes(&op, cstr, strlen(cstr), SCF_UTF8, NULL);
    scf_string_iterator iter = scf_string_start(s);
    scf_char ch;
    bool result = true;
    
    result = result && ASSERT_TRUE(scf_string_next(&iter, &ch))
    && ASSERT_EQ(2, ch.byte_count)
    && ASSERT_EQ(0xC2, ch.bytes[0])
    && ASSERT_EQ(0xA3, ch.bytes[1])
    ;
    
    result = result && ASSERT_TRUE(scf_string_next(&iter, &ch))
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(65, ch.bytes[0])
    ;
    
    result = result && ASSERT_TRUE(scf_string_next(&iter, &ch))
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(66, ch.bytes[0])
    ;
    
    result = result && ASSERT_TRUE(scf_string_next(&iter, &ch))
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(67, ch.bytes[0])
    ;
    
    result = result && ASSERT_TRUE(scf_string_next(&iter, &ch))
    && ASSERT_EQ(3, ch.byte_count)
    && ASSERT_EQ(0xE2, ch.bytes[0])
    && ASSERT_EQ(0x99, ch.bytes[1])
    && ASSERT_EQ(0x9A, ch.bytes[2])
    ;
    
    result = result && ASSERT_FALSE(scf_string_next(&iter, &ch));

    return result;
}

bool test_iterator_prev(void) {
    const char *cstr = "£ABC♚";
    scf_string *s = scf_string_from_bytes(&op, cstr, strlen(cstr), SCF_UTF8, NULL);
    scf_string_iterator iter = scf_string_end(s);
    scf_char ch;
    bool result = true;
    
    result = result && ASSERT_TRUE(scf_string_prev(&iter, &ch))
    && ASSERT_EQ(3, ch.byte_count)
    && ASSERT_EQ(0xE2, ch.bytes[0])
    && ASSERT_EQ(0x99, ch.bytes[1])
    && ASSERT_EQ(0x9A, ch.bytes[2])
    ;
    
    result = result && ASSERT_TRUE(scf_string_prev(&iter, &ch))
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(67, ch.bytes[0])
    ;
    
    result = result && ASSERT_TRUE(scf_string_prev(&iter, &ch))
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(66, ch.bytes[0])
    ;
    
    result = result && ASSERT_TRUE(scf_string_prev(&iter, &ch))
    && ASSERT_EQ(1, ch.byte_count)
    && ASSERT_EQ(65, ch.bytes[0])
    ;
    
    result = result && ASSERT_TRUE(scf_string_prev(&iter, &ch))
    && ASSERT_EQ(2, ch.byte_count)
    && ASSERT_EQ(0xC2, ch.bytes[0])
    && ASSERT_EQ(0xA3, ch.bytes[1])
    ;
    
    result = result && ASSERT_FALSE(scf_string_prev(&iter, &ch));

    return result;
}

bool test_string_append(void) {
    scf_string *s1 = scf_string_from_cstr(&op, "£123");
    scf_string *s2 = scf_string_from_cstr(&op, ".45");
    scf_string_append(s1, s2);
    return ASSERT_EQ(7, s1->char_count)
    && ASSERT_EQ(8, s1->buf.size)
    && ASSERT_EQ(0, memcmp(s1->buf.data, "£123.45", 8))
    ;
}

bool test_char_cmp(void) {
    scf_char utf8_pound = {SCF_UTF8, 2, {0xC2, 0xA3}};
    scf_char utf8_one = scf_ascii('1');
    scf_char utf16_pound = {SCF_UTF16_BE, 2, {0x0, 0xA3}};
    return ASSERT_EQ(0, scf_char_cmp(utf8_pound, utf16_pound))
    && ASSERT_EQ(-1, scf_char_cmp(utf8_one, utf16_pound))
    && ASSERT_EQ(1, scf_char_cmp(utf16_pound, utf8_one))
    ;
}

bool test_string_cmp(void) {
    scf_string *s1 = scf_string_from_cstr(&op, "abc");
    scf_string *s2 = scf_string_from_cstr(&op, "abc");
    scf_string *s3 = scf_string_from_cstr(&op, "abc£");
    scf_string *s4 = scf_string_from_cstr(&op, "aa");
    
    return ASSERT_EQ(0, scf_string_cmp(s1, s2))
    && ASSERT_EQ(-1, scf_string_cmp(s1, s3))
    && ASSERT_EQ(1, scf_string_cmp(s3, s1))
    && ASSERT_EQ(1, scf_string_cmp(s1, s4))
    && ASSERT_EQ(-1, scf_string_cmp(s4, s1))
    ;
}

bool test_substring(void) {
    scf_string *s = scf_string_from_cstr(&op, "abc £123");
    scf_string_iterator iter = scf_string_iterator_at(s, 4);
    scf_string *substring = scf_substring(iter, 3);
    bool result = ASSERT_EQ(3, substring->char_count)
    && ASSERT_EQ(4, substring->buf.size)
    ;
    
    char *cstr = scf_string_to_cstr(substring);
    result = result && ASSERT_EQ("£12", cstr);
    return result;
}

bool test_string_clone(void) {
    SCF_OPERATION(op1);
    scf_string *s = scf_string_from_cstr(&op, "£123");
    scf_string *clone = scf_string_clone(&op1, s);
    bool result = ASSERT_FALSE(s == clone) && ASSERT_EQ("£123", scf_string_to_cstr(clone));
    scf_complete(&op1);
    return result;
}

bool test_stringlist_add_and_get(void) {
    scf_stringlist *list = scf_stringlist_create(&op);
    scf_stringlist_add(list, scf_string_from_cstr(&op, "s0"));
    scf_stringlist_add(list, scf_string_from_cstr(&op, "s1"));
    bool result = ASSERT_EQ(2, scf_stringlist_size(list));
    scf_string *s0a = scf_stringlist_get(list, 0);
    scf_string *s0b = scf_stringlist_get(list, 0);
    scf_string *s1a = scf_stringlist_get(list, 1);
    scf_string *s1b = scf_stringlist_get_copy(&op, list, 1);
    result = result && ASSERT_EQ("s0", scf_string_to_cstr(s0a));
    result = result && ASSERT_EQ("s0", scf_string_to_cstr(s0b));
    result = result && ASSERT_EQ("s1", scf_string_to_cstr(s1a));
    result = result && ASSERT_EQ("s1", scf_string_to_cstr(s1b));
    
    result = result && ASSERT_TRUE(s0a == s0b);
    result = result && ASSERT_FALSE(s1a == s1b);

    return result;
}

bool test_stringlist_insert(void) {
    scf_stringlist *list = scf_stringlist_create(&op);
    scf_stringlist_add_cstr(list, "abc");
    scf_stringlist_add_cstr(list, "123");
    scf_stringlist_insert(list, scf_string_from_cstr(&op, "ABC"), 1);
    
    bool result = ASSERT_EQ(3, scf_stringlist_size(list));
    result = ASSERT_EQ("abc", scf_string_to_cstr(scf_stringlist_get(list, 0)));
    result = ASSERT_EQ("ABC", scf_string_to_cstr(scf_stringlist_get(list, 1)));
    result = ASSERT_EQ("123", scf_string_to_cstr(scf_stringlist_get(list, 2)));

    return result;
}

bool test_stringlist_remove(void) {
    scf_stringlist *list = scf_stringlist_create(&op);
    scf_stringlist_add_cstr(list, "abc");
    scf_stringlist_add_cstr(list, "123");
    scf_stringlist_add_cstr(list, "xyz");
    scf_stringlist_remove(list, 1);
    
    bool result = ASSERT_EQ(2, scf_stringlist_size(list));
    result = ASSERT_EQ("abc", scf_string_to_cstr(scf_stringlist_get(list, 0)));
    result = ASSERT_EQ("xyz", scf_string_to_cstr(scf_stringlist_get(list, 1)));

    return result;
}

bool test_stringlist_sort(void) {
    scf_stringlist *list = scf_stringlist_create(&op);
    scf_stringlist_add_cstr(list, "abc");
    scf_stringlist_add_cstr(list, "ab");
    scf_stringlist_add_cstr(list, "123");
    scf_stringlist_add_cstr(list, "ABC");
    scf_stringlist_sort(list, NULL);
    
    bool result = ASSERT_EQ("123", scf_string_to_cstr(scf_stringlist_get(list, 0)));
    result = ASSERT_EQ("ABC", scf_string_to_cstr(scf_stringlist_get(list, 1)));
    result = ASSERT_EQ("ab", scf_string_to_cstr(scf_stringlist_get(list, 2)));
    result = ASSERT_EQ("abc", scf_string_to_cstr(scf_stringlist_get(list, 3)));

    return result;
}

BEGIN_TEST_GROUP(string_tests)
    INIT(init_string_tests)
    CLEANUP(cleanup_string_tests)
    TEST(test_scf_utf8_char_from_codepoint_utf8)
    TEST(test_scf_codepoint_from_utf8_char)
    TEST(test_char_info)
    TEST(test_string_from_bytes_utf8)
    TEST(test_string_from_bytes_utf16)
    TEST(test_string_from_bytes_invalid)
    TEST(test_iterator_next)
    TEST(test_iterator_prev)
    TEST(test_string_append)
    TEST(test_char_cmp)
    TEST(test_string_cmp)
    TEST(test_string_cmp)
    TEST(test_substring)
    TEST(test_string_clone)
    TEST(test_stringlist_add_and_get)
    TEST(test_stringlist_insert)
    TEST(test_stringlist_remove)
    TEST(test_stringlist_sort)
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
