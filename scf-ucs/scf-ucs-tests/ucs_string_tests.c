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

static SCF_OPERATION(op);

static const char data[] = {0x41, 0xEF, 0xBF, 0xBD, 0xF0, 0x90,0x8E, 0xBD, 0xC2, 0xA3, 0};
static size_t datalen;

static void init(void) {
    datalen = strlen(data);
}

static void cleanup(void) {
    scf_complete(&op);
}

static bool test_string_from_bytes(void) {
    ucs_string s = ucs_string_from_bytes(&op, data, datalen, UCS_UTF8);
    return ASSERT_EQ(4, s.char_count);
}

static bool test_iterator(void) {
    ucs_string s = ucs_string_from_bytes(&op, data, datalen, UCS_UTF8);
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

BEGIN_TEST_GROUP(ucs_string_tests)
INIT(init)
CLEANUP(cleanup)
TEST(test_string_from_bytes)
TEST(test_iterator)
END_TEST_GROUP

