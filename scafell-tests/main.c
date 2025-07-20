//
//  main.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include <stdio.h>
#include "scuts.h"


int main(int argc, const char * argv[]) {
    
    REGISTER(mmgt_tests);
    REGISTER(list_tests);
    REGISTER(hash_tests);
    REGISTER(string_tests);
    REGISTER(buffer_tests);
    return scuts(argc, argv);
}
