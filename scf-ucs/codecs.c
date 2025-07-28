//
//  codecs.c
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#include "codecs.h"
#include "ucsdb.h"
#include "err_handling.h"

typedef decoded_char (*decoder)(const unsigned char *, const unsigned char *);

typedef encoded_char (*encoder)(ucs_codepoint);

/*-----------------------------------
 * UTF8-specific logic
 *---------------------------------*/

inline static size_t get_utf8_bytecount(unsigned char first_byte) {
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

/*
 * Decode a UTF8 character from a byte array, starting at the specified address.
 *
 * It is assumed that the 's' is within the bounds of the array, but is not
 * necessarily the first character of a valid UTF8 sequence.
 */
static decoded_char decode_utf8(const unsigned char *s, const unsigned char *end) {
    unsigned char first_byte = s[0];
    size_t bytecount = get_utf8_bytecount(first_byte);

    decoded_char result;
    if (bytecount > end - s) {
        result.bytecount = 0;
        result.codepoint = UCS_INVALID;
    } else {
        ucs_codepoint codepoint = 0;
        
        switch (bytecount) {
            case 0:
                codepoint = UCS_INVALID;
                break;
            case 1:
                codepoint = first_byte;
                break;
            case 2:
                codepoint = first_byte & 0x3F;
                codepoint = (codepoint << 6) + (s[1] & 0x7F);
                break;
            case 3:
                codepoint = first_byte & 0x1F;
                codepoint = (codepoint << 6) + (s[1] & 0x7F);
                codepoint = (codepoint << 6) + (s[2] & 0x7F);
                break;
            case 4:
                codepoint = first_byte & 0xF;
                codepoint = (codepoint << 6) + (s[1] & 0x7F);
                codepoint = (codepoint << 6) + (s[2] & 0x7F);
                codepoint = (codepoint << 6) + (s[3] & 0x7F);
                break;
        }
        
        result.codepoint = codepoint;
        result.bytecount = bytecount;
    }
    
    return result;
}

inline static unsigned char shift_and_mask(ucs_codepoint cp, int shift, unsigned char mask) {
    return (unsigned char)((cp >> shift) & mask);
}

static encoded_char encode_utf8(ucs_codepoint cp) {
    encoded_char result;
    if (cp <= 0x7F) {
        result.bytecount = 1;
        result.bytes[0] = (unsigned char)cp;
    } else if (cp > 0xFFFF) {
        if (cp > 0x10FFFF) {
            result.bytecount = 0;
        } else {
            result.bytecount = 4;
            result.bytes[0] = 0xF0 | shift_and_mask(cp, 18, 0xFF);
            result.bytes[1] = 0x80 | shift_and_mask(cp, 12, 0x3F);
            result.bytes[2] = 0x80 | shift_and_mask(cp, 6, 0x3F);
            result.bytes[3] = 0x80 | shift_and_mask(cp, 0, 0x3F);
        }
    } else if (cp > 0x7FF) {
        result.bytecount = 3;
        result.bytes[0] = 0xE0 | shift_and_mask(cp, 12, 0xFF);
        result.bytes[1] = 0x80 | shift_and_mask(cp, 6, 0x3F);
        result.bytes[2] = 0x80 | shift_and_mask(cp, 0, 0x3F);
    } else {
        result.bytecount = 2;
        result.bytes[0] = 0xC0 | shift_and_mask(cp, 6, 0xFF);
        result.bytes[1] = 0x80 | shift_and_mask(cp, 0, 0x3F);
    }

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
        bytes[0] = unit >> 8;
        bytes[1] = unit;
    } else {
        bytes[0] = unit;
        bytes[1] = unit >> 8;
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
            int16_t first_unit = (cp >> 10) + first_high_surrogate;
            int16_t second_unit = (cp & 0x3F) + first_low_surrogate;
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
 * Logic covering all encodings
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
                                  
bool ucs_encode(
                scf_buffer *source,
                size_t source_offset,
                ucs_encoding source_encoding,
                scf_buffer *target,
                ucs_encoding target_encoding) {
    return ucs_encode_bytes(source->data, source->size - source_offset, source_encoding, target, target_encoding);
}

bool ucs_encode_bytes(
                      const void *bytes,
                      size_t length,
                      ucs_encoding source_encoding,
                      scf_buffer *target,
                      ucs_encoding target_encoding) {
    decoder dec = get_decoder(source_encoding);
    encoder enc = get_encoder(target_encoding);
    const unsigned char *s = bytes;
    const unsigned char *end = bytes + length;
    while (s != end) {
        decoded_char decoded = dec(s, end);
        if (decoded.bytecount == 0) {
            return false;
        }
        
        encoded_char encoded = enc(decoded.codepoint);
        if (encoded.bytecount == 0) {
            return false;
        }
        
        scf_buffer_append_bytes(target, encoded.bytes, encoded.bytecount);
        s += decoded.bytecount;
    }
    
    return true;
    
}


