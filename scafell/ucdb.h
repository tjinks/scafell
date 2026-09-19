#ifndef ucdb_h
#define ucdb_h

#include <stdint.h>

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
    int codepoint;
    scf_char_category category;
    int digit_value;
    int uc_codepoint;
    int lc_codepoint;
    int tc_codepoint;
} scf_char_info;

typedef int32_t scf_codepoint;

scf_char_info scf_get_char_info(scf_codepoint ch);

scf_codepoint scf_last_uc_codepoint(void);

#endif /* ucdb_h */
