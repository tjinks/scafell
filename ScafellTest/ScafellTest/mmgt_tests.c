//
//  mmgt_tests.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include <stdio.h>
#include "scuts.h"
#include "mmgt.h"

static void *alloc1;
static void *alloc2;
static void *alloc3;
static int alloc1_cleanup_count;
static int alloc2_cleanup_count;
static int alloc3_cleanup_count;

static void cleanup(void *p) {
    if (p == alloc1) {
        alloc1_cleanup_count++;
    }
    
    if (p == alloc2) {
        alloc2_cleanup_count++;
    }
    
    if (p == alloc3) {
        alloc3_cleanup_count++;
    }
}

void mmgt_init(void) {
    alloc1 = NULL;
    alloc2 = NULL;
    alloc3 = NULL;
    alloc1_cleanup_count = 0;
    alloc2_cleanup_count = 0;
    alloc3_cleanup_count = 0;
}

bool test_alloc_and_free(void) {
    SCF_OPERATION(op);
    alloc1 = scf_alloc_with_cleanup(&op, cleanup, 10);
    alloc2 = scf_alloc_with_cleanup(&op, cleanup, 20);
    scf_complete(&op);
    
    return
    ASSERT_EQ(1, alloc1_cleanup_count)
    && ASSERT_EQ(1, alloc2_cleanup_count)
    && ASSERT_EQ(0, alloc3_cleanup_count);
}

bool test_realloc(void) {
    SCF_OPERATION(op);
    alloc1 = scf_alloc_with_cleanup(&op, cleanup, 10);
    alloc2 = scf_alloc_with_cleanup(&op, cleanup, 20);
    alloc3 = scf_realloc(alloc1, 30);
    scf_complete(&op);
    
    return
    ASSERT_EQ(0, alloc1_cleanup_count)
    && ASSERT_EQ(1, alloc2_cleanup_count)
    && ASSERT_EQ(1, alloc3_cleanup_count);
}

BEGIN_TEST_GROUP(mmgt_tests)
    INIT(mmgt_init)
    TEST(test_alloc_and_free)
    TEST(test_realloc)
END_TEST_GROUP



