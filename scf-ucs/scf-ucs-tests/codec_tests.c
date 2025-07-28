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

BEGIN_TEST_GROUP(codec_tests)
CLEANUP(cleanup)
TEST(test_valid_utf8_encoding)
TEST(test_invalid_utf8_encoding)
END_TEST_GROUP

