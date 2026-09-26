//
//  unix_file_io_tests.c
//  ScafellTest
//
//  Created by Tony on 17/09/2026.
//

#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "scuts.h"
#include "err_handling.h"
#include "mmgt.h"
#include "str.h"
#include "file_io.h"
#include "os/osdefs.h"

SCF_OPERATION(op);

static scf_string *path_to_working_dir;

static void delete_working_dir(void);

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
        }
        
        scf_string_append(path_to_working_dir, working_dir_name);
        delete_working_dir();
        const char *wdpath = scf_string_to_cstr(path_to_working_dir);
        mkdir(wdpath, 0700);
        return;
    }
    
    ASSERT_FAILURE("Unable to get temp dir name on this OS");
}

static void delete_working_dir(void) {
    DIR *dir = opendir(scf_string_to_cstr(path_to_working_dir));
    if (dir) {
        scf_stringlist *contents = scf_stringlist_create(&op);
        struct dirent *next = readdir(dir);
        while (next) {
            if (strcmp(next->d_name, ".")) {
                if (strcmp(next->d_name, "..")) {
                    scf_string *full_path = scf_string_clone(NULL, path_to_working_dir);
                    scf_string_append_char(full_path, scf_ascii('/'));
                    scf_string_append(full_path, scf_string_from_cstr(&op, next->d_name));
                    scf_stringlist_add(contents, full_path);
                }
            }
            
            next = readdir(dir);
        }
        
        closedir(dir);
        for (size_t i = 0; i < contents->strings.size; i++) {
            scf_string *path = scf_stringlist_get(contents, i);
            const char *path_cstr = scf_string_to_cstr(path);
            unlink(path_cstr);
        }
        
        rmdir(scf_string_to_cstr(path_to_working_dir));
    }
}

void unix_file_io_tests_init(void) {
    create_working_dir();
}

void unix_file_io_tests_cleanup(void) {
    delete_working_dir();
    scf_complete(&op);
}

bool test_write_and_read_single_byte(void) {
    bool result = true;
    scf_string *test_file_path = scf_string_clone(NULL, path_to_working_dir);
    scf_string_append(test_file_path, scf_string_from_cstr(&op, "/testfile"));
    SCF_TRY(ec) {
        scf_os_file_writer *writer = scf_os_file_writer_create(&op, test_file_path, 5, false, &ec);
        scf_os_write_single_byte(writer, 123, &ec);
        scf_os_close_writer(writer, &ec);
        
        scf_os_file_reader *reader = scf_os_file_reader_create(&op, test_file_path, 0, &ec);
        unsigned char byte = 0;
        result = result && ASSERT_TRUE(scf_os_read_single_byte(reader, &byte, &ec));
        result = result && ASSERT_EQ(123, byte);
        result = result && ASSERT_FALSE(scf_os_read_single_byte(reader, &byte, &ec));
        scf_os_close_reader(reader, &ec);
    }
    SCF_CATCH {
        result = ASSERT_FAILURE(ec.err_info.message);
    }
    SCF_END_TRY
    
    return result;
}

bool test_write_and_read_block(void) {
    bool result = true;
    scf_string *test_file_path = scf_string_clone(NULL, path_to_working_dir);
    scf_string_append(test_file_path, scf_string_from_cstr(&op, "/testfile"));
    SCF_TRY(ec) {
        scf_os_file_writer *writer = scf_os_file_writer_create(&op, test_file_path, 0, false, &ec);
        scf_os_write_single_byte(writer, '1', &ec);
        scf_os_write_bytes(writer, (unsigned char *)"23456789", 8, &ec);
        scf_os_close_writer(writer, &ec);
        
        scf_os_file_reader *reader = scf_os_file_reader_create(&op, test_file_path, 5, &ec);
        unsigned char byte = 0;
        result = result && ASSERT_TRUE(scf_os_read_single_byte(reader, &byte, &ec));
        result = result && ASSERT_EQ('1', byte);
        scf_buffer buf = scf_os_read_bytes(&op, reader, 10, &ec);
        result = result && ASSERT_EQ(8, buf.size);
        result = result && ASSERT_EQ(0, memcmp("23456789", buf.data, 8));
        result = result && ASSERT_FALSE(scf_os_read_single_byte(reader, &byte, &ec));
        scf_os_close_reader(reader, &ec);
    }
    SCF_CATCH {
        result = ASSERT_FAILURE(ec.err_info.message);
    }
    SCF_END_TRY
    
    return result;
}

static bool callback(const scf_buffer *buf, void *additional_data, scf_err_context *ec) {
    scf_buffer *data = additional_data;
    scf_buffer_append(data, buf);
    return true;
}

bool test_read_with_callback(void) {
    bool result = true;
    scf_string *test_file_path = scf_string_clone(NULL, path_to_working_dir);
    scf_string_append(test_file_path, scf_string_from_cstr(&op, "/testfile"));
    SCF_TRY(ec) {
        scf_os_file_writer *writer = scf_os_file_writer_create(&op, test_file_path, 0, false, &ec);
        scf_os_write_single_byte(writer, '1', &ec);
        scf_os_write_bytes(writer, (unsigned char *)"23456789", 8, &ec);
        scf_os_close_writer(writer, &ec);
        
        scf_os_file_reader *reader = scf_os_file_reader_create(&op, test_file_path, 5, &ec);
        unsigned char byte;
        scf_os_read_single_byte(reader, &byte, &ec);
        result = result && ASSERT_EQ('1', byte);
        scf_buffer data = scf_buffer_create(&op, 4);
        scf_os_read_with_callback(reader, callback, &data, &ec);
        result = result && ASSERT_EQ(8, data.size);
        result = result && ASSERT_EQ(0, memcmp("23456789", data.data, 8));
        result = result && ASSERT_FALSE(scf_os_read_single_byte(reader, &byte, &ec));
        scf_os_close_reader(reader, &ec);
    }
    SCF_CATCH {
        result = ASSERT_FAILURE(ec.err_info.message);
    }
    SCF_END_TRY
    
    return result;
}

BEGIN_TEST_GROUP(unix_file_io_tests)
    INIT(unix_file_io_tests_init)
    CLEANUP(unix_file_io_tests_cleanup)
    TEST(test_write_and_read_single_byte)
    TEST(test_write_and_read_block)
    TEST(test_read_with_callback)
END_TEST_GROUP

