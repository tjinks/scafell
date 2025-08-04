//
//  mmgt_tests.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include "scuts.h"
#include "ucsdb.h"

static void init(void) {
    ucs_dbinit();
}

static void cleanup(void) {
    ucs_dbclose();
}

static bool test_lookup_valid_codepoint(void) {
    ucs_details details = ucs_lookup(0xC2B5);
    bool result = ASSERT_EQ(0xB5, details.codepoint);
    result &= ASSERT_EQ(0xC2B5, details.utf8);
    result &= ASSERT_EQ(UCS_LETTER | UCS_LOWER, (int)details.category);
    result &= ASSERT_EQ(-1, details.digit_value);
    result &= ASSERT_EQ(0xCE9C, details.uc_utf8);
    result &= ASSERT_EQ(0xCE9C, details.tc_utf8);
    result &= ASSERT_EQ(0xC2B5, details.lc_utf8);
    return result;
}

static bool test_lookup_invalid_codepoint(void) {
    ucs_details details = ucs_lookup(0xDBFF);
    bool result = ASSERT_EQ(UCS_INVALID, details.codepoint);
    result &= ASSERT_EQ(UCS_NONE, (int)details.category);
    result &= ASSERT_EQ(-1, details.digit_value);
    result &= ASSERT_EQ(UCS_INVALID, details.uc_utf8);
    result &= ASSERT_EQ(UCS_INVALID, details.tc_utf8);
    result &= ASSERT_EQ(UCS_INVALID, details.lc_utf8);
    result &= ASSERT_EQ(UCS_INVALID, details.utf8);
    return result;
}

BEGIN_TEST_GROUP(ucsdb_tests)
    INIT(init)
    CLEANUP(cleanup)
    TEST(test_lookup_valid_codepoint)
    TEST(test_lookup_invalid_codepoint)
END_TEST_GROUP



