#include "ucsdb.h"
#include "unicode_data.h"
#include "hash.h"
#include "mmgt.h"

const int32_t UCS_INVALID = -1;

static scf_dictionary ucsdb;

static const int NUMBER_OF_CODEPOINTS = sizeof(unicode_data) / sizeof(unicode_data[0]);

static SCF_OPERATION(op);

static size_t hashfunc(scf_datum d) {
    int ch = (int) d.i_value;
    int result =  ((ch >> 16) ^ ch);
    result = result ^ (result << 8);
    return result;
}

static void hash_db(void) {
    for (int i = 0; i < NUMBER_OF_CODEPOINTS; i++) {
        scf_datum key = dt_int(unicode_data[i].utf8);
        scf_datum value = dt_ptr(unicode_data + i);
        scf_dictionary_add(&ucsdb, key, value);
    }
}

void ucs_dbinit(void) {
    ucsdb = scf_dictionary_create(&op, hashfunc, dt_int_compare, NUMBER_OF_CODEPOINTS);
    hash_db();
}

void ucs_dbclose(void) {
    scf_complete(&op);
}

ucs_details ucs_lookup(ucs_utf8_char utf8) {
    ucs_details result;
    const scf_datum *value = scf_dictionary_lookup(&ucsdb, dt_int(utf8));
    if (value) {
        result = *((ucs_details *)value->p_value);
    } else {
        result.codepoint = UCS_INVALID;
        result.utf8 = UCS_INVALID;
        result.lc_utf8 = UCS_INVALID;
        result.uc_utf8 = UCS_INVALID;
        result.tc_utf8 = UCS_INVALID;
        result.category = UCS_NONE;
        result.digit_value = -1;
    }
    
    return result;
}
