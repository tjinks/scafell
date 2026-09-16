//
//  unix_file_io.h
//  scafell
//
//  Created by Tony on 15/09/2026.
//

#ifndef unix_file_io_h
#define unix_file_io_h

#include <fcntl.h>
#include "../err_handling.h"
#include "../mmgt.h"
#include "../str.h"
#include "../abstract_io.h"
#include "osdefs.h"

scf_err_info scf_os_file_reader_create(scf_operation *op,
                                         const scf_string *path,
                                         size_t buffer_size,
                                         scf_reader **result);

scf_err_info scf_os_file_writer_create(scf_operation *op,
                                         const scf_string *path,
                                         size_t buffer_size,
                                         bool append,
                                         scf_writer **result);


#endif /* unix_file_io_h */
