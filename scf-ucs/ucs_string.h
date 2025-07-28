//
//  ucs_string.h
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#ifndef ucs_string_h
#define ucs_string_h

#include "ucsdb.c"
#include "mmgt.h"
#include "codecs.h"

typedef struct {
    size_t char_count;
    scf_buffer bytes;
} ucs_string;

typedef struct {
    size_t byte_index;
    size_t char_index;
    ucs_string *s;
} ucs_iterator;

ucs_string ucs_string_create(scf_operation *op);

ucs_string ucs_string_from_bytes(scf_operation *op, const void *bytes, size_t bytecount, ucs_encoding enc);

ucs_iterator ucs_get_iterator(const ucs_string *s);

ucs_iterator ucs_get_iterator_at(const ucs_string *s, size_t char_index);

ucs_codepoint ucs_current(const ucs_iterator *iter);

void ucs_next(ucs_iterator *iter);

void ucs_prev(ucs_iterator *iter);

inline bool ucs_at_end(const ucs_iterator *iter) {
    return iter->byte_index == iter->s->bytes.size;
}

inline bool ucs_at_start(const ucs_iterator *iter) {
    return iter->byte_index == 0;
}


#endif /* ucs_string_h */
