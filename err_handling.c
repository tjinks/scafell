//
//  err_handling.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include <stdlib.h>

#include "err_handling.h"

void scf_fatal_error(const char *filename, int line_number, const char *msg) {
    fprintf(stderr, "%s: Line %d - %s\n", filename, line_number, msg);
    exit(1);
}
