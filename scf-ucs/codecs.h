//
//  codecs.h
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#ifndef codecs_h
#define codecs_h

#include <stdbool.h>
#include <stdint.h>

#include "mmgt.h"
#include "ucs_db.h"

typedef enum {
    UCS_OPAQUE = 0
    ,UCS_UTF8 = 1
    ,UCS_UTF16 = 2
    ,UCS_UTF32 = 3
    ,UCS_WINDOWS_1252 = 4
    ,UCS_BOM = 0x1000
    ,UCS_LE = 0x2000
    ,UCS_BE = 0x4000
    ,UCS_ENCODING_MASK = UCS_BOM - 1
} ucs_encoding;

ucs_codepoint ucs_utf8_to_codepoint(ucs_utf8_char ch);

ucs_utf8_char ucs_codepoint_to_utf8(ucs_codepoint cp);

ucs_utf8_char ucs_utf8_get(const scf_buffer *buf, size_t *index);

bool ucs_utf8_append(scf_buffer *buf, ucs_utf8_char ch);


/*------------------------------------------------------------
 * Converts a string of bytes from one encoding to another.
 * The converted bytes are appended to 'target'.
 *
 * If successful, returns the number of characters converted.
 * Otherwise returns SIZE_MAX. (In this case the contents
 * of the target buffer are left unchanged, though the
 * capacity may have increased).
 -----------------------------------------------------------*/
size_t ucs_encode(
                  const scf_buffer *source,
                  size_t offset,
                  ucs_encoding source_encoding,
                  scf_buffer *target,
                  ucs_encoding target_encoding);

inline size_t ucs_get_utf8_bytecount(unsigned char first_byte) {
    if (first_byte <= 0x7F) return 1;
    
    unsigned char disc = (first_byte >> 3) & 0x1F;
    switch (disc) {
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
            return 2;
        case 0x1C:
        case 0x1D:
            return 3;
        case 0x1E:
            return 4;
        default:
            return 0;
    }
}

#endif /* codecs_h */
