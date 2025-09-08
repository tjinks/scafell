//
//  scf.c
//  scf
//
//  Created by Tony on 12/08/2025.
//

#include "scf.h"
#include "machine_info.h"
#include "ucs_db.h"

void scf_init(scf_err_handler err_handler) {
    scf_set_err_handler(err_handler);
    scf_initialise_machine_info();
    ucs_dbinit();
}

void scf_shutdown(void) {
    ucs_dbclose();
}

