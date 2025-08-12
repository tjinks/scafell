//
//  system_info.c
//  scf-core
//
//  Created by Tony on 12/08/2025.
//

#include <stdint.h>
#include "err_handling.h"
#include "machine_info.h"

static scf_machine_info machine_info;

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

void scf_initialise_machine_info(void) {
    machine_info.endianness = determine_endianness();
}

scf_machine_info scf_get_machine_info(void) {
    return machine_info;
}

