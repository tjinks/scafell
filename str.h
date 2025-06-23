//
//  utf8.h
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#ifndef utf8_h
#define utf8_h

#include "os/osdefs.h"

typedef int utf8_char;

typedef struct {
    int size, capacity;
    unsigned char data;
} scf_string;

extern const utf8_char SCF_INVALID_CODEPOINT;

utf8_char utf8_from_codepoint(scf_codepoint cp);



#endif /* utf8_h */
