//
//  codecs.c
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#include "codecs.h"
#include "ucsdb.h"
#include "err_handling.h"

typedef struct {
    ucs_codepoint codepoint;
    size_t bytecount;
} decoded_char;

typedef struct {
    unsigned char bytes[4];
    size_t bytecount;
} encoded_char;

typedef decoded_char (*decoder)(const unsigned char *, const unsigned char *);

typedef encoded_char (*encoder)(ucs_codepoint);

/*-----------------------------------
 * UTF8-specific logic
 *---------------------------------*/

// Declared inline - see codecs.h
extern size_t ucs_get_utf8_bytecount(unsigned char first_byte);

ucs_codepoint ucs_utf8_get(const unsigned char *current, const unsigned char *end, size_t *bytecount) {
    if (current == end) {
        *bytecount = 0;
        return UCS_INVALID;
    }
    
    unsigned char first_byte = current[0];
    *bytecount = ucs_get_utf8_bytecount(first_byte);

    if (*bytecount > end - current) {
        *bytecount = 0;
        return UCS_INVALID;
    }
    
    ucs_codepoint result;
    switch (*bytecount) {
        case 1:
            result = first_byte;
            break;
        case 2:
            result = first_byte & 0x3F;
            result = (result << 6) + (current[1] & 0x7F);
            break;
        case 3:
            result = first_byte & 0x1F;
            result = (result << 6) + (current[1] & 0x7F);
            result = (result << 6) + (current[2] & 0x7F);
            break;
        case 4:
            result = first_byte & 0xF;
            result = (result << 6) + (current[1] & 0x7F);
            result = (result << 6) + (current[2] & 0x7F);
            result = (result << 6) + (current[3] & 0x7F);
            break;
        default:
            result = UCS_INVALID;
            bytecount = 0;
            break;
    }
    
    return result;

}

/*
 * Decode a UTF8 character from a byte array, starting at the specified address.
 */
static decoded_char decode_utf8(const unsigned char *s, const unsigned char *end) {
    const unsigned char *original = s;
    decoded_char result;
    result.codepoint = ucs_utf8_get(s, end, &result.bytecount);
    return result;
}

inline static unsigned char shift_and_mask(ucs_codepoint cp, int shift, unsigned char mask) {
    return (unsigned char)((cp >> shift) & mask);
}

void ucs_utf8_put(ucs_codepoint codepoint, unsigned char *target, const unsigned char *end, size_t *bytecount) {
    if (codepoint <= 0x7F) {
        *bytecount = 1;
    } else if (codepoint > 0xFFFF) {
        if (codepoint > 0x10FFFF) {
            *bytecount = 0;
        } else {
            *bytecount = 4;
        }
    } else if (codepoint > 0x7FF) {
        *bytecount = 3;
    } else {
        *bytecount = 2;
    }
    
    if (end - target < *bytecount) *bytecount = 0;
    
    switch (*bytecount) {
        case 1:
            target[0] = (unsigned char)codepoint;
            break;
        case 2:
            target[0] = 0xC0 | shift_and_mask(codepoint, 6, 0xFF);
            target[1] = 0x80 | shift_and_mask(codepoint, 0, 0x3F);
            break;
        case 3:
            target[0] = 0xE0 | shift_and_mask(codepoint, 12, 0xFF);
            target[1] = 0x80 | shift_and_mask(codepoint, 6, 0x3F);
            target[2] = 0x80 | shift_and_mask(codepoint, 0, 0x3F);
            break;
        case 4:
            target[0] = 0xF0 | shift_and_mask(codepoint, 18, 0xFF);
            target[1] = 0x80 | shift_and_mask(codepoint, 12, 0x3F);
            target[2] = 0x80 | shift_and_mask(codepoint, 6, 0x3F);
            target[3] = 0x80 | shift_and_mask(codepoint, 0, 0x3F);
            break;
    }
}

static encoded_char encode_utf8(ucs_codepoint cp) {
    encoded_char result;
    ucs_utf8_put(cp, result.bytes, result.bytes + sizeof(result.bytes), &result.bytecount);
    return result;
}

/*-----------------------------------
 * UTF16-specific logic
 *---------------------------------*/
static const uint16_t first_high_surrogate = 0xD800;
static const uint16_t last_high_surrogate = 0xDBFF;
static const uint16_t first_low_surrogate = 0xDC00;
static const uint16_t last_low_surrogate = 0xDFFF;

static inline uint16_t get_unit(const unsigned char *bytes, bool le) {
    if (le) {
        return ((uint16_t)bytes[1] << 8) + bytes[0];
    } else {
        return ((uint16_t)bytes[0] << 8) + bytes[1];
    }
}

static inline void put_unit(uint16_t unit, unsigned char *bytes, bool le) {
    if (le) {
        bytes[0] = unit;
        bytes[1] = unit >> 8;
    } else {
        bytes[0] = unit >> 8;
        bytes[1] = unit;
    }
}

static inline bool is_high_surrogate(uint16_t unit) {
    return (unit >= first_high_surrogate) && (unit <= last_high_surrogate);
}

static inline bool is_low_surrogate(uint16_t unit) {
    return (unit >= first_low_surrogate) && (unit <= last_low_surrogate);
}

static decoded_char decode_utf16(const unsigned char *s, const unsigned char *end, bool le) {
    decoded_char result;
    if (end - s < 2)
    {
        result.bytecount = 0;
        result.codepoint = 0;
        return result;
    }
    
    uint16_t first_unit = get_unit(s, le);
    result.codepoint = first_unit;
    result.bytecount = 2;
    
    if (is_high_surrogate(first_unit)) {
        if (end - s > 2) {
            uint16_t second_unit = get_unit(s + 2, le);
            if (is_low_surrogate(second_unit)) {
                result.codepoint = ((first_unit - first_high_surrogate) << 10) + second_unit - first_low_surrogate;
                result.codepoint += 0x10000;
                result.bytecount = 4;
            }
        }
    }
    
    return result;
}

static encoded_char encode_utf16(ucs_codepoint cp, bool le) {
    encoded_char result;
    if (cp <= 0xFFFF) {
        put_unit(cp, result.bytes, le);
        result.bytecount = 2;
    } else {
        if (cp > 0x10FFFF) {
            result.bytecount = 0;
        } else {
            uint16_t first_unit = ((cp - 0x10000) >> 10) + first_high_surrogate;
            uint16_t second_unit = (cp & 0x3FF) + first_low_surrogate;
            put_unit(first_unit, result.bytes, le);
            put_unit(second_unit, result.bytes + 2, le);
            result.bytecount = 4;
        }
    }
    
    return result;
}

static decoded_char decode_utf16_le(const unsigned char *s, const unsigned char *end) {
    return decode_utf16(s, end, true);
}

static decoded_char decode_utf16_be(const unsigned char *s, const unsigned char *end) {
    return decode_utf16(s, end, false);
}

static encoded_char encode_utf16_le(ucs_codepoint cp) {
    return encode_utf16(cp, true);
}

static encoded_char encode_utf16_be(ucs_codepoint cp) {
    return encode_utf16(cp, false);
}

/*-----------------------------------
 * Logic common to all encodings
 *---------------------------------*/
static decoder get_decoder(ucs_encoding enc) {
    switch ((int)enc) {
        case UCS_UTF8:
            return decode_utf8;
        case UCS_UTF16:
        case UCS_UTF16 | UCS_LE:
            return decode_utf16_le;
        case UCS_UTF16 | UCS_BE:
            return decode_utf16_be;
        default:
            scf_raise_error(SCF_INVALID_ENCODING, "Unsupported encoding");
    }
}

static encoder get_encoder(ucs_encoding enc) {
    switch ((int)enc) {
        case UCS_UTF8:
            return encode_utf8;
        case UCS_UTF16:
        case UCS_UTF16 | UCS_LE:
            return encode_utf16_le;
        case UCS_UTF16 | UCS_BE:
            return encode_utf16_be;
        default:
            scf_raise_error(SCF_INVALID_ENCODING, "Unsupported encoding");
    }
}
                                  
size_t ucs_encode(
                scf_buffer *source,
                size_t source_offset,
                ucs_encoding source_encoding,
                scf_buffer *target,
                ucs_encoding target_encoding) {
    return ucs_encode_bytes(source->data, source->size - source_offset, source_encoding, target, target_encoding);
}

size_t ucs_encode_bytes(
                      const void *bytes,
                      size_t length,
                      ucs_encoding source_encoding,
                      scf_buffer *target,
                      ucs_encoding target_encoding) {
    decoder dec = get_decoder(source_encoding);
    encoder enc = get_encoder(target_encoding);
    size_t original_target_size = target->size;
    size_t charcount = 0;
    const unsigned char *s = bytes;
    const unsigned char *end = s + length;
    while (s != end) {
        decoded_char decoded = dec(s, end);
        if (decoded.bytecount == 0) goto encoding_failed;
        encoded_char encoded = enc(decoded.codepoint);
        if (encoded.bytecount == 0) goto encoding_failed;
        scf_buffer_append_bytes(target, encoded.bytes, encoded.bytecount);
        s += decoded.bytecount;
        charcount++;
    }
    
    return charcount;
    
encoding_failed:
    target->size = original_target_size;
    return SIZE_MAX;
}


