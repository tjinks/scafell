//
//  ucs_string.c
//  scf-ucs
//
//  Created by Tony on 25/07/2025.
//

#include <string.h>
#include <wchar.h>

#include "ucs_string.h"
#include "codecs.h"
#include "err_handling.h"
#include "machine_info.h"

static inline void check_valid(const ucs_string* s) {
	if (!ucs_is_valid(s)) scf_raise_error(SCF_LOGIC_ERROR, "Specified string is not a valid UTF8 encoding");
}

static inline void add_terminator(ucs_string* s) {
	scf_buffer_append_byte(&s->bytes, 0);
}

static inline void remove_terminator(ucs_string* s) {
	s->bytes.size--;
}

static inline scf_operation* get_operation(const ucs_string* s) {
	return scf_get_operation(s->bytes.data);
}

ucs_string ucs_string_create(scf_operation* op) {
	ucs_string result = { 0, scf_buffer_create(op, 0) };
	add_terminator(&result);
	return result;
}

ucs_string ucs_from_bytes(scf_operation* op, const void* bytes, size_t bytecount, ucs_encoding enc) {
	ucs_string result;
	scf_buffer wrapped_bytes = scf_buffer_wrap((void*)bytes, bytecount);
	result.bytes = scf_buffer_create(op, bytecount);
	result.char_count = ucs_encode(&wrapped_bytes, 0, enc, &result.bytes, UCS_UTF8);
	add_terminator(&result);
	return result;
}

ucs_string ucs_from_cstr_with_encoding(scf_operation* op, const char* s, ucs_encoding enc) {
	return ucs_from_bytes(op, s, strlen(s), enc);
}

ucs_string ucs_from_cstr(scf_operation* op, const char* s) {
    return ucs_from_cstr_with_encoding(op, s, UCS_UTF8);
}

ucs_string ucs_from_wstr_with_encoding(scf_operation* op, const wchar_t* s, ucs_encoding enc) {
    size_t bytecount = sizeof(wchar_t) * wcslen(s);
	return ucs_from_bytes(op, s, bytecount, enc);
}

ucs_string ucs_from_wstr(scf_operation* op, const wchar_t* s) {
    ucs_encoding enc;
    
#ifdef _MSC_VER
    enc = UCS_UTF16;
#else
    enc = UCS_UTF32;
#endif
    
    switch (scf_get_machine_info().endianness) {
        case SCF_BIG_ENDIAN:
            enc |= UCS_BE;
            break;
        case SCF_LITTLE_ENDIAN:
            enc |= UCS_LE;
            break;
    }
        
    return ucs_from_wstr_with_encoding(op, s, enc);
}

ucs_string ucs_string_copy(scf_operation* op, const ucs_string* s) {
	check_valid(s);
	ucs_string result = *s;
	size_t bytecount = s->bytes.size;
	result.bytes.capacity = bytecount;
	result.bytes.data = scf_alloc(op, bytecount);
	result.bytes.pinned = false;
	memcpy(result.bytes.data, s->bytes.data, bytecount);
	return result;
}

void ucs_append(ucs_string* s1, const ucs_string* s2) {
	check_valid(s1);
	check_valid(s2);
    remove_terminator(s1);
	scf_buffer_append(&s1->bytes, &s2->bytes);
	s1->char_count += s2->char_count;
}

void ucs_append_char(ucs_string *s, ucs_utf8_char ch) {
    check_valid(s);
    remove_terminator(s);
    ucs_utf8_append(&s->bytes, ch);
    add_terminator(s);
}

ucs_string ucs_substring(const ucs_iterator* from, size_t length) {
	ucs_string result = ucs_string_create(get_operation(from->s));
    remove_terminator(&result);
    result.char_count = length;
    ucs_iterator current = *from;
    
	for (size_t i = 0; i < length; i++) {
		ucs_utf8_char ch;
		if (ucs_next(&current, &ch)) {
            ucs_utf8_append(&result.bytes, ch);
		} else {
            result.char_count = i;
			break;
		}
	}

    add_terminator(&result);
	return result;
}

int ucs_compare(const ucs_string *s1, const ucs_string *s2) {
    check_valid(s1);
    check_valid(s2);
    int result = strcmp((const char *)s1->bytes.data, (const char *)s2->bytes.data);
    return result;
}

ucs_iterator ucs_get_iterator(ucs_string* s) {
	check_valid(s);

	ucs_iterator result;
	result.s = s;
	result.byte_index = 0;
	result.char_index = 0;
	return result;
}

ucs_iterator ucs_get_iterator_at(ucs_string* s, size_t char_index) {
	ucs_iterator result = ucs_get_iterator(s);
	while (result.char_index < char_index && result.char_index <= s->char_count) {
		ucs_utf8_get(&s->bytes, &result.byte_index);
		result.char_index++;
	}

	return result;
}

bool ucs_next(ucs_iterator* iter, ucs_utf8_char* ch) {
	if (iter->char_index < iter->s->char_count) {
		*ch = ucs_utf8_get(&iter->s->bytes, &iter->byte_index);
	} else {
		*ch = UCS_INVALID;
	}

	if (*ch == UCS_INVALID) return false;

	iter->char_index++;
	return true;
}

bool ucs_prev(ucs_iterator *iter, ucs_utf8_char *ch) {
    if (iter->byte_index == 0) {
        *ch = UCS_INVALID;
        return false;
    }
    
    iter->byte_index--;
    const unsigned char *ptr = iter->s->bytes.data + iter->byte_index;
    *ch = *ptr;
    if (*ptr <= 0x7F) {
        return true;
    }
    
    int shift = 8;
    while (true) {
        ptr--;
        iter->byte_index--;
        ucs_utf8_char nextbyte = *ptr;
        *ch |= (nextbyte << shift);
        if ((nextbyte & 0xC0) != 0x80) {
            return true;
        }
        
        shift += 8;
    }
}

/*----------------------------------------------
 * extern declarations for inline functions
 ---------------------------------------------*/

extern bool ucs_at_end(const ucs_iterator* iter);

extern bool ucs_at_start(const ucs_iterator* iter);

extern bool ucs_is_valid(const ucs_string* s);

