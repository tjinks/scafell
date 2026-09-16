//
//  err_handling.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef err_handling_h
#define err_handling_h

#include <stdnoreturn.h>
#include <stdbool.h>
#include "os/osdefs.h"

typedef enum {
    SCF_SUCCESS = 0
    
    ,SCF_OUT_OF_MEMORY
    ,SCF_LOGIC_ERROR
    ,SCF_OS_ERROR
    ,SCF_BAD_INDEX
    ,SCF_INVALID_STRING_OPERATION
    ,SCF_EOF
} scf_error_code;

typedef struct scf_err_info {
    scf_error_code code;
    scf_os_error_code os_error_code;
    const char *message;
} scf_err_info;

extern const scf_err_info scf_success;

inline bool scf_ok(scf_err_info info) {
    return info.code == SCF_SUCCESS;
}

scf_err_info scf_err_info_create(scf_error_code, const char *msg);

scf_err_info scf_os_err_info_create(scf_os_error_code);

typedef void (*scf_err_handler)(const scf_err_info *err_info);

void scf_set_err_handler(scf_err_handler handler);

noreturn void scf_raise_error(scf_error_code code, const char *msg);

noreturn void scf_raise_os_error(scf_os_error_code code, const char *msg);

#endif /* err_handling_h */
