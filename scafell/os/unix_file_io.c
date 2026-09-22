//
//  unix_file_io.c
//  scafell
//
//  Created by Tony on 15/09/2026.
//

#include <errno.h>
#include <unistd.h>
#include "unix_file_io.h"
#include "err_handling.h"

static const size_t default_buffer_size = 1024;

static bool fill(scf_reader *reader, scf_err_context *ec) {
    int fd = SCF_DEREF(int, reader->additional_data);
    ssize_t read_count = read(fd, reader->buf.data, reader->buf.capacity);
    reader->index = 0;
    reader->buf.size = read_count;
    if (read_count == 0) {
        return false;
    }
    
    if (read_count == -1) {
        scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
    }
    
    return true;
}

static void close_reader(scf_reader *reader, scf_err_context *ec) {
    int fd = SCF_DEREF(int, reader->additional_data);
    if (close(fd) == 0) {
        return;
    }
    
    scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
}

static void empty(scf_writer *writer, scf_err_context *ec) {
    if (writer->buf.size == 0) {
        return;
    }
    
    int fd = SCF_DEREF(int, writer->additional_data);
    ssize_t write_count = write(fd, writer->buf.data, writer->buf.size);
    
    if (write_count == -1) {
        scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
    }
    
    if (write_count < writer->buf.size) {
        scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
    }
    
    scf_buffer_clear(&writer->buf);
}

static void close_writer(scf_writer *writer, scf_err_context *ec) {
    empty(writer, ec);
    int fd = SCF_DEREF(int, writer->additional_data);
    if (close(fd) == 0) {
        return;
    }
    
    scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
}

scf_reader *scf_os_file_reader_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               scf_err_context *ec) {
    const char *path_as_cstr = scf_string_to_cstr(path);
    int *fd = SCF_ALLOC(op, int);
    *fd = open(path_as_cstr, O_RDONLY);
    if (*fd == -1) {
        switch (errno) {
            case EACCES:
                scf_raise_error(scf_os_err_info_create(SCF_ACCESS_DENIED, errno), ec);
            case ENOENT:
            case ENOTDIR:
                scf_raise_error(scf_os_err_info_create(SCF_FILE_DOES_NOT_EXIST, errno), ec);
            default:
                scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);

        }
    }
    
    scf_reader *result = scf_alloc_with_cleanup(op, sizeof(scf_reader), scf_cleanup_reader);
    result->buf = scf_buffer_create(op, buffer_size ? buffer_size : default_buffer_size);
    result->fill = fill;
    result->close = close_reader;
    result->additional_data = fd;
    result->index = 0;
    result->is_open = true;
    return result;
}

scf_writer *scf_os_file_writer_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               bool append,
                               scf_err_context *ec) {
    const char *path_as_cstr = scf_string_to_cstr(path);
    int *fd = SCF_ALLOC(op, int);
    int mode = O_WRONLY;
    if (append) {
        mode |= O_APPEND;
    } else {
        mode |= O_CREAT;
    }
    
    *fd = open(path_as_cstr, mode, 0500);
    if (*fd == -1) {
        switch (errno) {
            case EACCES:
                scf_raise_error(scf_os_err_info_create(SCF_ACCESS_DENIED, errno), ec);
            case ENOENT:
            case ENOTDIR:
                scf_raise_error(scf_os_err_info_create(SCF_FILE_DOES_NOT_EXIST, errno), ec);
            default:
                scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);

        }
    }
    
    scf_writer *result = scf_alloc_with_cleanup(op, sizeof(scf_writer), scf_cleanup_writer);
    result->buf = scf_buffer_create(op, buffer_size ? buffer_size : default_buffer_size);
    result->empty = empty;
    result->close = close_writer;
    result->additional_data = fd;
    result->is_open = true;
    return result;
}
