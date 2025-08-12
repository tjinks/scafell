//
//  system_info.h
//  scf-core
//
//  Created by Tony on 12/08/2025.
//

#ifndef system_info_h
#define system_info_h

typedef enum {
    SCF_BIG_ENDIAN,
    SCF_LITTLE_ENDIAN
} scf_endianness;

typedef struct {
    scf_endianness endianness;
} scf_machine_info;

void scf_initialise_machine_info(void);

scf_machine_info scf_get_machine_info(void);

#endif /* system_info_h */
