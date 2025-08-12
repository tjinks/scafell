//
//  scf.h
//  scf
//
//  Created by Tony on 12/08/2025.
//

#ifndef scf_h
#define scf_h

#include "err_handling.h"

typedef enum {
    SCF_BIG_ENDIAN,
    SCF_LITTLE_ENDIAN
} scf_endianness;

scf_endianness scf_get_endianness(void);

void scf_init(scf_err_handler err_handler);

void scf_shutdown(void);

#endif /* scf_h */
