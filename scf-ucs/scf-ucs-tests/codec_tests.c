//
//  codec_tests.c
//  scf-ucs-tests
//
//  Created by Tony on 27/07/2025.
//

#include "scuts.h"
#include "codecs.h"
#include "mmgt.h"


static SCF_OPERATION(op);

static void cleanup(void) {
    scf_complete(&op);
}

static bool test_codepoint_to_utf8_char(void) {
    bool result = ASSERT_EQ(0x41, ucs_codepoint_to_utf8(0x41));
    result &= ASSERT_EQ(0xC2A3, ucs_codepoint_to_utf8(0xA3));
    result &= ASSERT_EQ(0xEFBFBD, ucs_codepoint_to_utf8(0xFFFD));
    result &= ASSERT_EQ(0xF0908299, ucs_codepoint_to_utf8(0x10099));

    return result;
}

static bool test_utf8_char_to_codepoint(void) {
    bool result = ASSERT_EQ(0x41, ucs_utf8_to_codepoint(0x41));
    result &= ASSERT_EQ(0xA3, ucs_utf8_to_codepoint(0xC2A3));
    result &= ASSERT_EQ(0xFFFD, ucs_utf8_to_codepoint(0xEFBFBD));
    result &= ASSERT_EQ(0x10099, ucs_utf8_to_codepoint(0xF0908299));
    
    return result;
}

static bool test_valid_utf8_encoding(void) {
    unsigned char data[] = {0x41, 0xC2, 0xA3, 0xF3, 0xA0, 0x81, 0xA1};
    scf_buffer source = scf_buffer_wrap(data, 7);
    scf_buffer target = scf_buffer_create(&op, 0);
    size_t result = ucs_encode(&source, 0, UCS_UTF8, &target, UCS_UTF8);
    ASSERT_EQ(3, result);
    result = result && ASSERT_EQ(7, target.size);
    bool data_matches = memcmp(data, target.data, 7) == 0;
    result = result && ASSERT_TRUE(data_matches);
    return result;
}

/*
 * The following are encodings (without null terminator) of the string 'A£<U10041>'
 *
 * (U10041 = LINEAR B SYLLABLE B043 A3)
 */

static const unsigned char utf8_encoded_test_data[] = {0x41, 0xC2, 0xA3, 0xF0, 0x90, 0x81, 0x8D};
static const unsigned char utf16_le_encoded_test_data[] = {0x41, 0x00, 0xA3, 0x00, 0x00, 0xD8, 0x4D, 0xDC};

#define UTF8_LEN (sizeof(utf8_encoded_test_data) / sizeof(unsigned char))
#define UTF16_LEN (sizeof(utf16_le_encoded_test_data) / sizeof(unsigned char))

static bool test_utf16_le_to_utf8(void) {
    scf_buffer source = scf_buffer_wrap((void *)utf16_le_encoded_test_data, UTF16_LEN);
    scf_buffer target = scf_buffer_create(&op, 0);
    size_t result = ucs_encode(&source, 0, UCS_UTF16 | UCS_LE, &target, UCS_UTF8);
    ASSERT_EQ(3, result);
    
    result = result && ASSERT_EQ(UTF8_LEN, target.size);
    result = result && ASSERT_EQ(0, memcmp(target.data, utf8_encoded_test_data, UTF8_LEN));
    return result;
}

static bool test_utf8_to_utf16_le(void) {
    scf_buffer source = scf_buffer_wrap((void *)utf8_encoded_test_data, UTF8_LEN);
    scf_buffer target = scf_buffer_create(&op, 0);
    size_t result = ucs_encode(&source, 0, UCS_UTF8, &target, UCS_UTF16 | UCS_LE);
    ASSERT_EQ(3, result);
    
    result = result && ASSERT_EQ(UTF16_LEN, target.size);
    result = result && ASSERT_EQ(0, memcmp(target.data, utf16_le_encoded_test_data, UTF16_LEN));
    return result;
}

BEGIN_TEST_GROUP(codec_tests)
CLEANUP(cleanup)
TEST(test_valid_utf8_encoding)
TEST(test_codepoint_to_utf8_char)
TEST(test_utf8_char_to_codepoint)
TEST(test_utf8_to_utf16_le)
TEST(test_utf16_le_to_utf8)
END_TEST_GROUP

