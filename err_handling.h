//
//  err_handling.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef err_handling_h
#define err_handling_h

#include "os/osdefs.h"

#define MAX_ERR_MSG_LEN (255)

typedef enum {
    SCF_SUCCESS = 0
    ,SCF_OUT_OF_MEMORY
    ,SCF_LOGIC_ERROR
    ,SCF_OS_ERROR
} scf_error_type;



typedef struct scf_err_info {
    scf_error_type type;
    scf_os_error_code os_error_code;
    char message[MAX_ERR_MSG_LEN + 1];
} scf_err_info;

typedef void (*scf_err_handler)(void);

void scf_set_err_handler(scf_err_handler handler);

void scf_fatal_error(const char *msg);

#endif /* err_handling_h */
