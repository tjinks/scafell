//
//  scf.c
//  scafell
//
//  Created by Tony on 04/10/2026.
//

#include "scf.h"
#include "err_handling.h"
#include "mmgt.h"

static SCF_OPERATION(global_op);

void scf_init(void) {

}

void scf_shutdown(void) {
    scf_complete(&global_op);
}
