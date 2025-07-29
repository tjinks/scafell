//
//  codec_tests.c
//  scf-ucs-tests
//
//  Created by Tony on 27/07/2025.
//

#include "scuts.h"
#include "codecs.h"
#include "mmgt.h"
#include "scuts.h"


static SCF_OPERATION(op);

static void cleanup(void) {
    scf_complete(&op);
}

static bool test_valid_utf8_encoding(void) {
    unsigned char data[] = {0x41, 0xC2, 0xA3, 0xF3, 0xA0, 0x81, 0xA1};
    scf_buffer encoded = scf_buffer_create(&op, 10);
    bool result = ucs_encode_bytes(data, 7, UCS_UTF8, &encoded, UCS_UTF8);
    ASSERT_TRUE(result);
    result = result && ASSERT_EQ(7, encoded.size);
    bool data_matches = memcmp(data, encoded.data, 7) == 0;
    result = result && data_matches;
    return result;
}

static bool test_invalid_utf8_encoding(void) {
    unsigned char data[] = {0x41, 0xC2, 0xA3, 0xF3, 0xA0, 0x81, 0xA1};
    scf_buffer encoded = scf_buffer_create(&op, 10);
    bool result = ucs_encode_bytes(data, 6, UCS_UTF8, &encoded, UCS_UTF8);
    ASSERT_FALSE(result);
    return !result;
}

static bool test_utf8_to_utf16(void) {
    const char *utf8 = "A£𐁍";
    scf_buffer utf16 = scf_buffer_create(&op, 0);
    bool result = ucs_encode_bytes(utf8, strlen(utf8), UCS_UTF8, &utf16, UCS_UTF16 | UCS_LE);
    ASSERT_TRUE(result);
    //Unicode: U+1004D, UTF-8: F0 90 81 8D
    
    result = result && ASSERT_EQ(8, utf16.size);
    result = result && ASSERT_EQ(0x41, utf16.data[0]);
    result = result && ASSERT_EQ(0x00, utf16.data[1]);
    result = result && ASSERT_EQ(0xA3, utf16.data[2]);
    result = result && ASSERT_EQ(0x00, utf16.data[3]);
    result = result && ASSERT_EQ(0x00, utf16.data[4]);
    result = result && ASSERT_EQ(0xD8, utf16.data[5]);
    result = result && ASSERT_EQ(0x4D, utf16.data[6]);
    result = result && ASSERT_EQ(0xDC, utf16.data[7]);
    return result;
}

BEGIN_TEST_GROUP(codec_tests)
CLEANUP(cleanup)
TEST(test_valid_utf8_encoding)
TEST(test_invalid_utf8_encoding)
TEST(test_utf8_to_utf16)
END_TEST_GROUP

