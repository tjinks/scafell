//
//  file_io.c
//  scafell
//
//  Created by Tony on 25/09/2026.
//

#include "file_io.h"

extern bool scf_os_read_single_byte(scf_os_file_reader *reader, unsigned char *byte, scf_err_context *ec);
 
extern void scf_os_write_single_byte(scf_os_file_writer *writer, unsigned char byte, scf_err_context *ec);

