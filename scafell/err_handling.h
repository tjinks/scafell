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
#include <setjmp.h>
#include "osdefs.h"

#define SCF_INTERNAL_ERR_START (1000)
#define SCF_OS_ERR_START (2000)
#define SCF_CUSTOM_ERR_START (10000)

enum {
    SCF_SUCCESS = 0
    
    ,SCF_OUT_OF_MEMORY = SCF_INTERNAL_ERR_START
    ,SCF_LOGIC_ERROR
    ,SCF_BAD_INDEX
    ,SCF_INVALID_STRING_OPERATION

    ,SCF_OS_ERROR = SCF_OS_ERR_START
    ,SCF_IO_ERROR
};

#define SCF_MAX_ERR_MSG_SIZE (1000)

typedef struct {
    int code;
    bool is_os_error;
    scf_os_error_code os_error_code;
    char message[SCF_MAX_ERR_MSG_SIZE + 1];
} scf_err_info;

typedef struct {
    jmp_buf target;
    scf_err_info err_info;
} scf_err_context;

#define SCF_ERR_INFO(varname) scf_err_info varname = {SCF_SUCCESS, false, (scf_os_error_code)0, {'\0'}}

#define SCF_TRY(ec) { \
scf_err_context ec; \
ec.err_info = scf_err_info_create(SCF_SUCCESS, ""); \
if (!setjmp(ec.target))

#define SCF_CATCH else

#define SCF_END_TRY }

scf_err_info scf_err_info_create(int, const char *msg);

scf_err_info scf_os_err_info_create(int scf_error, scf_os_error_code os_error);

typedef void (*scf_fatal_err_handler)(const scf_err_info *err_info);

void scf_set_fatal_err_handler(scf_fatal_err_handler handler);

noreturn void scf_raise_fatal_error(int error_code, const char *msg);

noreturn void scf_raise_fatal_os_error(int error_code, scf_os_error_code os_error_code);

noreturn void scf_raise_error(scf_err_info err_info, scf_err_context *ec);

#endif /* err_handling_h */
