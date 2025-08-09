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

typedef uint32_t ucs_codepoint;

/*------------------------------------------------------------
 * Holds the UTF8 encoding of a unicode character, in
 * big-endian format.
 -----------------------------------------------------------*/
typedef uint32_t ucs_utf8_char;


/*------------------------------------------------------------
 * Used to flag an invalid codepoint or utf8 char value.
 -----------------------------------------------------------*/
extern const uint32_t UCS_INVALID;

typedef struct {
    ucs_utf8_char utf8;
    ucs_codepoint codepoint;
    ucs_char_category category;
    int digit_value;
    ucs_utf8_char uc_utf8;
    ucs_utf8_char lc_utf8;
    ucs_utf8_char tc_utf8;
} ucs_details;

void ucs_dbinit(void);
void ucs_dbclose(void);

ucs_details ucs_lookup(ucs_codepoint cp);

#endif /* ucdb_h */
