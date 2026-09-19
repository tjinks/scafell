//
//  unix_file_io_tests.c
//  ScafellTest
//
//  Created by Tony on 17/09/2026.
//

#include <stdlib.h>
#include "scuts.h"
#include "err_handling.h"
#include "mmgt.h"
#include "str.h"
#include "os/unix_file_io.h"
#include "os/osdefs.h"

SCF_OPERATION(op);

static scf_string *working_dir_name;

static void *create_working_dir(void) {
    const char *tmpdir;
    tmpdir = getenv("TMPDIR");
    if (!tmpdir) {
        tmpdir = "/tmp";
    }

    if (tmpdir) {
        return scf_string_from_cstr(&op, tmpdir);
    }
    
    ASSERT_FAILURE("Unable to get temp dir name on this OS");
    return NULL;
}

