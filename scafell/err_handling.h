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
#include "os/osdefs.h"

#define SCF_INTERNAL_ERR_START (1000)
#define SCF_IO_ERR_START (2000)

typedef enum {
    SCF_SUCCESS = 0
    
    ,SCF_OUT_OF_MEMORY = SCF_INTERNAL_ERR_START
    ,SCF_LOGIC_ERROR
    ,SCF_BAD_INDEX
    ,SCF_INVALID_STRING_OPERATION

    ,SCF_IO_ERROR = SCF_IO_ERR_START
    ,SCF_FILE_DOES_NOT_EXIST
    ,SCF_ACCESS_DENIED
    ,SCF_DEVICE_FULL
    ,SCF_NOT_A_DIRECTORY
} scf_error_code;

#define SCF_MAX_ERR_MSG_SIZE (1000)

typedef struct {
    scf_error_code code;
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

scf_err_info scf_err_info_create(scf_error_code, const char *msg);

scf_err_info scf_os_err_info_create(scf_error_code scf_error, scf_os_error_code os_error);

typedef void (*scf_fatal_err_handler)(const scf_err_info *err_info);

void scf_set_fatal_err_handler(scf_fatal_err_handler handler);

noreturn void scf_raise_fatal_error(scf_error_code code, const char *msg);

noreturn void scf_raise_fatal_os_error(scf_error_code code, scf_os_error_code os_error_code);

noreturn void scf_raise_error(scf_err_info err_info, scf_err_context *ec);

#endif /* err_handling_h */
