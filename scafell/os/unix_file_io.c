//
//  unix_file_io.c
//  scafell
//
//  Created by Tony on 15/09/2026.
//

#include <errno.h>
#include <unistd.h>
#include "unix_file_io.h"

#ifdef XXX

static scf_err_info fill(scf_reader *reader) {
    int fd = SCF_DEREF(int, reader->additional_data);
    ssize_t read_count = read(fd, reader->buf.data, reader->buf.capacity);
    reader->index = 0;
    reader->buf.size = read_count;
    if (read_count == 0) {
        return scf_err_info_create(SCF_EOF, "");
    }
    
    if (read_count == -1) {
        return scf_os_err_info_create(errno);
    }
    
    return scf_success;
}

static scf_err_info close_reader(scf_reader *reader) {
    int fd = SCF_DEREF(int, reader->additional_data);
    return close(fd) == 0 ? scf_success : scf_os_err_info_create(errno);
}

static scf_err_info empty(scf_writer *writer) {
    int fd = SCF_DEREF(int, writer->additional_data);
    ssize_t write_count = write(fd, writer->buf.data, writer->buf.size);
    
    if (write_count == -1) {
        return scf_os_err_info_create(errno);
    }
    
    if (write_count < writer->buf.size) {
        return scf_os_err_info_create(EIO);
    }
    
    return scf_success;
}

static scf_err_info close_writer(scf_writer *writer) {
    int fd = SCF_DEREF(int, writer->additional_data);
    return close(fd) == 0 ? scf_success : scf_os_err_info_create(errno);
}


scf_err_info scf_os_file_reader_create(scf_operation *op,
                                         const scf_string *path,
                                         size_t buffer_size,
                                         scf_reader **result) {
    *result = NULL;
    const char *path_as_cstr = scf_string_to_cstr(path);
    int *fd = SCF_ALLOC(op, int);
    *fd = open(path_as_cstr, O_RDONLY);
    if (*fd == -1) {
        switch (errno) {
            case EACCES:
                return SCF_NO_ACCESS;
            case ENOENT:
            case ENOTDIR:
                return SCF_FILE_DOES_NOT_EXIST;
            default:
                return SCF_IO_ERROR;
                
        }
    }
    
    *result = SCF_ALLOC(op, scf_reader);
    (*result)->buf = scf_buffer_create(op, buffer_size);
    (*result)->fill = fill;
    (*result)->close = close_reader;
    (*result)->additional_data = fd;
    return SCF_SUCCESS;
}

scf_error_code scf_os_file_writer_create(scf_operation *op,
                                         const scf_string *path,
                                         size_t buffer_size,
                                         bool append,
                                         scf_writer **result) {
    
}

#endif
