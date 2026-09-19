//
//  err_handling_tests.c
//  ScafellTest
//
//  Created by Tony on 16/09/2026.
//

#include <stdio.h>
#include "scuts.h"
#include "err_handling.h"

static int error_raising_func(bool raise_error, scf_err_context *ec) {
    if (raise_error) {
        scf_raise_error(scf_err_info_create(SCF_LOGIC_ERROR, "It's all gone wrong!"), ec);
    }
    
    return 42;
}

bool test_try(void) {
    int n = 0;
    bool result = true;
    SCF_ERR_INFO(err_info);
    
    SCF_TRY(ec) {
        n = error_raising_func(false, &ec);
    }
    SCF_CATCH {
        err_info = ec.err_info;
        n = 99;
    }
    SCF_END_TRY

    result = result && ASSERT_EQ(42, n);
    result = result && ASSERT_EQ(0, err_info.code);
    
    n = 0;
    SCF_TRY(ec) {
        n = error_raising_func(true, &ec);
    }
    SCF_CATCH {
        err_info = ec.err_info;
        n = 99;
    }
    SCF_END_TRY

    result = result && ASSERT_EQ(99, n);
    result = result && ASSERT_EQ(SCF_LOGIC_ERROR, err_info.code);
    return result;
}

BEGIN_TEST_GROUP(err_handling_tests)
    TEST(test_try)
END_TEST_GROUP
