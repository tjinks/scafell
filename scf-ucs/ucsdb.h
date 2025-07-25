#ifndef ucdb_h
#define ucdb_h

#include <stdint.h>

typedef enum {
    UCS_LETTER = 0x8
    ,UCS_UPPER = 0x1
    ,UCS_LOWER = 0x2
    ,UCS_TITLE = 0x4
    ,UCS_DIGIT = 0x10
    ,UCS_SPACE = 0x20
    ,UCS_OTHER = 0x40
    ,UCS_NONE = 0x00
} ucs_char_category;

typedef int32_t ucs_codepoint;

extern const ucs_codepoint UCS_INVALID;

typedef struct {
    ucs_codepoint codepoint;
    ucs_char_category category;
    int digit_value;
    ucs_codepoint uc_codepoint;
    ucs_codepoint lc_codepoint;
    ucs_codepoint tc_codepoint;
} ucs_details;

void ucs_dbinit(void);
void ucs_dbclose(void);

ucs_details ucs_lookup(ucs_codepoint cp);

#endif /* ucdb_h */
