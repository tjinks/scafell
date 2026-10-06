//
//  file_io.c
//  scafell
//
//  Created by Tony on 25/09/2026.
//

#include "scf_internal.h"
#include "file_io.h"

#if defined(SCF_OS_UNIX)
#define SEPARATOR ('/')
#endif

#if defined(SCF_OS_WINDOWS)
#define SEPARATOR ('\\')
#endif

const scf_char scf_path_separator = {SCF_UTF8, 1, {SEPARATOR}};

const scf_string *scf_path_separator_string;
const scf_string *scf_dot;
const scf_string *scf_dotdot;


extern bool scf_os_read_single_byte(scf_os_file_reader *reader, unsigned char *byte, scf_err_context *ec);
 
extern void scf_os_write_single_byte(scf_os_file_writer *writer, unsigned char byte, scf_err_context *ec);

void scf_file_io_init(scf_operation *op) {
    scf_string *path_separator_string = scf_string_create(op);
    scf_string_append_char(path_separator_string, scf_ascii(SEPARATOR));
    scf_path_separator_string = path_separator_string;
    
    scf_dot = scf_string_from_cstr(op, ".");
    scf_dotdot = scf_string_from_cstr(op, "..");
}


