//
//  datum.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include "datum.h"

const int SCF_DATUM_SIZE = sizeof(scf_datum);

scf_datum dt_none(void) {
    static scf_datum none = {DT_NONE, .i_value = 0};
    return none;
}

scf_datum dt_true(void) {
    static scf_datum _true = {DT_BOOL, .b_value = true};
    return _true;
}

scf_datum dt_false(void) {
    static scf_datum _false = {DT_BOOL, .b_value = false};
    return _false;
}

scf_datum dt_int(int i) {
    scf_datum result = {DT_INT, .i_value = i};
    return result;
}

scf_datum dt_ptr(void *p) {
    scf_datum result = {DT_PTR, .p_value = p};
    return result;
}

bool dt_int_compare(scf_datum d1, scf_datum d2) {
    if (d1.type != DT_INT || d2.type != DT_INT) return false;
    return d1.i_value == d2.i_value;
}

