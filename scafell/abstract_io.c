//
//  abstract_io.c
//  scafell
//
//  Created by Tony on 14/09/2026.
//

#include "abstract_io.h"

void scf_close_reader(scf_reader *reader, scf_err_context *ec) {
    if (!reader->is_open) {
        return;
    }
    
    SCF_TRY(inner_ec) {
        reader->close(reader, &inner_ec);
    }
    SCF_CATCH {
        reader->is_open = false;
        scf_raise_error(inner_ec.err_info, ec);
    }
    SCF_END_TRY
    
    reader->is_open = false;
}

void scf_close_writer(scf_writer *writer, scf_err_context *ec) {
    if (!writer->is_open) {
        return;
    }
    
    SCF_TRY(inner_ec) {
        writer->close(writer, &inner_ec);
    }
    SCF_CATCH {
        writer->is_open = false;
        scf_raise_error(inner_ec.err_info, ec);
    }
    SCF_END_TRY
    
    writer->is_open = false;
}

void scf_cleanup_reader(void *p) {
    scf_reader *reader = p;
    SCF_TRY(ec) {
        scf_close_reader(reader, &ec);
    }
    SCF_CATCH { }
    SCF_END_TRY
}

void scf_cleanup_writer(void *p) {
    scf_writer *writer = p;
    SCF_TRY(ec) {
        scf_close_writer(writer, &ec);
    }
    SCF_CATCH { }
    SCF_END_TRY
}

scf_buffer scf_read_bytes(scf_reader *reader, size_t count, scf_err_context *ec) {
    scf_operation *op = scf_get_operation(reader);
    scf_buffer result = scf_buffer_create(op, count);
    size_t xfr_count = 0;
    while (xfr_count < count) {
        size_t available = reader->buf.size - reader->index;
        if (available == 0) {
            reader->fill(reader, ec);
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

void scf_read_with_callback(scf_reader *reader, scf_read_callback callback, void *additional_data, scf_err_context *ec) {
    if (reader->index > 0) {
        size_t initial_size = reader->buf.size - reader->index;
        if (initial_size > 0) {
            SCF_OPERATION(op);
            scf_buffer buf = scf_buffer_create(&op, initial_size);
            scf_buffer_append_bytes(&buf, reader->buf.data + reader->index, initial_size);
            if (!callback(&buf, additional_data, ec)) {
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
        reader->fill(reader, ec);
        if (!callback(&reader->buf, additional_data, ec)) {
            return;
        }
    } while (reader->buf.size);
}


void scf_write_bytes(scf_writer *writer, const unsigned char *bytes, size_t count, scf_err_context *ec) {
    size_t xfr_count = 0;
    while (xfr_count < count) {
        size_t space_in_buffer = writer->buf.capacity - writer->buf.size;
        if (space_in_buffer == 0) {
            writer->empty(writer, ec);
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


extern bool scf_read_single_byte(scf_reader *reader, unsigned char *byte, scf_err_context *);

extern void scf_write_single_byte(scf_writer *writer, unsigned char byte, scf_err_context *);

extern void scf_flush(scf_writer *writer, scf_err_context *ec);
