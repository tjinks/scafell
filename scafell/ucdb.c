//
//  ucdb.c
//  scafell
//
//  Created by Tony on 21/06/2025.
//

#include "ucdb.h"
#include "unicode_data.h"
#include "hash.h"
#include "str.h"
#include "mmgt.h"

static scf_dictionary ucdb;

static const int UC_CHAR_COUNT = sizeof(uc_database) / sizeof(uc_database[0]);

static SCF_OPERATION(op);

static size_t hashfunc(scf_datum d) {
    int ch = (int) d.i_value;
    int result =  ch ^ (ch >> 16);
    return result;
}

static scf_char_info *populate_char_data(void) {
    scf_char_info *result = scf_alloc(&op, sizeof(scf_char_info) * UC_CHAR_COUNT);
    for (int i = 0; i < UC_CHAR_COUNT; i++) {
        scf_char_info *item = result + i;
        item->base = utf8_from_codepoint(uc_database[i].codepoint);
        item->lower = utf8_from_codepoint(uc_database[i].lc_codepoint);
        item->upper = utf8_from_codepoint(uc_database[i].uc_codepoint);
        item->title = utf8_from_codepoint(uc_database[i].tc_codepoint);
        item->digit_value = uc_database[i].digit_value;
        item->category = uc_database[i].category;
        item->codepoint = uc_database[i].codepoint;
    }
    
    return result;
}

static void hash_char_data(scf_char_info *char_data) {
    for (int i = 0; i < UC_CHAR_COUNT; i++) {
        scf_datum key = dt_int(char_data[i].base);
        scf_datum value = dt_ptr(char_data + i);
        scf_dictionary_add(&ucdb, key, value);
    }
}

void scf_ucdb_init(void) {
    ucdb = scf_dictionary_create(&op, hashfunc, dt_int_compare, UC_CHAR_COUNT);
    scf_char_info *char_data = populate_char_data();
    hash_char_data(char_data);
}

void scf_ucdb_close(void) {
    scf_complete(&op);
}

const scf_char_info scf_get_char_info(utf8_char ch) {
    scf_char_info result;
    const scf_datum *value = scf_dictionary_lookup(&ucdb, dt_int(ch));
    if (value) {
        result = *((scf_char_info *)value->p_value);
    } else {
        result.base = UTF8_INVALID;
        result.lower = result.base;
        result.upper = result.base;
        result.title = result.base;
        result.codepoint = result.base;
        result.category = UC_NONE;
        result.digit_value = -1;
    }
    
    return result;
}


