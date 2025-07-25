//
//  mmgt_tests.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include <stdio.h>
#include "scuts.h"
#include "ucsdb.h"

static void init(void) {
    ucs_dbinit();
}

static void cleanup(void) {
    ucs_dbclose();
}

static bool test_lookup_valid_codepoint(void) {
    ucs_details details = ucs_lookup(0xB5);
    bool result = ASSERT_EQ(0xB5, details.codepoint);
    result &= ASSERT_EQ(UCS_LETTER | UCS_LOWER, (int)details.category);
    result &= ASSERT_EQ(-1, details.digit_value);
    result &= ASSERT_EQ(0x39C, details.uc_codepoint);
    result &= ASSERT_EQ(0x39C, details.tc_codepoint);
    result &= ASSERT_EQ(0xB5, details.lc_codepoint);
    return result;
}

static bool test_lookup_invalid_codepoint(void) {
    ucs_details details = ucs_lookup(0xDBFF);
    bool result = ASSERT_EQ(0xDBFF, details.codepoint);
    result &= ASSERT_EQ(UCS_NONE, (int)details.category);
    result &= ASSERT_EQ(-1, details.digit_value);
    result &= ASSERT_EQ(0xDBFF, details.uc_codepoint);
    result &= ASSERT_EQ(0xDBFF, details.tc_codepoint);
    result &= ASSERT_EQ(0xDBFF, details.lc_codepoint);
    return result;
}

BEGIN_TEST_GROUP(ucsdb_tests)
    INIT(init)
    CLEANUP(cleanup)
    TEST(test_lookup_valid_codepoint)
    TEST(test_lookup_invalid_codepoint)
//    TEST(test_realloc)
END_TEST_GROUP



