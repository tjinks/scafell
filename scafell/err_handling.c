//
//  err_handling.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include <stdlib.h>

#include "err_handling.h"

const scf_err_info scf_success = {SCF_SUCCESS, 0, ""};

static scf_err_handler err_handler;

static noreturn void default_err_handler(const scf_err_info *err_info) {
    fprintf(stderr, "Error Code = %d, OS Error Code = %d: %s\n", (int)err_info->code, (int)err_info->os_error_code, err_info->message);
    exit(1);
}

static noreturn void raise_error(scf_error_code code, scf_os_error_code os_error_code, const char *msg) {
    scf_err_info err_info = {code, os_error_code, msg};
    if (err_handler) {
        err_handler(&err_info);
    } else {
        default_err_handler(&err_info);
    }
    
    fprintf(stderr, "Unexpected return from error handler\n");
    abort();
}

void scf_set_err_handler(scf_err_handler handler) {
    err_handler = handler;
}

scf_err_info scf_err_info_create(scf_error_code code, const char *msg) {
    scf_err_info result = {code, 0, msg};
    return result;
}

scf_err_info scf_os_err_info_create(scf_os_error_code code) {
    scf_err_info result = {SCF_OS_ERROR, code, ""};
    
#ifdef UNIX
    result.message = strerror(code);
#endif
    
    return result;
}


noreturn void scf_raise_error(scf_error_code code, const char *msg) {
    raise_error(code, 0, msg);
}

noreturn void scf_raise_os_error(scf_os_error_code code, const char *msg){
    raise_error(SCF_OS_ERROR, code, msg);
}

extern bool scf_ok(scf_err_info info);

