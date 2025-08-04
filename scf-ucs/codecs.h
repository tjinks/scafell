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
#include "ucsdb.h"

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

ucs_codepoint ucs_utf8_get(const unsigned char *current, const unsigned char *end, size_t *bytecount);

void ucs_utf8_put(ucs_codepoint codepoint, unsigned char *target, const unsigned char *end, size_t *bytecount);


/*------------------------------------------------------------
 * Converts a string of bytes from one encoding to another.
 * The converted bytes are appended to 'target'.
 *
 * If successful, returns the number of characters converted.
 * Otherwise returns SIZE_MAX. (In this case the contents
 * of the target buffer are left unchanged, though the
 * capacity may have increased).
 -----------------------------------------------------------*/
size_t ucs_encode_bytes(
                const void *bytes,
                size_t length,
                ucs_encoding source_encoding,
                scf_buffer *target,
                ucs_encoding target_encoding);

size_t ucs_encode(
                scf_buffer *source,
                size_t source_offset,
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
