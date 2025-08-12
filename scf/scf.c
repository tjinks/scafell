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
    scf_determine_endianness();
    ucs_dbinit();
}

void scf_shutdown(void) {
    ucs_dbclose();
}

scf_endianness scf_get_endianness(void) {
    return endianness;
}

