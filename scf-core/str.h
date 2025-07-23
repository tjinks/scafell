//
//  utf8.h
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#ifndef utf8_h
#define utf8_h

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "osdefs.h"
#include "mmgt.h"
#include "err_handling.h"

/*
 * SCF_SHORT_WCHAR is defined on systems (Windows) where wchar_t is too small to hold all unicode codepoints.
 */
#if WCHAR_MAX < 0x10FFFF
#define SCF_SHORT_WCHAR
#endif

typedef int32_t utf8_char;

typedef struct {
    bool is_utf8;
    size_t char_count;
    scf_buffer chars;
} scf_string;

typedef struct {
    const scf_string *s;
    size_t byte_index;
    size_t char_index;
} utf8_iterator;

extern const utf8_char UTF8_INVALID;

utf8_char utf8_from_codepoint(int32_t cp);

scf_string scf_string_create(scf_operation *op);

scf_string scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count);

void scf_string_append(scf_string *s1, const scf_string *s2);

void scf_string_append_char(scf_string *s, utf8_char c);

char *scf_string_to_cstr(const scf_string *s);

scf_string scf_substring(const utf8_iterator iter, size_t length);

bool utf8_next(utf8_iterator *iter);

bool utf8_prev(utf8_iterator *iter);

utf8_char utf8_current(utf8_iterator iter);

scf_string scf_string_from_cstr(scf_operation *op, const char *cstr);

scf_string scf_string_from_wstr(scf_operation *op, const wchar_t *wstr, bool *is_valid);

/*
 * Returns the number of bytes required to store the string, excluding the terminating zero byte.
 */
inline size_t scf_string_byte_count(const scf_string *s) {
    return s->chars.size - 1;
}

utf8_iterator utf8_iterator_at(const scf_string *s, int char_index);

inline bool utf8_is_at_end(utf8_iterator iter) {
    return iter.char_index == iter.s->char_count;
}

inline bool utf8_is_at_start(utf8_iterator iter) {
    return iter.char_index == 0;
}

inline utf8_iterator scf_string_iterator(const scf_string *s) {
    return utf8_iterator_at(s, 0);
}

#endif /* utf8_h */
