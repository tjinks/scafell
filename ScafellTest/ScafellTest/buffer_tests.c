//
//  buffer_tests.c
//  ScafellTest
//
//  Created by Tony on 26/06/2025.
//

#include <stdio.h>
#include <scuts.h>
#include <string.h>
#include "mmgt.h"

static SCF_OPERATION(op);

BEGIN_TEST_GROUP(buffer_tests)
    INIT(buffer_tests_init)
    CLEANUP(buffer_tests_cleanup)
    TEST(test_buffer_append_bytes)
    TEST(test_buffer_insert_bytes)
    TEST(test_buffer_remove)
    TEST(test_buffer_extract)
END_TEST_GROUP

void buffer_tests_init(void) {
    
}

void buffer_tests_cleanup(void) {
    scf_complete(&op);
}

bool test_buffer_append_bytes(void) {
    scf_buffer buf = scf_buffer_create(&op, 1);
    bool result = ASSERT_EQ(4, buf.capacity);
    scf_buffer_append_bytes(&buf, "abc", 3);
    scf_buffer_append_bytes(&buf, "123", 4);
    result &= ASSERT_EQ(8, buf.capacity);
    result &= ASSERT_EQ(7, buf.size);
    result &= ASSERT_EQ(0, strcmp("abc123", (char *)buf.data));
    return result;
}

bool test_buffer_insert_bytes(void) {
    scf_buffer buf = scf_buffer_create(&op, 1);
    scf_buffer_append_bytes(&buf, "abc", 3);
    scf_buffer_insert_bytes(&buf, "", 2, 0);
    bool result = true;
    result &= ASSERT_EQ(3, buf.size);
    result &= ASSERT_EQ(0, memcmp("abc123", (char *)buf.data, buf.size));
    
    scf_buffer_insert_bytes(&buf, "12", 3, 2);
    result &= ASSERT_EQ(5, buf.size);
    result &= ASSERT_EQ(0, memcmp("abc12", (char *)buf.data, buf.size));
    
    scf_buffer_insert_bytes(&buf, "3456", 3, 4);
    result &= ASSERT_EQ(9, buf.size);
    result &= ASSERT_EQ(0, memcmp("abc345612", (char *)buf.data, buf.size));
    return result;
}

bool test_buffer_remove(void) {
    scf_buffer buf = scf_buffer_create(&op, 1);
    scf_buffer_append_bytes(&buf, "123456", 6);
    scf_buffer_remove(&buf, 2, 3);
    bool result = true;
    result &= ASSERT_EQ(3, buf.size);
    result &= ASSERT_EQ(0, memcmp("126", (char *)buf.data, buf.size));
    return result;
}

bool test_buffer_extract(void) {
    scf_buffer buf = scf_buffer_create(&op, 1);
    scf_buffer_append_bytes(&buf, "123456", 6);
    scf_buffer buf2 = scf_buffer_extract(&buf, 2, 0);
    bool result = true;
    result &= ASSERT_EQ(0, buf2.size);

    buf2 = scf_buffer_extract(&buf, 1, 3);
    result &= ASSERT_EQ(3, buf2.size);
    result &= ASSERT_EQ(0, memcmp("234", (char *)buf2.data, buf2.size));
    return result;
}

