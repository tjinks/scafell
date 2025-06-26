//
//  err_handling.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include <stdlib.h>

#include "err_handling.h"

void scf_fatal_error(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}
