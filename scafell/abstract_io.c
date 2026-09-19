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

extern bool scf_read_byte(scf_reader *reader, unsigned char *byte, scf_err_context *);

extern void scf_write_byte(scf_writer *writer, unsigned char byte, scf_err_context *);
