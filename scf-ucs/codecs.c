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

typedef ucs_utf8_char (*extractor)(const scf_buffer *, size_t *index);

typedef bool (*appender)(scf_buffer *, ucs_utf8_char);

/*-----------------------------------
 * UTF8-specific logic
 *---------------------------------*/

// Declared inline - see codecs.h
extern size_t ucs_get_utf8_bytecount(unsigned char first_byte);

ucs_codepoint ucs_utf8_to_codepoint(ucs_utf8_char ch) {
    ucs_codepoint result;
    if (ch < 0xFF) {
        result = ch;
    } else if (ch < 0xFFFF) {
        result = ((ch & 0x1F00) >> 2) | (ch & 0x3F);
    } else if (ch <= 0xFFFFFF) {
        result = ((ch & 0xF0000) >> 4) | ((ch & 0x3F00) >> 2) | (ch & 0x3F);
    } else {
        result = ((ch & 0x7000000) >> 6) | ((ch & 0x3F0000) >> 4) | ((ch & 0x3F00) >> 2) | (ch & 0x3F);
    }
    
    return result;
}

ucs_utf8_char ucs_codepoint_to_utf8(ucs_codepoint cp) {
    if (cp <= 0x7F) {
        return cp;
    }
    
    ucs_utf8_char result = cp & 0x3F;
    if (cp <= 0x7FF) {
        result |= ((cp & 0x7C0) << 2);
        result |= 0xC080;
    } else if (cp <= 0xFFFF) {
        result |= ((cp & 0xFC0) << 2);
        result |= ((cp & 0xF000) << 4);
        result |= 0xE08080;
    } else {
        result |= ((cp & 0xFC0) << 2);
        result |= ((cp & 0x3F000) << 4);
        result |= ((cp & 0x1C0000) << 6);
        result |= 0xF0808080;
    }
    
    return result;
}

ucs_utf8_char ucs_utf8_get(const scf_buffer *buf, size_t *index) {
    if (*index >= buf->size) {
        return UCS_INVALID;
    }
    
    unsigned const char *current = buf->data + *index;
    unsigned char first_byte = current[0];
    size_t bytecount = ucs_get_utf8_bytecount(first_byte);

    if (*index + bytecount > buf->size) {
        bytecount = 0;
        return UCS_INVALID;
    }
    
    ucs_utf8_char result = 0;
    switch (bytecount) {
        case 4:
            result += ((ucs_utf8_char)current[0] << 24);
            current++;
        case 3:
            result += ((ucs_utf8_char)current[0] << 16);
            current++;
        case 2:
            result += ((ucs_utf8_char)current[0] << 8);
            current++;
        case 1:
            result += (ucs_utf8_char)current[0];
            *index += bytecount;
            break;
        default:
            result = UCS_INVALID;
            bytecount = 0;
            break;
    }
    
    return result;

}

bool ucs_utf8_append(scf_buffer *buf, ucs_utf8_char ch) {
    size_t bytecount;
    if (ch < 0xFF) {
        bytecount = 1;
    } else if (ch < 0xFFFF) {
        bytecount = 2;
    } else if (ch < 0xFFFFFF) {
        bytecount = 3;
    } else {
        bytecount = 4;
    }
    
    unsigned char data[4];
    switch (bytecount) {
        case 4:
            data[0] = ch >> 24;
        case 3:
            data[1] = ch >> 16;
        case 2:
            data[2] = ch >> 8;
        case 1:
            data[3] = ch;
            scf_buffer_append_bytes(buf, &data[4 - bytecount], bytecount);
            break;
        default:
            return false;
    }
    
    return true;
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
static extractor get_extractor(ucs_encoding enc) {
    switch ((int)enc) {
        case UCS_UTF8:
            return ucs_utf8_get;
            /*
        case UCS_UTF16:
        case UCS_UTF16 | UCS_LE:
            return decode_utf16_le;
        case UCS_UTF16 | UCS_BE:
            return decode_utf16_be;
             */
        default:
            scf_raise_error(SCF_INVALID_ENCODING, "Unsupported encoding");
    }
}

static appender get_appender(ucs_encoding enc) {
    switch ((int)enc) {
        case UCS_UTF8:
            return ucs_utf8_append;
            /*
        case UCS_UTF16:
        case UCS_UTF16 | UCS_LE:
            return encode_utf16_le;
        case UCS_UTF16 | UCS_BE:
            return encode_utf16_be;
             */
        default:
            scf_raise_error(SCF_INVALID_ENCODING, "Unsupported encoding");
    }
}
   
/*
size_t ucs_encode(
                scf_buffer *source,
                size_t source_offset,
                ucs_encoding source_encoding,
                scf_buffer *target,
                ucs_encoding target_encoding) {
    return ucs_encode_bytes(source->data, source->size - source_offset, source_encoding, target, target_encoding);
}
*/

size_t ucs_encode(
                  const scf_buffer *source,
                  size_t offset,
                  ucs_encoding source_encoding,
                  scf_buffer *target,
                  ucs_encoding target_encoding) {
    extractor extract = get_extractor(source_encoding);
    appender append = get_appender(target_encoding);
    size_t original_target_size = target->size;
    size_t charcount = 0;
    size_t index = offset;
    while (index < source->size) {
        ucs_utf8_char ch = extract(source, &index);
        if (ch == UCS_INVALID) goto encoding_failed;
        if (!append(target, ch)) goto encoding_failed;
        charcount++;
    }
    
    return charcount;
    
encoding_failed:
    target->size = original_target_size;
    return SIZE_MAX;
}


