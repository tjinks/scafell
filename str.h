//
//  utf8.h
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#ifndef utf8_h
#define utf8_h

#include <string.h>
#include "os/osdefs.h"
#include "mmgt.h"

typedef int utf8_char;

typedef struct {
    size_t char_count;
    scf_buffer chars;
} scf_string;

typedef struct {
    const scf_string *s;
    size_t byte_index;
    size_t char_index;
} utf8_iterator;

extern const utf8_char UTF8_INVALID;

utf8_char utf8_from_codepoint(scf_codepoint cp);

scf_string scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count);

inline scf_string scf_string_from_cstr(scf_operation *op, const char *cstr) {
    return scf_string_from_bytes(op, cstr, strlen(cstr));
}

inline size_t scf_string_byte_count(const scf_string *s) {
    return s->chars.size;
}

inline utf8_iterator scf_string_iterator(const scf_string *s) {
    utf8_iterator result = {s, s->char_count == -1 ? scf_string_byte_count(s) : 0, 0};
    return result;
}

#endif /* utf8_h */
