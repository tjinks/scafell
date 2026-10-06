//
//  scf.c
//  scafell
//
//  Created by Tony on 04/10/2026.
//

#include "scf.h"
#include "scf_internal.h"
#include "err_handling.h"

static SCF_OPERATION(global_op);

void scf_init(void) {
    scf_file_io_init(&global_op);
}

void scf_shutdown(void) {
    scf_complete(&global_op);
}
