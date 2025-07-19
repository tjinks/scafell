//
//  utf8.c
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#include "str.h"
#include "datum.h"
#include "os/osdefs.h"
#include "err_handling.h"

const utf8_char UTF8_INVALID = -1;

static void check_is_utf8(const scf_string *s) {
    if (s->is_utf8 == false) {
        scf_raise_error(UTF8_INVALID, "Specified string is not UTF8 encoded");
    }
}

static inline size_t get_utf8_byte_count(unsigned char first_byte) {
    unsigned char disc = first_byte & 0xF8;
    if (disc < 0x7F) return 1;
    if (disc >= 0xC0 && disc < 0xE0) return 2;
    if (disc >= 0xE0 && disc < 0xF0) return 3;
    if (disc >= 0xF0 && disc < 0xF8) return 4;
    
    return 0;
}

static bool is_valid_utf8_char(const scf_string *s, size_t *offset) {
    unsigned char *ch = s->chars.data + *offset;
    size_t byte_count = get_utf8_byte_count(*ch);
    if (byte_count == 0 || *offset + byte_count > s->chars.size) {
        return false;
    }
    
    switch (byte_count) {
        case 4:
            ch++;
            if ((*ch & 0xC0) != 0x80) return false;
        case 3:
            ch++;
            if ((*ch & 0xC0) != 0x80) return false;
        case 2:
            ch++;
            if ((*ch & 0xC0) != 0x80) return false;
        case 1:
            break;
        default:
            return false;
    }
    
    *offset += byte_count;
    return true;
}

utf8_char utf8_from_codepoint(wchar_t cp) {
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
    
    return UTF8_INVALID;
}

scf_string scf_string_create(scf_operation *op) {
    scf_string result = {true, 0, scf_buffer_create(op, 0)};
    return result;
}

scf_string scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count) {
    scf_string result = {true, 0, scf_buffer_create(op, byte_count)};
    scf_buffer_append_bytes(&result.chars, p, byte_count);
    size_t offset = 0;
    while (offset < result.chars.size) {
        if (!is_valid_utf8_char(&result, &offset)) {
            result.is_utf8 = false;
            break;
        }
        
        result.char_count++;
    }
    
    return result;
}

void scf_string_append(scf_string *s1, const scf_string *s2) {
    scf_buffer_append(&s1->chars, &s2->chars);
    if (s1->is_utf8) {
        if (!s2->is_utf8) {
            s1->is_utf8 = false;
        } else {
            s1->char_count += s2->char_count;
        }
    }
}

scf_string scf_substring(utf8_iterator iter, size_t length) {
    const scf_string *s = iter.s;
    scf_operation *op = scf_get_operation(s->chars.data);
    size_t start = iter.char_index;
    if (start + length > s->char_count) {
        length = s->char_count - start;
    }
    
    scf_string result = scf_string_create(op);
    if (length == 0) {
        return result;
    }
        
    for (int i = 0; i < length && !utf8_is_at_end(iter); i++) {
        utf8_char c = utf8_current(iter);
        scf_string_append_char(&result, c);
        utf8_next(&iter);
    }
    
    return result;
}

void scf_string_append_char(scf_string *s, utf8_char c) {
    unsigned char bytes[4] = {0, 0, 0, 0};
    size_t byte_count = get_utf8_byte_count((unsigned char) c);
    switch (byte_count) {
        case 4:
            bytes[3] = (unsigned char) (c >> 24);
        case 3:
            bytes[2] = (unsigned char) (c >> 16);
        case 2:
            bytes[1] = (unsigned char) (c >> 8);
        case 1:
            bytes[0] = (unsigned char) c;
            break;
        default:
            scf_raise_error(SCF_INVALID_UTF8_OPERATION, "Invalid UTF8 character passed to scf_string_append_char");
    }
    
    scf_buffer_append_bytes(&s->chars, bytes, byte_count);
    s->char_count++;
}

char *scf_string_to_cstr(const scf_string *s) {
    char *result = scf_alloc(scf_get_operation(s->chars.data), s->chars.size + 1);
    memcpy(result, s->chars.data, s->chars.size);
    result[s->chars.size] = 0;
    return result;
}

bool utf8_next(utf8_iterator *iter) {
    size_t new_byte_index = iter->byte_index + get_utf8_byte_count(iter->s->chars.data[iter->byte_index]);
    iter->byte_index = new_byte_index;
    iter->char_index++;
    return (new_byte_index < iter->s->chars.size);
}

bool utf8_prev(utf8_iterator *iter) {
    size_t new_byte_index = iter->byte_index;
    for (;;) {
        if (new_byte_index == 0) return false;
        new_byte_index--;
        unsigned char ch = iter->s->chars.data[new_byte_index];
        if ((ch & 0xC) != 0x80) break;
    }
    
    iter->byte_index = new_byte_index;
    iter->char_index--;
    return true;
}

utf8_char utf8_current(utf8_iterator iter) {
    if (utf8_is_at_end(iter)) scf_raise_error(SCF_INVALID_UTF8_OPERATION, "Iterator points beyond end of string");
    unsigned char *current_char = iter.s->chars.data + iter.byte_index;
    utf8_char result = 0;
    size_t byte_count = get_utf8_byte_count(current_char[0]);
    switch (byte_count) {
        case 4:
            result += (current_char[3] << 24);
        case 3:
            result += (current_char[2] << 16);
        case 2:
            result += (current_char[1] << 8);
        case 1:
            result += current_char[0];
            break;
        default:
            scf_raise_error(SCF_INVALID_UTF8_OPERATION, "Found invalid UTF8 byte sequence in string");
    }
    
    return result;
}

utf8_iterator utf8_iterator_at(const scf_string *s, int char_index) {
    check_is_utf8(s);
    
    utf8_iterator result;
    result.s = s;
    if (char_index >= s->char_count) {
        result.byte_index = s->chars.size;
        result.char_index = s->char_count;
        return result;
    } else {
        result.byte_index = 0;
        result.char_index = 0;
        while (result.char_index != char_index) {
            if (!utf8_next(&result)) break;
        }
        
        return result;
    }
}



// extern defs for inline functions
extern scf_string scf_string_from_cstr(scf_operation *op, const char *cstr);
extern size_t scf_string_byte_count(const scf_string *s);
extern utf8_iterator scf_string_iterator(const scf_string *s);
extern bool scf_is_valid_utf8(const scf_string *s);
extern bool utf8_is_at_end(utf8_iterator iter);
extern bool utf8_is_at_start(utf8_iterator iter);




