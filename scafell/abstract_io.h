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

inline bool scf_read_single_byte(scf_reader *reader, unsigned char *byte, scf_err_context *ec) {
    scf_buffer *buf = &reader->buf;
    if (reader->index >= buf->size) {
        if (!reader->fill(reader, ec)) {
            // EOF
            *byte = '\0';
            return false;
        }
        
        reader->index = 0;
    }

    *byte = buf->data[reader->index++];
    return true;
}

inline void scf_write_single_byte(scf_writer *writer, unsigned char byte, scf_err_context *ec) {
    scf_buffer *buf = &writer->buf;
    if (buf->size == buf->capacity) {
        writer->empty(writer, ec);
        buf->size = 0;
    }

    buf->data[buf->size++] = byte;
}

inline void scf_flush(scf_writer *writer, scf_err_context *ec) {
    writer->empty(writer, ec);
}

scf_buffer scf_read_bytes(scf_reader *reader, size_t count, scf_err_context *ec);

typedef bool (*scf_read_callback)(const scf_buffer *, void *, scf_err_context *);

void scf_read_with_callback(scf_reader *reader, scf_read_callback callback, void *additional_data, scf_err_context *ec);

void scf_write_bytes(scf_writer *writer, const unsigned char *bytes, size_t count, scf_err_context *ec);

void scf_close_reader(scf_reader *reader, scf_err_context *ec);

void scf_close_writer(scf_writer *writer, scf_err_context *ec);

void scf_cleanup_reader(void *p);

void scf_cleanup_writer(void *p);

#endif /* abstract_io_h */
