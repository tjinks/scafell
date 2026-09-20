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

static scf_string *path_to_working_dir;

void unix_file_io_tests_cleanup(void) {
    scf_complete(&op);
}

static void create_working_dir(void) {
    const char *tmpdir;
    tmpdir = getenv("TMPDIR");
    if (!tmpdir) {
        tmpdir = "/tmp";
    }

    if (tmpdir) {
        scf_string *slash = scf_string_from_cstr(&op, "/");
        scf_string *working_dir_name = scf_string_from_cstr(&op, "_SCF_TESTING");
        path_to_working_dir = scf_string_from_cstr(&op, tmpdir);
        scf_string *last_char = scf_substring(scf_string_iterator_at(path_to_working_dir, -1), 1);
        if (scf_string_cmp(slash, last_char) != 0) {
            scf_string_append(path_to_working_dir, slash);
            return;
        }
    }
    
    ASSERT_FAILURE("Unable to get temp dir name on this OS");
}

void unix_file_io_tests_init(void) {
    create_working_dir();
}

bool dummy_test(void) {
    return true;
}

BEGIN_TEST_GROUP(unix_file_io_tests)
    INIT(unix_file_io_tests_init)
    CLEANUP(unix_file_io_tests_cleanup)
    TEST(dummy_test)
END_TEST_GROUP

