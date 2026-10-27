//
//  unix_file_io.c
//  scafell
//
//  Created by Tony on 15/09/2026.
//

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "file_io.h"
#include "err_handling.h"
#include "str.h"
#include "scf_internal.h"

static const size_t default_buffer_size = 1024;

static bool fill(scf_os_file_reader *reader, scf_err_context *ec) {
    int fd = reader->handle;
    ssize_t read_count = read(fd, reader->buf.data, reader->buf.capacity);
    reader->index = 0;
    reader->buf.size = read_count;
    switch (read_count) {
        case 0:
            return false;
        case -1:
            scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
        default:
            return true;
    }
}

static void empty(scf_os_file_writer *writer, scf_err_context *ec) {
    if (writer->buf.size == 0) {
        return;
    }
    
    int fd = writer->handle;
    ssize_t write_count = write(fd, writer->buf.data, writer->buf.size);
    
    if (write_count == -1) {
        scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
    }
    
    if (write_count < writer->buf.size) {
        scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
    }
    
    scf_buffer_clear(&writer->buf);
}

static void cleanup_reader(void *p) {
    scf_os_file_reader *reader = p;
    SCF_TRY(ec) {
        scf_os_close_reader(reader, &ec);
    }
    SCF_CATCH { }
    SCF_END_TRY
}

static void cleanup_writer(void *p) {
    scf_os_file_writer *writer = p;
    SCF_TRY(ec) {
        scf_os_close_writer(writer, &ec);
    }
    SCF_CATCH { }
    SCF_END_TRY
}


void scf_os_close_reader(scf_os_file_reader *reader, scf_err_context *ec) {
    if (reader->is_open) {
        reader->is_open = false;
        int fd = reader->handle;
        if (close(fd) == 0) {
            return;
        }
        
        scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
    }
}

void scf_os_close_writer(scf_os_file_writer *writer, scf_err_context *ec) {
    if (writer->is_open) {
        writer->is_open = false;
        int fd = writer->handle;

        SCF_TRY(ec1) {
            empty(writer, &ec1);
        }
        SCF_CATCH {
            close(fd);
            scf_raise_error(ec1.err_info, ec);
        }
        SCF_END_TRY
        
        if (close(fd) == 0) {
            return;
        }
        
        scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
    }
}

scf_os_file_reader *scf_os_file_reader_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               scf_err_context *ec) {
    const char *path_as_cstr = scf_string_to_cstr(path);
    int fd = open(path_as_cstr, O_RDONLY);
    if (fd == -1) {
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
    
    scf_os_file_reader *result = scf_alloc_with_cleanup(op, sizeof(scf_os_file_reader), cleanup_reader);
    result->buf = scf_buffer_create(op, buffer_size ? buffer_size : default_buffer_size);
    result->handle = fd;
    result->index = 0;
    result->is_open = true;
    return result;
}

scf_os_file_writer *scf_os_file_writer_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               bool append,
                               scf_err_context *ec) {
    const char *path_as_cstr = scf_string_to_cstr(path);
    int mode = O_WRONLY;
    if (append) {
        mode |= O_APPEND;
    } else {
        mode |= O_CREAT;
    }
    
    int fd = open(path_as_cstr, mode, 0500);
    if (fd == -1) {
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
    
    scf_os_file_writer *result = scf_alloc_with_cleanup(op, sizeof(scf_os_file_writer), cleanup_writer);
    result->buf = scf_buffer_create(op, buffer_size ? buffer_size : default_buffer_size);
    result->handle = fd;
    result->is_open = true;
    return result;
}

scf_buffer scf_os_read_bytes(scf_operation *op, scf_os_file_reader *reader, size_t count, scf_err_context *ec) {
    op = op ? op : scf_get_operation(reader);
    scf_buffer result = scf_buffer_create(op, count);
    size_t xfr_count = 0;
    while (xfr_count < count) {
        size_t available = reader->buf.size - reader->index;
        if (available == 0) {
            fill(reader, ec);
            if (reader->buf.size == 0) {
                return result;
            } else {
                available = reader->buf.size;
            }
        }
    
        size_t required = count - xfr_count;
        if (required < available) {
            scf_buffer_append_bytes(&result, reader->buf.data + reader->index, required);
            xfr_count += required;
            reader->index += required;
        } else {
            scf_buffer_append_bytes(&result, reader->buf.data + reader->index, available);
            xfr_count += available;
            reader->index += available;
        }
    }
    
    return result;
}

void scf_os_read_with_callback(scf_os_file_reader *reader, scf_read_callback callback, void *additional_data, scf_err_context *ec) {
    if (reader->index > 0) {
        size_t initial_size = reader->buf.size - reader->index;
        if (initial_size > 0) {
            SCF_OPERATION(op);
            scf_buffer buf = scf_buffer_create(&op, initial_size);
            scf_buffer_append_bytes(&buf, reader->buf.data + reader->index, initial_size);
            if (!callback(&buf, additional_data, ec)) {
                scf_complete(&op);
                return;
            }
            
            scf_complete(&op);
            reader->index = reader->buf.size;
        }
    } else if (reader->buf.size) {
        if (!callback(&reader->buf, additional_data, ec)) {
            return;
        }
    }
        
    do {
        fill(reader, ec);
        if (!callback(&reader->buf, additional_data, ec)) {
            return;
        }
    } while (reader->buf.size);
}


void scf_os_write_bytes(scf_os_file_writer *writer, const unsigned char *bytes, size_t count, scf_err_context *ec) {
    size_t xfr_count = 0;
    while (xfr_count < count) {
        size_t space_in_buffer = writer->buf.capacity - writer->buf.size;
        if (space_in_buffer == 0) {
            empty(writer, ec);
            space_in_buffer = writer->buf.capacity;
        }
        
        size_t still_to_write = count - xfr_count;
        if (still_to_write < space_in_buffer) {
            scf_buffer_append_bytes(&writer->buf, bytes + xfr_count, still_to_write);
            xfr_count += still_to_write;
        } else {
            scf_buffer_append_bytes(&writer->buf, bytes + xfr_count, space_in_buffer);
            xfr_count += space_in_buffer;
        }
    }
}

scf_list scf_os_get_directory_contents(scf_operation *op, const scf_string *path, scf_err_context *ec) {
    const char *path_as_cstr;
    if (scf_is_absolute_path(path)) {
        path_as_cstr = scf_string_to_cstr(path);
    } else {
        scf_string *full_path = scf_current_dir(op);
        scf_path_append(full_path, path);
        path_as_cstr = scf_string_to_cstr(full_path);
    }
    
    DIR *reader = opendir(path_as_cstr);
    scf_free(path_as_cstr);
    if (!reader) {
        switch (errno) {
            case EACCES:
                scf_raise_error(scf_os_err_info_create(SCF_ACCESS_DENIED, errno), ec);
            case ENOENT:
                scf_raise_error(scf_os_err_info_create(SCF_FILE_DOES_NOT_EXIST, errno), ec);
            case ENOTDIR:
                scf_raise_error(scf_os_err_info_create(SCF_NOT_A_DIRECTORY, errno), ec);
            default:
                scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);

        }
    }
    
    scf_list result = scf_list_create(op, sizeof(scf_fs_entity_data), 10);
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(reader);
        if (!entry) {
            switch (errno) {
                case 0:
                    closedir(reader);
                    return result;
                default:
                    scf_raise_error(scf_os_err_info_create(SCF_IO_ERROR, errno), ec);
            }
        }
        
        scf_fs_entity_data data;
        data.name = scf_string_from_cstr(op, entry->d_name);
        
        if (scf_string_cmp(data.name, scf_dot) == 0) continue;
        if (scf_string_cmp(data.name, scf_dotdot) == 0) continue;

        switch (entry->d_type) {
            case DT_REG:
                data.type = SCF_FILE;
                break;
            case DT_LNK:
                data.type = SCF_LINK;
                break;
            case DT_DIR:
                data.type = SCF_DIRECTORY;
                break;
            default:
                data.type = SCF_OTHER;
                break;
        }
        
        scf_list_add(&result, &data);
    }
}

scf_string *scf_current_dir(scf_operation *op) {
    size_t buf_size = 10;
    do {
        char *buffer = malloc(buf_size);
        if (!buffer) {
            scf_raise_fatal_error(SCF_OUT_OF_MEMORY, "Out of memory!");
        }
        
        if (getcwd(buffer, buf_size)) {
            scf_string *result = scf_string_from_cstr(op, buffer);
            free(buffer);
            return result;
        }
        
        free(buffer);
        buf_size *= 2;
    } while (errno == ERANGE);
    
    scf_raise_fatal_os_error(SCF_IO_ERROR, errno);
}

bool scf_is_absolute_path(const scf_string *path) {
    scf_string_iterator iter = scf_string_start(path);
    scf_char first_char;
    if (scf_string_next(&iter, &first_char)) {
        return scf_char_cmp(first_char, scf_path_separator) == 0;
    }
        
    return false;
}

void scf_path_append(scf_string *base, const scf_string *appendee) {
    if (scf_is_absolute_path(appendee)) {
        scf_raise_fatal_error(SCF_INVALID_PATH, "Attempting to append an absolute path");
    }

    if (!scf_string_has_suffix(base, scf_path_separator_string)) {
        scf_string_append(base, scf_path_separator_string);
    }
    
    scf_string_append(base, appendee);
}


