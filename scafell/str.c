//
//  utf8.c
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#include <memory.h>
#include "str.h"
#include "ucdb.h"
#include "os/osdefs.h"
#include "err_handling.h"

#define CMP(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const int INITIAL_STRING_SIZE = 16;
static const int INITIAL_STRINGLIST_SIZE = 16;

static noreturn void invalid_encoding(void) {
    scf_raise_error(SCF_INVALID_STRING_OPERATION, "Unsupported character encoding");
}

static scf_char utf8_char_from_codepoint(scf_codepoint cp) {
    scf_char result = {SCF_UTF8, 0};
    int index = 0;
    
    if (cp <= 0x7F) {
        result.bytes[index++] = cp;
    } else if (cp <= 0x7FF) {
        result.bytes[index++] = 0xC0 + (cp >> 6);
        result.bytes[index++] = 0x80 + (cp & 0x3F);
    } else if (cp <= 0xFFFF) {
        result.bytes[index++] = 0xE0 + (cp >> 12);
        result.bytes[index++] = 0x80 + ((cp >> 6) & 0x3F);
        result.bytes[index++] = 0x80 + (cp & 0x3F);
    } else if (cp <= 0x10FFFF) {
        result.bytes[index++] = 0xF0 + (cp >> 18);
        result.bytes[index++] = 0x80 + ((cp >> 12) & 0x3F);
        result.bytes[index++] = 0x80 + ((cp >> 6) & 0x3F);
        result.bytes[index++] = 0x80 + (cp & 0x3F);
    }
    
    result.byte_count = index;
    return result;
}

static scf_char utf16_char_from_codepoint(scf_codepoint cp, scf_encoding enc) {
    scf_char result = {enc, 2};
    if (enc == SCF_UTF16_BE) {
        result.bytes[0] = cp >> 8;
        result.bytes[1] = cp;
    } else {
        result.bytes[0] = cp;
        result.bytes[2] = cp >> 8;
    }
    
    return result;
}

static scf_codepoint codepoint_from_utf8_char(scf_char ch) {
    scf_codepoint result;
    switch (ch.byte_count) {
        case 1:
            result = ch.bytes[0];
            break;
        case 2:
            result = ch.bytes[0] & 0x1F;
            result <<= 6;
            result += ch.bytes[1] & 0x3F;
            break;
        case 3:
            result = ch.bytes[0] & 0xF;
            result <<= 6;
            result += ch.bytes[1] & 0x3F;
            result <<= 6;
            result += ch.bytes[2] & 0x3F;
            break;
        case 4:
            result = ch.bytes[0] & 0x7;
            result <<= 6;
            result += ch.bytes[1] & 0x3F;
            result <<= 6;
            result += ch.bytes[2] & 0x3F;
            result <<= 6;
            result += ch.bytes[3] & 0x3F;
            break;
        default:
            scf_raise_error(SCF_INVALID_STRING_OPERATION, "Invalid byte count in utf8 character");
    }
    
    return result;
}

static scf_codepoint codepoint_from_utf16_char(scf_char ch) {
    if (ch.byte_count != 2) {
        scf_raise_error(SCF_INVALID_STRING_OPERATION, "Invalid byte count in utf16 character");
    }
    
    scf_codepoint bytes[] = {ch.bytes[0], ch.bytes[1]};
    if (ch.encoding == SCF_UTF16_BE) {
        return (bytes[0] << 8) + bytes[1];
    } else {
        return (bytes[1] << 8) + bytes[0];
    }
}

static int get_byte_count(const scf_string *s, int offset) {
    int result = -1;
    if (offset >= s->buf.size) goto invalid_offset;
    
    switch (s->encoding) {
        case SCF_UTF8: {
            unsigned char disc = s->buf.data[offset] & 0xF8;
            if (disc < 0x7F) {
                result = 1;
                break;
            }
            
            if (disc >= 0xC0 && disc < 0xE0) {
                result = 2;
                break;
            }
            
            if (disc >= 0xE0 && disc < 0xF0) {
                result = 3;
                break;
            }
            
            if (disc >= 0xF0 && disc < 0xF8) {
                result = 4;
                break;
            }
            
            break;
        }
            
        case SCF_UTF16_BE:
        case SCF_UTF16_LE:
            result = 2;
            break;
        default:
            invalid_encoding();
    }
    
    if (offset + result - 1 >= s->buf.size) goto invalid_offset;
    return result;
    
invalid_offset:
    return -1;
}

scf_char scf_char_from_codepoint(scf_codepoint cp, scf_encoding enc) {
    switch (enc) {
        case SCF_UTF8:
            return utf8_char_from_codepoint(cp);
        case SCF_UTF16_BE:
        case SCF_UTF16_LE:
            return utf16_char_from_codepoint(cp, enc);
        default:
            invalid_encoding();
    }
}

scf_codepoint scf_codepoint_from_char(scf_char ch) {
    switch (ch.encoding) {
        case SCF_UTF8:
            return codepoint_from_utf8_char(ch);
        case SCF_UTF16_BE:
        case SCF_UTF16_LE:
            return codepoint_from_utf16_char(ch);
        default:
            invalid_encoding();
    }
}

int scf_char_cmp(scf_char ch1, scf_char ch2) {
    scf_codepoint cp1 = scf_codepoint_from_char(ch1);
    scf_codepoint cp2 = scf_codepoint_from_char(ch2);
    return CMP(cp1, cp2);
}

scf_char scf_char_to_lower(scf_char ch) {
    scf_codepoint cp = scf_codepoint_from_char(ch);
    scf_char_info info = scf_get_char_info(cp);
    if (info.category == UC_NONE) {
        return ch;
    } else {
        return scf_char_from_codepoint(info.lc_codepoint, ch.encoding);
    }
}

int scf_char_to_int(scf_char ch) {
    scf_codepoint cp = scf_codepoint_from_char(ch);
    scf_char_info info = scf_get_char_info(cp);
    if (info.category == UC_NONE) {
        return -1;
    } else {
        return info.digit_value;
    }
}

scf_char scf_char_to_upper(scf_char ch) {
    scf_codepoint cp = scf_codepoint_from_char(ch);
    scf_char_info info = scf_get_char_info(cp);
    if (info.category == UC_NONE) {
        return ch;
    } else {
        return scf_char_from_codepoint(info.uc_codepoint, ch.encoding);
    }
}

scf_string *scf_string_with_encoding(scf_operation *op, scf_encoding encoding) {
    scf_string *result = scf_alloc(op, sizeof(scf_string));
    result->encoding = encoding;
    result->char_count = 0;
    result->buf = scf_buffer_create(op, INITIAL_STRING_SIZE);
    return result;
}

void scf_string_append_char(scf_string *s, scf_char c) {
    if (s->encoding == c.encoding) {
        scf_buffer_append_bytes(&s->buf, c.bytes, c.byte_count);
    } else {
        scf_codepoint cp = scf_codepoint_from_char(c);
        scf_char converted = scf_char_from_codepoint(cp, s->encoding);
        scf_buffer_append_bytes(&s->buf, converted.bytes, converted.byte_count);
    }
    
    s->char_count++;
}

int scf_string_cmp(const scf_string *s1, const scf_string *s2) {
    int result;
    if (s1->encoding == s2->encoding) {
        size_t s1_byte_count = s1->buf.size;
        size_t s2_byte_count = s2->buf.size;
        size_t number_of_bytes_to_compare = MIN(s1_byte_count, s2_byte_count);
        result = memcmp(s1->buf.data, s2->buf.data, number_of_bytes_to_compare);
        if (result == 0) {
            result = CMP(s1_byte_count, s2_byte_count);
        }
    } else {
        size_t s1_char_count = s1->char_count;
        size_t s2_char_count = s2->char_count;
        size_t number_of_chars_to_compare = MIN(s1_char_count, s2_char_count);
        scf_string_iterator iter1 = scf_string_start(s1);
        scf_string_iterator iter2 = scf_string_start(s2);
        result = 0;
        for (size_t i = 0; i < number_of_chars_to_compare && result == 0; i++) {
            scf_char ch1, ch2;
            scf_string_next(&iter1, &ch1);
            scf_string_next(&iter2, &ch2);
            result = scf_char_cmp(ch1, ch2);
        }

        if (result == 0) {
            result = CMP(s1_char_count, s2_char_count);
        }
    }
    
    return result;
}


scf_string *scf_string_convert(const scf_string *s, scf_encoding target_encoding) {
    scf_operation *op = scf_get_operation(s->buf.data);
    scf_string *result = scf_string_with_encoding(op, target_encoding);
    scf_string_append(result, s);
    return result;
}

bool scf_string_next(scf_string_iterator *iter, scf_char *c) {
    scf_char result;
    result.encoding = iter->s->encoding;
    int byte_count = get_byte_count(iter->s, iter->index);
    if (byte_count == -1) {
        return false;
    }
    
    if (c) {
        c->encoding = iter->s->encoding;
        c->byte_count = byte_count;
        memcpy(c->bytes, iter->s->buf.data + iter->index, byte_count);
    }

    iter->index += byte_count;
    return true;
}


void scf_string_append(scf_string *s1, const scf_string *s2) {
    if (s1->encoding == s2->encoding) {
        scf_buffer_append(&s1->buf, &s2->buf);
        s1->char_count += s2->char_count;
    } else {
        scf_string_iterator iter = scf_string_start(s2);
        scf_char ch;
        while (scf_string_next(&iter, &ch)) {
            scf_string_append_char(s1, ch);
        }
    }
}

void scf_string_append_cstr(scf_string *s, const char *cstr) {
    SCF_OPERATION(op);
    scf_string_append(s, scf_string_from_cstr(&op, cstr));
    scf_complete(&op);
}

void scf_string_append_ascii(scf_string *s, char ascii) {
    scf_string_append_char(s, scf_ascii(ascii));
}

scf_string *scf_string_from_bytes(scf_operation *op, const void *p, size_t byte_count, scf_encoding encoding) {
    scf_string *result = scf_alloc(op, sizeof(scf_string));
    result->encoding = encoding;
    result->buf = scf_buffer_create(op, byte_count);
    scf_buffer_append_bytes(&result->buf, p, byte_count);
    scf_string_iterator iter = scf_string_start(result);
    result->char_count = 0;
    while (scf_string_next(&iter, NULL)) {
        result->char_count++;
    }
    
    return result;
}


char *scf_string_to_cstr(const scf_string *s) {
    const scf_string *utf8;
    scf_string *converted = NULL;
    if (s->encoding == SCF_UTF8) {
        utf8 = s;
    } else {
        converted = scf_string_convert(s, SCF_UTF8);
        utf8 = converted;
    }
    
    scf_operation *op = scf_get_operation(s->buf.data);
    size_t size = utf8->buf.size;
    char *result = scf_alloc(op, size + 1);
    memcpy(result, s->buf.data, size);
    result[size] = '\0';
    if (converted) {
        scf_string_free(converted);
    }
    
    return result;
}

scf_string *scf_string_clone(scf_operation *op, const scf_string *s) {
    op = op ? op : scf_get_operation(s);
    scf_string *result = scf_string_with_encoding(op, s->encoding);
    scf_buffer_append(&result->buf, &s->buf);
    result->char_count = s->char_count;
    return result;
}

scf_string_iterator scf_string_iterator_at(const scf_string *s, int index) {
    scf_string_iterator result = scf_string_start(s);
    for (int i = 0; i < index; i++) {
        if (!scf_string_next(&result, NULL)) {
            break;
        }
    }
    
    return result;
}

scf_string *scf_substring(scf_string_iterator start, int char_count) {
    const scf_string *s = start.s;
    scf_operation *op = scf_get_operation(s->buf.data);
    scf_string *result = scf_string_with_encoding(op, s->encoding);
    scf_string_iterator iter = start;
    for (int i = 0; i < char_count; i++) {
        scf_char ch;
        if (!scf_string_next(&iter, &ch)) {
            break;
        }
        
        scf_string_append_char(result, ch);
    }
    
    return result;
}

scf_stringlist *scf_stringlist_create(scf_operation *op) {
    scf_stringlist *result = scf_alloc(op, sizeof(scf_stringlist));
    result->strings = scf_list_create(op, sizeof(scf_string *), INITIAL_STRINGLIST_SIZE);
    return result;
}

void scf_stringlist_add(scf_stringlist *list, const scf_string *s) {
    scf_string *clone = scf_string_clone(scf_get_operation(list), s);
    scf_list_add(&list->strings, &clone);
}

scf_string *scf_stringlist_get(const scf_stringlist *list, size_t index) {
    scf_string *result;
    scf_list_get(&list->strings, index, &result);
    return result;
}

scf_string *scf_stringlist_get_copy(scf_operation *op, const scf_stringlist *list, size_t index) {
    return scf_string_clone(op, scf_stringlist_get(list, index));
}


void scf_stringlist_clear(scf_stringlist *list) {
    scf_list_clear(&list->strings);
}

void scf_stringlist_insert(scf_stringlist *list, const scf_string *s, size_t before) {
    scf_string *clone = scf_string_clone(scf_get_operation(list), s);
    scf_list_insert(&list->strings, &clone, before);
}

scf_string *scf_stringlist_remove(scf_stringlist *list, size_t index) {
    scf_string *result = scf_stringlist_get(list, index);
    scf_list_remove(&list->strings, index);
    return result;
}

static int default_comparison_func(const void *p1, const void *p2) {
    const scf_string *s1 = SCF_DEREF(const scf_string *, p1);
    const scf_string *s2 = SCF_DEREF(const scf_string *, p2);
    return scf_string_cmp(s1, s2);
}

void scf_stringlist_sort(scf_stringlist *list, scf_comparison_func cmp) {
    if (cmp == NULL) {
        cmp = default_comparison_func;
    }
    
    scf_list_sort(&list->strings, cmp);
}



// extern defs for inline functions
extern scf_string_iterator scf_string_start(const scf_string *s);
extern scf_string *scf_utf8_string(scf_operation *op);
extern void scf_string_free(scf_string *s);
extern scf_string *scf_string_from_cstr(scf_operation *op, const char *cstr);
extern scf_char scf_ascii(char c);
extern void scf_stringlist_push(scf_stringlist *list, const scf_string *s);
extern scf_string *scf_stringlist_pop(scf_stringlist *list);
extern void scf_stringlist_add_cstr(scf_stringlist *list, const char *cstr);
extern size_t scf_stringlist_size(const scf_stringlist *list);


