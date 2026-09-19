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
    bool is_open;
    bool (*fill)(struct scf_reader *, scf_err_context *ec);
    void (*close)(struct scf_reader *, scf_err_context *ec);
    void *additional_data;
} scf_reader;

typedef struct scf_writer {
    scf_buffer buf;
    size_t index;
    bool is_open;
    void (*empty)(struct scf_writer *, scf_err_context *ec);
    void (*close)(struct scf_writer *, scf_err_context *ec);
    void *additional_data;
} scf_writer;

inline bool scf_read_byte(scf_reader *reader, unsigned char *byte, scf_err_context *ec) {
    scf_buffer *buf = &reader->buf;
    if (reader->index >= buf->size) {
        if (!reader->fill(reader, ec)) {
            // EOF
            *byte = '\0';
            return false;
        }
    }

    *byte = buf->data[reader->index++];
    return true;
}

inline void scf_write_byte(scf_writer *writer, unsigned char byte, scf_err_context *ec) {
    scf_buffer *buf = &writer->buf;
    if (buf->size == buf->capacity) {
        writer->empty(writer, ec);
    }

    buf->data[buf->size++] = byte;
}

void scf_close_reader(scf_reader *reader, scf_err_context *ec);

void scf_close_writer(scf_writer *writer, scf_err_context *ec);

void scf_cleanup_reader(void *p);

void scf_cleanup_writer(void *p);

#endif /* abstract_io_h */
