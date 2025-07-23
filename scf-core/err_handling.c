//
//  err_handling.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include <stdlib.h>

#include "err_handling.h"

static scf_err_handler err_handler;

static noreturn void default_err_handler(const scf_err_info *err_info) {
    fprintf(stderr, "Error Code = %d, OS Error Code = %d: %s\n", (int)err_info->code, (int)err_info->os_error_code, err_info->message);
    exit(1);
}

static noreturn void raise_error(scf_error_code code, scf_os_error_code os_error_code, const char *msg) {
    scf_err_info err_info = {code, os_error_code};
    int i;
    for (i = 0; i < MAX_ERR_MSG_LEN && msg[i]; i++) {
        err_info.message[i] = msg[i];
    }
    
    err_info.message[i] = '\0';
    if (err_handler) {
        err_handler(&err_info);
    }
    
    default_err_handler(&err_info);
}

void scf_set_err_handler(scf_err_handler handler) {
    err_handler = handler;
}

void scf_raise_error(scf_error_code code, const char *msg) {
    raise_error(code, 0, msg);
}

void scf_raise_os_error(scf_os_error_code code, const char *msg){
    raise_error(SCF_OS_ERROR, code, msg);
}


