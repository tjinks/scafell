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

#include "os/osdefs.h"
#include "mmgt.h"
#include "err_handling.h"

typedef enum {
    SCF_UTF8,
    SCF_UTF16_BE,
    SCF_UTF16_LE,
    SCF_UTF32
} scf_encoding;

typedef struct {
    scf_encoding encoding;
    int byte_count;
    char bytes[4];
} scf_char;

typedef struct {
    scf_encoding encoding;
    int char_count;
    scf_buffer buf;
} scf_string;

typedef struct {
    const scf_string *s;
    int index;
} scf_string_iterator;

scf_string *scf_string_with_encoding(scf_operation *op, scf_encoding encoding);

inline scf_string *scf_utf8_string(scf_operation *op) {
    return scf_string_with_encoding(op, SCF_UTF8);
}

scf_string *scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count, scf_encoding encoding);

scf_string *scf_string_convert(const scf_string *s, scf_encoding target_encoding);

void scf_string_append(scf_string *s1, const scf_string *s2);

void scf_string_append_char(scf_string *s, scf_char c);

char *scf_string_to_cstr(const scf_string *s);

scf_string *scf_substring(const scf_string_iterator *start, int char_count);

bool scf_string_next(scf_string_iterator *iter, scf_char *c);

scf_string_iterator scf_string_iterator_at(const scf_string *s, int index);

inline scf_string_iterator scf_string_start(const scf_string *s) {
    return scf_string_iterator_at(s, 0);
}

inline void scf_string_free(scf_string *s) {
    scf_buffer_free(&s->buf);
}

#endif /* utf8_h */
