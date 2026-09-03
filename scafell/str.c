//
//  utf8.c
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#include "str.h"
#include "ucdb.h"
#include "os/osdefs.h"
#include "err_handling.h"

static noreturn void invalid_encoding(void) {
    scf_raise_error(SCF_INVALID_STRING_OPERATION, "Unsupported character encoding");
}

static scf_char utf8_char_from_codepoint(scf_codepoint cp) {
    scf_char result = {SCF_UTF8, 0};
    int index = 0;
    
    if (cp <= 0x7F) {
        result.bytes[index++] = cp;
        result.byte_count = index;
    } else if (cp <= 0x7FF) {
        result.bytes[index++] = 0x80 + (cp & 0x3F);
        result.bytes[index++] = 0xC0 + (cp >> 6);
        result.byte_count = index;
    } else if (cp <= 0xFFFF) {
        result.bytes[index++] = 0x80 + (cp & 0x3F);
        result.bytes[index++] = 0x80 + ((cp >> 6) & 0x3F);
        result.bytes[index++] = 0xE0 + (cp >> 12);
        result.byte_count = index;
    } else if (cp <= 0x10FFFF) {
        result.bytes[index++] = 0x80 + (cp & 0x3F);
        result.bytes[index++] = 0x80 + ((cp >> 6) & 0x3F);
        result.bytes[index++] = 0x80 + ((cp >> 12) & 0x3F);
        result.bytes[index++] = 0xF0 + (cp >> 18);
        result.byte_count = index;
    }
    
    return result;
}

static scf_char utf16_char_from_codepoint(scf_codepoint cp, scf_encoding enc) {
    scf_char result = {enc, 2};
    if (enc == SCF_UTF16_BE) {
        result.bytes[0] = cp >> 8;
        result.bytes[1] = cp;
    } else {
        result.bytes[0] = cp;
        result.bytes[2] = cp >> 8;
    }
    
    return result;
}

static scf_codepoint codepoint_from_utf8_char(scf_char ch) {
    scf_codepoint result;
    switch (ch.byte_count) {
        case 1:
            result = ch.bytes[0];
            break;
        case 2:
            result = ch.bytes[0] & 0x1F;
            result <<= 6;
            result += ch.bytes[1] & 0x3F;
            break;
        case 3:
            result = ch.bytes[0] & 0xF;
            result <<= 6;
            result += ch.bytes[1] & 0x3F;
            result <<= 6;
            result += ch.bytes[2] & 0x3F;
            break;
        case 4:
            result = ch.bytes[0] & 0x7;
            result <<= 6;
            result += ch.bytes[1] & 0x3F;
            result <<= 6;
            result += ch.bytes[2] & 0x3F;
            result <<= 6;
            result += ch.bytes[3] & 0x3F;
            break;
        default:
            scf_raise_error(SCF_INVALID_STRING_OPERATION, "Invalid byte count in utf8 character");
    }
    
    return result;
}

static scf_codepoint codepoint_from_utf16_char(scf_char ch) {
    if (ch.byte_count != 2) {
        scf_raise_error(SCF_INVALID_STRING_OPERATION, "Invalid byte count in utf16 character");
    }
    
    scf_codepoint bytes[] = {ch.bytes[0], ch.bytes[1]};
    if (ch.encoding == SCF_UTF16_BE) {
        return (bytes[0] << 8) + bytes[1];
    } else {
        return (bytes[1] << 8) + bytes[0];
    }
}

static int get_byte_count(const scf_string *s, int offset) {
    int result = -1;
    if (offset >= s->buf.size) goto invalid_offset;
    
    switch (s->encoding) {
        case SCF_UTF8: {
            unsigned char disc = s->buf.data[offset] & 0xF8;
            if (disc < 0x7F) {
                result = 1;
                break;
            }
            
            if (disc >= 0xC0 && disc < 0xE0) {
                result = 2;
                break;
            }
            
            if (disc >= 0xE0 && disc < 0xF0) {
                result = 3;
                break;
            }
            
            if (disc >= 0xF0 && disc < 0xF8) {
                result = 4;
                break;
            }
            
            break;
        }
            
        case SCF_UTF16_BE:
        case SCF_UTF16_LE:
            result = 2;
            break;
        default:
            invalid_encoding();
    }
    
    if (offset + result - 1 >= s->buf.size) goto invalid_offset;
    return result;
    
invalid_offset:
    return -1;
}

scf_char scf_char_from_codepoint(scf_codepoint cp, scf_encoding enc) {
    switch (enc) {
        case SCF_UTF8:
            return utf8_char_from_codepoint(cp);
        case SCF_UTF16_BE:
        case SCF_UTF16_LE:
            return utf16_char_from_codepoint(cp, enc);
        default:
            invalid_encoding();
    }
}

scf_codepoint scf_codepoint_from_char(scf_char ch) {
    switch (ch.encoding) {
        case SCF_UTF8:
            return codepoint_from_utf8_char(ch);
        case SCF_UTF16_BE:
        case SCF_UTF16_LE:
            return codepoint_from_utf16_char(ch);
        default:
            invalid_encoding();
    }
}

scf_string *scf_string_with_encoding(scf_operation *op, scf_encoding encoding) {
    scf_string *result = scf_alloc(op, sizeof(scf_string));
    result->encoding = encoding;
    result->buf = scf_buffer_create(op, 16);
    return result;
}

void scf_string_append_char(scf_string *s, scf_char c) {
    if (s->encoding == c.encoding) {
        scf_buffer_append_bytes(&s->buf, c.bytes, c.byte_count);
    } else {
        scf_codepoint cp = scf_codepoint_from_char(c);
        scf_char converted = scf_char_from_codepoint(cp, s->encoding);
        scf_buffer_append_bytes(&s->buf, converted.bytes, converted.byte_count);
    }
    
    s->char_count++;
}

scf_string *scf_string_convert(const scf_string *s, scf_encoding target_encoding) {
    scf_operation *op = scf_get_operation(s->buf.data);
    scf_string *result = scf_string_with_encoding(op, target_encoding);
    scf_string_append(result, s);
    return result;
}

bool scf_string_next(scf_string_iterator *iter, scf_char *c) {
    scf_char result;
    result.encoding = iter->s->encoding;
    int byte_count = get_byte_count(iter->s, iter->index);
    if (byte_count == -1) {
        return false;
    }
    
    if (c) {
        c->encoding = iter->s->encoding;
        c->byte_count = byte_count;
        memcpy(c->bytes, iter->s + iter->index, byte_count);
    }

    iter->index += byte_count;
    return true;
}


void scf_string_append(scf_string *s1, const scf_string *s2) {
    if (s1->encoding == s2->encoding) {
        scf_buffer_append(&s1->buf, &s2->buf);
        s1->char_count += s2->char_count;
    } else {
        scf_string_iterator iter = scf_string_start(s2);
        scf_char ch;
        while (scf_string_next(&iter, &ch)) {
            scf_string_append_char(s1, ch);
            s1->char_count++;
        }
    }
}

scf_string *scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count, scf_encoding encoding) {
    scf_string *result = scf_alloc(op, sizeof(scf_string));
    result->encoding = encoding;
    result->buf = scf_buffer_create(op, byte_count);
    scf_buffer_append_bytes(&result->buf, p, byte_count);
    scf_string_iterator iter = scf_string_start(result);
    result->char_count = 0;
    while (scf_string_next(&iter, NULL)) {
        result->char_count++;
    }
    
    return result;
}


char *scf_string_to_cstr(const scf_string *s) {
    const scf_string *utf8;
    scf_string *converted = NULL;
    if (s->encoding == SCF_UTF8) {
        utf8 = s;
    } else {
        converted = scf_string_convert(s, SCF_UTF8);
        utf8 = converted;
    }
    
    scf_operation *op = scf_get_operation(s->buf.data);
    size_t size = utf8->buf.size;
    char *result = scf_alloc(op, size + 1);
    memcpy(result, s->buf.data, size);
    result[size] = '\0';
    if (converted) {
        scf_string_free(converted);
    }
    
    return result;
}

scf_string *scf_substring(const scf_string_iterator *start, int char_count) {
    const scf_string *s = start->s;
    scf_operation *op = scf_get_operation(s->buf.data);
    scf_string *result = scf_string_with_encoding(op, s->encoding);
    scf_string_iterator iter = *start;
    for (int i = 0; i < char_count; i++) {
        scf_char ch;
        if (!scf_string_next(&iter, &ch)) {
            break;
        }
        
        scf_string_append_char(result, ch);
    }
    
    return result;
}



// extern defs for inline functions
extern void scf_string_append_char(scf_string *s, scf_char c);
extern scf_string *scf_utf8_string(scf_operation *op);
extern void scf_string_free(scf_string *s);




