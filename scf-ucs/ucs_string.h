//
//  ucs_string.h
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#ifndef ucs_string_h
#define ucs_string_h

#include "ucs_db.c"
#include "mmgt.h"
#include "codecs.h"

/*-------------------------------------------------
 * Holds a UTF8 encoded string.
 ------------------------------------------------*/
typedef struct {
    /*
     * The number of UTF8 characters in the string.
     *
     * If an attempt is made to create a string from an invalid
     * encoding then this is set to SIZE_MAX.
     */
    size_t char_count;
    
    /*
     * A buffer containing the bytes of the string.
     *
     * charcount == SIZE_MAX then the contents of this buffer are not defined.
     */
    scf_buffer bytes;
} ucs_string;

typedef struct {
    size_t byte_index;
    size_t char_index;
    ucs_string *s;
} ucs_iterator;

ucs_string ucs_string_create(scf_operation *op);

ucs_string ucs_string_from_bytes(scf_operation *op, const void *bytes, size_t bytecount, ucs_encoding enc);

ucs_iterator ucs_get_iterator(ucs_string *s);

ucs_iterator ucs_get_iterator_at(ucs_string *s, size_t char_index);

bool ucs_next(ucs_iterator *iter, ucs_codepoint *codepoint);

inline bool ucs_at_end(const ucs_iterator *iter) {
    return iter->byte_index == iter->s->bytes.size;
}

inline bool ucs_at_start(const ucs_iterator *iter) {
    return iter->byte_index == 0;
}

inline bool ucs_is_valid(const ucs_string *s) {
    return s->char_count != SIZE_MAX;
}



#endif /* ucs_string_h */
