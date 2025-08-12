//
//  scf.c
//  scf
//
//  Created by Tony on 12/08/2025.
//

#include "scf.h"
#include "ucs_db.h"

static scf_endianness endianness;

static scf_endianness determine_endianness(void) {
    static const uint32_t testvalue = 0x12345678;
    static const char *test_bytes = (const char *)&testvalue;
    switch (((uint32_t)test_bytes[0] << 24) + ((uint32_t)test_bytes[1] << 16) + ((uint32_t)test_bytes[2] << 8) + test_bytes[3]) {
        case 0x12345678:
            return SCF_BIG_ENDIAN;
        case 0x78563412:
            return SCF_LITTLE_ENDIAN;
        default:
            scf_raise_error(SCF_INTERNAL_ERROR, "Cannot determine endianness of current system");
    }
}

void scf_init(scf_err_handler err_handler) {
    scf_set_err_handler(err_handler);
    endianness = determine_endianness();
    ucs_dbinit();
}

void scf_shutdown(void) {
    ucs_dbclose();
}

scf_endianness scf_get_endianness(void) {
    return endianness;
}

