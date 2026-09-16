//
//  abstract_io.h
//  scafell
//
//  Created by Tony on 14/09/2026.
//

#ifndef abstract_io_h
#define abstract_io_h

#include "mmgt.h"
#include "os/osdefs.h"
#include "err_handling.h"

typedef struct scf_reader {
    scf_buffer buf;
    size_t index;
    scf_err_info (*fill)(struct scf_reader *);
    scf_err_info (*close)(struct scf_reader *);
    void *additional_data;
} scf_reader;

typedef struct scf_writer {
    scf_buffer buf;
    size_t index;
    scf_err_info (*empty)(struct scf_writer *);
    scf_err_info (*close)(struct scf_writer *);
    void *additional_data;
} scf_writer;

inline scf_err_info scf_read_byte(scf_reader *reader, unsigned char *target) {
    scf_buffer *buf = &reader->buf;
    if (reader->index < buf->size) {
    have_data_in_buffer:
        *target = buf->data[reader->index++];
        return scf_success;
    } else {
        scf_err_info status = reader->fill(reader);
        if (scf_ok(status)) {
            goto have_data_in_buffer;
        } else {
            return status;
        }
    }
}

inline scf_err_info scf_write_byte(scf_writer *writer, unsigned char byte) {
    scf_buffer *buf = &writer->buf;
    if (buf->size < buf->capacity) {
    have_space_in_buffer:
        buf->data[buf->size++] = byte;
        return scf_success;
    } else {
        scf_err_info status = writer->empty(writer);
        if (scf_ok(status)) {
            goto have_space_in_buffer;
        } else {
            return status;
        }
    }
}

#endif /* abstract_io_h */
