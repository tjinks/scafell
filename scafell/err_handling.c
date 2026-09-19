//
//  err_handling.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err_handling.h"

static scf_fatal_err_handler err_handler;

static noreturn void default_err_handler(const scf_err_info *err_info) {
    if (err_info->is_os_error) {
        fprintf(stderr, "Error Code = %d, OS Error Code = %d: %s\n", (int)err_info->code, (int)err_info->os_error_code, err_info->message);
    } else {
        fprintf(stderr, "Error Code = %d: %s\n", (int)err_info->code, err_info->message);
    }
    
    exit(1);
}

static void set_message(scf_err_info *err_info, const char *msg) {
    for (size_t i = 0; i < SCF_MAX_ERR_MSG_SIZE; i++) {
        err_info->message[i] = msg[i];
        if (!msg[i]) {
            return;
        }
    }
    
    err_info->message[SCF_MAX_ERR_MSG_SIZE] = '\0';
}

static noreturn void raise_fatal_error(scf_err_info err_info) {
    if (err_handler) {
        err_handler(&err_info);
    } else {
        default_err_handler(&err_info);
    }
    
    fprintf(stderr, "Unexpected return from error handler\n");
    abort();
}

void scf_set_fatal_err_handler(scf_fatal_err_handler handler) {
    err_handler = handler;
}

scf_err_info scf_err_info_create(scf_error_code code, const char *msg) {
    scf_err_info result = {code, false, (scf_os_error_code)0};
    set_message(&result, msg);
    return result;
}

scf_err_info scf_os_err_info_create(scf_error_code scf_error, scf_os_error_code os_error) {
    scf_err_info result = {scf_error, true, os_error};
    
#ifdef SCF_OS_UNIX
    set_message(&result, strerror(os_error));
#endif
    
    return result;
}

noreturn void scf_raise_fatal_error(scf_error_code code, const char *msg) {
    raise_fatal_error(scf_err_info_create(code, msg));
}

noreturn void scf_raise_fatal_os_error(scf_error_code code, scf_os_error_code os_error_code) {
    scf_err_info err_info = scf_os_err_info_create(code, os_error_code);
    raise_fatal_error(err_info);
}

noreturn void scf_raise_error(scf_err_info err_info, scf_err_context *ec) {
    if (ec == NULL) {
        raise_fatal_error(err_info);
    } else {
        ec->err_info = err_info;
        longjmp(ec->target, 1);
    }
}



