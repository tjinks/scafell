//
//  codecs.c
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#include "codecs.h"
#include "ucs_db.h"
#include "err_handling.h"

typedef ucs_utf8_char (*extractor)(const scf_buffer *, size_t *index);

typedef bool (*appender)(scf_buffer *, ucs_utf8_char);

/*-----------------------------------
 * UTF8-specific logic
 *---------------------------------*/

// Declared inline - see codecs.h
//extern size_t ucs_get_utf8_bytecount(unsigned char first_byte);

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
    if (first_byte <= 0x7F) {
        *index += 1;
        return first_byte;
    }
    
    unsigned char disc = (first_byte >> 3) & 0x1F;
    ucs_utf8_char result = UCS_INVALID;
    
    switch (disc) {
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
            if (*index + 2 <= buf->size) {
                result = ((ucs_utf8_char)current[0] << 8) | current[1];
                *index += 2;
            }
            
            break;
        case 0x1C:
        case 0x1D:
            if (*index + 3 <= buf->size) {
                result = ((ucs_utf8_char)current[0] << 16) | ((ucs_utf8_char)current[1] << 8) | current[2];
                *index += 3;
            }
            
            break;
        case 0x1E:
            if (*index + 4 <= buf->size) {
                result = ((ucs_utf8_char)current[0] << 24);
                result |= ((ucs_utf8_char)current[1] << 16);
                result |= ((ucs_utf8_char)current[2] << 8);
                result |= current[3];
                
                *index += 4;
            }
            
            break;
        default:
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
 * UTF32-specific logic
 *---------------------------------*/

static ucs_codepoint utf32_get_codepoint(const scf_buffer *buf, size_t *index, bool le) {
    ucs_codepoint result;
    if (*index + 4 <= buf->size) {
        const unsigned char *current = buf->data + *index;
        *index += 4;
        if (le) {
            result = current[0];
            result |= ((ucs_codepoint)current[1] << 8);
            result |= ((ucs_codepoint)current[2] << 16);
            result |= ((ucs_codepoint)current[3] << 24);
        } else {
            result = current[3];
            result |= ((ucs_codepoint)current[2] << 8);
            result |= ((ucs_codepoint)current[1] << 16);
            result |= ((ucs_codepoint)current[0] << 24);
        }
    } else {
        result = UCS_INVALID;
    }
    
    return result;
}

static bool utf32_append_codepoint(scf_buffer *buf, ucs_codepoint cp, bool le) {
    unsigned char bytes[4];
    if (le) {
        bytes[0] = cp;
        bytes[1] = cp >> 8;
        bytes[2] = cp >> 16;
        bytes[3] = cp >> 24;
    } else {
        bytes[3] = cp;
        bytes[2] = cp >> 8;
        bytes[1] = cp >> 16;
        bytes[0] = cp >> 24;
    }
    
    scf_buffer_append_bytes(buf, bytes, 4);
    return true;
}

static ucs_utf8_char utf32le_get(const scf_buffer *buf, size_t *index) {
    ucs_codepoint cp = utf32_get_codepoint(buf, index, true);
    return cp == UCS_INVALID ? cp : ucs_codepoint_to_utf8(cp);
}

static ucs_utf8_char utf32be_get(const scf_buffer *buf, size_t *index) {
    ucs_codepoint cp = utf32_get_codepoint(buf, index, false);
    return cp == UCS_INVALID ? cp : ucs_codepoint_to_utf8(cp);
}

static bool utf32le_append(scf_buffer *buf, ucs_utf8_char ch) {
    ucs_codepoint cp = ucs_utf8_to_codepoint(ch);
    return utf32_append_codepoint(buf, cp, true);
}

static bool utf32be_append(scf_buffer *buf, ucs_utf8_char ch) {
    ucs_codepoint cp = ucs_utf8_to_codepoint(ch);
    return utf32_append_codepoint(buf, cp, false);
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

static ucs_codepoint utf16_get_codepoint(const scf_buffer *buf, size_t *index, bool le) {
    ucs_codepoint result = UCS_INVALID;
    if (*index + 2 <= buf->size)
    {
        uint16_t first_unit = get_unit(buf->data + *index, le);
        result = first_unit;
        *index += 2;
        
        if (is_high_surrogate(first_unit)) {
            if (*index + 2 <= buf->size) {
                uint16_t second_unit = get_unit(buf->data + *index, le);
                if (is_low_surrogate(second_unit)) {
                    result = ((first_unit - first_high_surrogate) << 10) + second_unit - first_low_surrogate;
                    result += 0x10000;
                    *index += 2;
                }
            }
        }
    }
    
    return result;
}

static ucs_utf8_char utf16le_get(const scf_buffer *buf, size_t *index) {
    ucs_codepoint cp = utf16_get_codepoint(buf, index, true);
    return cp == UCS_INVALID ? cp : ucs_codepoint_to_utf8(cp);
}

static ucs_utf8_char utf16be_get(const scf_buffer *buf, size_t *index) {
    ucs_codepoint cp = utf16_get_codepoint(buf, index, false);
    return cp == UCS_INVALID ? cp : ucs_codepoint_to_utf8(cp);
}

static bool utf16_append_codepoint(scf_buffer *buf, ucs_codepoint cp, bool le) {
    unsigned char bytes[4];
    size_t bytecount = 0;
    if (cp <= 0xFFFF) {
        put_unit(cp, bytes, le);
        bytecount = 2;
    } else {
        if (cp <= 0x10FFFF) {
            uint16_t first_unit = ((cp - 0x10000) >> 10) + first_high_surrogate;
            uint16_t second_unit = (cp & 0x3FF) + first_low_surrogate;
            put_unit(first_unit, bytes, le);
            put_unit(second_unit, bytes + 2, le);
            bytecount = 4;
        }
    }
    
    scf_buffer_append_bytes(buf, bytes, bytecount);
    return bytecount != 0;
}

static bool utf16le_append(scf_buffer *buf, ucs_utf8_char ch) {
    ucs_codepoint cp = ucs_utf8_to_codepoint(ch);
    return utf16_append_codepoint(buf, cp, true);
}

static bool utf16be_append(scf_buffer *buf, ucs_utf8_char ch) {
    ucs_codepoint cp = ucs_utf8_to_codepoint(ch);
    return utf16_append_codepoint(buf, cp, false);
}

/*-----------------------------------
 * Logic common to all encodings
 *---------------------------------*/
static extractor get_extractor(ucs_encoding enc) {
    switch ((int)enc) {
        case UCS_UTF8:
            return ucs_utf8_get;
        case UCS_UTF16:
        case UCS_UTF16 | UCS_LE:
            return utf16le_get;
        case UCS_UTF16 | UCS_BE:
            return utf16be_get;
        case UCS_UTF32:
        case UCS_UTF32 | UCS_LE:
            return utf32le_get;
        case UCS_UTF32 | UCS_BE:
            return utf32be_get;
        default:
            scf_raise_error(SCF_INVALID_ENCODING, "Unsupported encoding");
    }
}

static appender get_appender(ucs_encoding enc) {
    switch ((int)enc) {
        case UCS_UTF8:
            return ucs_utf8_append;
        case UCS_UTF16:
        case UCS_UTF16 | UCS_LE:
            return utf16le_append;
        case UCS_UTF16 | UCS_BE:
            return utf16be_append;
        case UCS_UTF32:
        case UCS_UTF32 | UCS_LE:
            return utf32le_append;
        case UCS_UTF32 | UCS_BE:
            return utf32be_append;
        default:
            scf_raise_error(SCF_INVALID_ENCODING, "Unsupported encoding");
    }
}
   
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


