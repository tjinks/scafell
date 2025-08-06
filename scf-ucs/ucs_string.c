//
//  ucs_string.c
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#include <string.h>

#include "ucs_string.h"
#include "err_handling.h"

static void check_valid(const ucs_string *s) {
    if (!ucs_is_valid(s)) scf_raise_error(SCF_LOGIC_ERROR, "Specified string is not a valid UTF* encoding");
}

static inline void add_terminator(ucs_string *s) {
    scf_buffer_append_byte(&s->bytes, 0);
}

static inline void remove_terminator(ucs_string *s) {
    s->bytes.size--;
}

ucs_string ucs_string_create(scf_operation *op) {
    ucs_string result = {0, scf_buffer_create(op, 0)};
    add_terminator(&result);
    return result;
}

ucs_string ucs_string_from_bytes(scf_operation *op, const void *bytes, size_t bytecount, ucs_encoding enc) {
    ucs_string result;
    scf_buffer wrapped_bytes = scf_buffer_wrap((void *)bytes, bytecount);
    result.bytes = scf_buffer_create(op, bytecount);
    result.char_count = ucs_encode(&wrapped_bytes, 0, enc, &result.bytes, UCS_UTF8);
    add_terminator(&result);
    return result;
}

ucs_iterator ucs_get_iterator(ucs_string *s) {
    check_valid(s);
    
    ucs_iterator result;
    result.s = s;
    result.byte_index = 0;
    result.char_index = 0;
    return result;
}

ucs_iterator ucs_get_iterator_at(ucs_string *s, size_t char_index) {
    ucs_iterator result = ucs_get_iterator(s);
    while (result.char_index < char_index && result.char_index < s->char_count) {
        result.byte_index += ucs_get_utf8_bytecount(s->bytes.data[result.byte_index]);
        result.char_index++;
    }
    
    return result;
}

bool ucs_next(ucs_iterator *iter, ucs_codepoint *codepoint) {
#if XXX
    scf_buffer bytebuffer = iter->s->bytes;
    if (iter->byte_index >= bytebuffer.size) {
        *codepoint = UCS_INVALID;
        return false;
    }
    
    size_t bytecount;
    *codepoint = ucs_utf8_get(&bytebuffer.data + iter->byte_index, bytebuffer.data + bytebuffer.size, &bytecount);
    if (*codepoint == UCS_INVALID) return false;
    
    iter->byte_index += bytecount;
    iter->char_index++;
#endif
    return true;
}

/*----------------------------------------------
 * extern declarations for inline functions
 ---------------------------------------------*/

extern bool ucs_at_end(const ucs_iterator *iter);

extern bool ucs_at_start(const ucs_iterator *iter);

extern bool ucs_is_valid(const ucs_string *s);

