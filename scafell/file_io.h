//
//  abstract_io.h
//  scafell
//
//  Created by Tony on 14/09/2026.
//

#ifndef abstract_io_h
#define abstract_io_h

#include "mmgt.h"
#include "str.h"
#include "os/osdefs.h"
#include "err_handling.h"
#include "list.h"

typedef struct {
    scf_buffer buf;
    size_t index;
    bool is_open;
    scf_os_file_handle handle;
} scf_os_file_reader;

typedef struct {
    scf_buffer buf;
    size_t index;
    bool is_open;
    scf_os_file_handle handle;
} scf_os_file_writer;

typedef enum {
    SCF_FILE,
    SCF_DIRECTORY,
    SCF_LINK,
    SCF_OTHER
} scf_fs_entity_type;

typedef struct {
    scf_fs_entity_id id;
    scf_string *name;
    scf_fs_entity_type type;
    scf_filesize_type size;
} scf_fs_entity_data;

scf_os_file_reader *scf_os_file_reader_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               scf_err_context *ec);

scf_os_file_writer *scf_os_file_writer_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               bool append,
                               scf_err_context *ec);

scf_list scf_os_get_directory_contents(scf_operation *op, const scf_string *path, scf_err_context *ec);

scf_buffer scf_os_read_bytes(scf_operation *op, scf_os_file_reader *reader, size_t count, scf_err_context *ec);

typedef bool (*scf_read_callback)(const scf_buffer *, void *, scf_err_context *);

void scf_os_read_with_callback(scf_os_file_reader *reader, scf_read_callback callback, void *additional_data, scf_err_context *ec);

void scf_os_write_bytes(scf_os_file_writer *writer, const unsigned char *bytes, size_t count, scf_err_context *ec);

void scf_os_close_reader(scf_os_file_reader *reader, scf_err_context *ec);

void scf_os_close_writer(scf_os_file_writer *writer, scf_err_context *ec);

inline bool scf_os_read_single_byte(scf_os_file_reader *reader, unsigned char *byte, scf_err_context *ec) {
    SCF_OPERATION(op);
    bool result = true;
    scf_buffer buf = scf_os_read_bytes(&op, reader, 1, ec);
    if (buf.size == 1) {
        *byte = buf.data[0];
    } else {
        result = false;
    }
    
    scf_complete(&op);
    return result;
}

inline void scf_os_write_single_byte(scf_os_file_writer *writer, unsigned char byte, scf_err_context *ec) {
    scf_os_write_bytes(writer, &byte, 1, ec);
}

#endif /* abstract_io_h */
