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

#include "mmgt.h"
#include "err_handling.h"
#include "ucdb.h"
#include "list.h"

typedef enum {
    SCF_UTF8,
    SCF_UTF16_BE,
    SCF_UTF16_LE
} scf_encoding;

typedef struct {
    scf_encoding encoding;
    int byte_count;
    unsigned char bytes[4];
} scf_char;

typedef struct {
    int char_count;
    scf_buffer buf;
} scf_string;

typedef struct {
    const scf_string *s;
    size_t index;
} scf_string_iterator;

typedef struct {
    scf_list strings;
} scf_stringlist;

scf_char scf_char_from_codepoint(scf_codepoint cp, scf_encoding enc, scf_err_context *);

int scf_char_cmp(scf_char ch1, scf_char ch2);

scf_codepoint scf_codepoint_from_char(scf_char ch);

inline scf_char scf_convert_char(scf_char ch, scf_encoding target_encoding) {
    if (ch.encoding == target_encoding) {
        return ch;
    }
    
    scf_codepoint cp = scf_codepoint_from_char(ch);
    return scf_char_from_codepoint(cp, target_encoding, NULL);
}

scf_char scf_char_to_lower(scf_char ch);

scf_char scf_char_to_upper(scf_char ch);

int scf_char_to_int(scf_char ch);

inline scf_char scf_ascii(char c) {
    if ((c & 0x80) != 0) scf_raise_fatal_error(SCF_INVALID_STRING_OPERATION, "Non-ASCII character passed to scf_ascii() function");
    scf_char result = {SCF_UTF8, 1, (unsigned char)c};
    return result;
}

scf_string *scf_string_create(scf_operation *op);

scf_string *scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count, scf_encoding enc, scf_err_context *);

inline scf_string *scf_string_from_cstr(scf_operation *op, const char *cstr) {
    return scf_string_from_bytes(op, cstr, strlen(cstr), SCF_UTF8, NULL);
}

int scf_string_cmp(const scf_string *s1, const scf_string *s2);

scf_buffer scf_string_to_bytes(const scf_string *s, scf_encoding target_encoding);

void scf_string_append(scf_string *s1, const scf_string *s2);

void scf_string_append_cstr(scf_string *s, const char *cstr);

void scf_string_append_ascii(scf_string *s, char ascii);

void scf_string_append_char(scf_string *s, scf_char c);

char *scf_string_to_cstr(const scf_string *s);

scf_string *scf_substring(scf_string_iterator start, int char_count);

bool scf_string_next(scf_string_iterator *iter, scf_char *c);

bool scf_string_prev(scf_string_iterator *iter, scf_char *c);

scf_string_iterator scf_string_iterator_at(const scf_string *s, int index);

inline scf_string_iterator scf_string_start(const scf_string *s) {
    scf_string_iterator result = {s, 0};
    return result;
}

inline scf_string_iterator scf_string_end(const scf_string *s) {
    scf_string_iterator result = {s, s->buf.size};
    return result;
}

inline bool scf_is_string_start(scf_string_iterator iter) {
    return iter.index == 0;
}

inline bool scf_is_string_end(scf_string_iterator iter) {
    return iter.index == iter.s->buf.size;
}

scf_string *scf_string_clone(scf_operation *op, const scf_string *s);

bool scf_string_has_prefix(scf_string *s, scf_string *prefix);

bool scf_string_has_suffix(scf_string *s, scf_string *suffix);

inline void scf_string_free(scf_string *s) {
    scf_buffer_free(&s->buf);
    scf_free(s);
}

scf_stringlist *scf_stringlist_create(scf_operation *op);

void scf_stringlist_add(scf_stringlist *list, const scf_string *s);

scf_string *scf_stringlist_get(const scf_stringlist *list, size_t index);

scf_string *scf_stringlist_get_copy(scf_operation *op, const scf_stringlist *list, size_t index);

void scf_stringlist_clear(scf_stringlist *list);

void scf_stringlist_insert(scf_stringlist *list, const scf_string *s, size_t before);

scf_string *scf_stringlist_remove(scf_stringlist *list, size_t index);

scf_string *scf_stringlist_combine(scf_stringlist *list, scf_string *separator);

void scf_stringlist_sort(scf_stringlist *list, scf_comparison_func cmp);

inline void scf_stringlist_push(scf_stringlist *list, const scf_string *s) {
    scf_stringlist_add(list, s);
}

inline scf_string *scf_stringlist_pop(scf_stringlist *list) {
    return scf_stringlist_remove(list, list->strings.size - 1);
}

inline void scf_stringlist_add_cstr(scf_stringlist *list, const char *cstr) {
    scf_stringlist_add(list, scf_string_from_cstr(scf_get_operation(list), cstr));
}

inline size_t scf_stringlist_size(const scf_stringlist *list) {
    return list->strings.size;
}

#endif /* utf8_h */
