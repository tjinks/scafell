//
//  codecs.h
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#ifndef codecs_h
#define codecs_h

#include <stdbool.h>

#include "mmgt.h"

typedef enum {
    UCS_UTF8 = 0
    ,UCS_UTF16 = 1
    ,UCS_UTF32= 2
    ,UCS_WINDOWS_1252 = 3
    ,UCS_BOM = 0x1000
    ,UCS_LE = 0x2000
    ,UCS_BE = 0x4000
} ucs_encoding;

typedef int32_t ucs_codepoint;

typedef struct {
    ucs_codepoint codepoint;
    size_t bytecount;
} decoded_char;

typedef struct {
    unsigned char bytes[4];
    size_t bytecount;
} encoded_char;


bool ucs_encode_bytes(
                const void *bytes,
                size_t length,
                ucs_encoding source_encoding,
                scf_buffer *target,
                ucs_encoding target_encoding);

bool ucs_encode(
                scf_buffer *source,
                size_t source_offset,
                ucs_encoding source_encoding,
                scf_buffer *target,
                ucs_encoding target_encoding);


#endif /* codecs_h */
