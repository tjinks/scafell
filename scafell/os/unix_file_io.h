//
//  unix_file_io.h
//  scafell
//
//  Created by Tony on 15/09/2026.
//

#ifndef unix_file_io_h
#define unix_file_io_h

#include <fcntl.h>
#include "err_handling.h"
#include "mmgt.h"
#include "str.h"
#include "abstract_io.h"
#include "osdefs.h"

scf_reader *scf_os_file_reader_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               scf_err_context *ec);

scf_writer *scf_os_file_writer_create(scf_operation *op,
                               const scf_string *path,
                               size_t buffer_size,
                               bool append,
                               scf_err_context *ec);


#endif /* unix_file_io_h */
