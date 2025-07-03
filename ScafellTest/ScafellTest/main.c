//
//  main.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include <stdio.h>
#include <scuts.h>

int main(int argc, const char * argv[]) {
    RUN_TEST_GROUP(mmgt_tests)
    RUN_TEST_GROUP(list_tests)
    RUN_TEST_GROUP(hash_tests)
    RUN_TEST_GROUP(string_tests)
    RUN_TEST_GROUP(buffer_tests)
    printf("%s\n", __FUNCTION__);
    return failedCount != 0;
}
