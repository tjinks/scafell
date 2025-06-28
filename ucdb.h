#ifndef ucdb_h
#define ucdb_h

#include "str.h"

typedef enum {
    UC_LETTER = 0x8
    ,UC_UPPER = 0x1
    ,UC_LOWER = 0x2
    ,UC_TITLE = 0x4
    ,UC_DIGIT = 0x10
    ,UC_SPACE = 0x20
    ,UC_OTHER = 0x40
    ,UC_NONE = 0x00
} scf_char_category;



typedef struct {
    utf8_char base, lower, upper, title;
    int codepoint;
    int digit_value;
    scf_char_category category;
} scf_char_info;

void scf_ucdb_init(void);
void scf_ucdb_close(void);

const scf_char_info scf_get_char_info(utf8_char ch);

#endif /* ucdb_h */
