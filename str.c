//
//  utf8.c
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#include "str.h"
#include "datum.h"
#include "os/osdefs.h"

const utf8_char SCF_INVALID_CODEPOINT = -1;

utf8_char utf8_from_codepoint(scf_codepoint cp) {
    if (cp <= 0x7F) {
        return cp;
    }
    
    if (cp <= 0x7FF) {
        int byte1 = 0x80 + (cp & 0x3F);
        int byte2 = 0xC0 + (cp >> 6);
        return byte2 + (byte1 << 8);
    }
    
    if (cp <= 0xFFFF) {
        int byte1 = 0x80 + (cp & 0x3F);
        int byte2 = 0x80 + ((cp >> 6) & 0x3F);
        int byte3 = 0xE0 + (cp >> 12);
        return byte3 + (byte2 << 8) + (byte1 << 16);
    }
    
    if (cp <= 0x10FFFF) {
        int byte1 = 0x80 + (cp & 0x3F);
        int byte2 = 0x80 + ((cp >> 6) & 0x3F);
        int byte3 = 0x80 + ((cp >> 12) & 0x3F);
        int byte4 = 0xF0 + (cp >> 18);
        return byte4 + (byte3 << 8) + (byte2 << 16) + (byte1 << 24);
    }
    
    return SCF_INVALID_CODEPOINT;
}

