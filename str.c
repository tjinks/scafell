//
//  utf8.c
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#include "str.h"
#include "datum.h"
#include "os/osdefs.h"

const utf8_char UTF8_INVALID = -1;

static size_t get_utf8_byte_count(unsigned char first_byte) {
    if (first_byte <= 0x7F) {
        return 1;
    }
    
    if ((first_byte & 0xE0) == 0xC0) {
        return 2;
    }
    
    if ((first_byte & 0xF0) == 0xE0) {
        return 3;
    }
    
    if ((first_byte & 0xF8) == 0xF0) {
        return 4;
    }
    
    return -1;
}

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
    
    return UTF8_INVALID;
}

scf_string scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count) {
    scf_string result = {0, scf_buffer_create(op, byte_count)};
    scf_buffer_append_bytes(&result.chars, p, byte_count);
    size_t byte_index = 0;
    for (;;) {
        size_t bytes_in_char = get_utf8_byte_count(result.chars.data[byte_index]);
        if (bytes_in_char == -1 || byte_index + bytes_in_char > byte_count) {
            result.char_count = -1;
            break;
        }
        
        result.char_count++;
        byte_index += bytes_in_char;
        if (byte_index == byte_count) {
            break;
        }
    }
    
    return result;
}



// extern defs for inline functions
extern scf_string scf_string_from_cstr(scf_operation *op, const char *cstr);
extern size_t scf_string_byte_count(const scf_string *s);
extern utf8_iterator scf_string_iterator(const scf_string *s);



