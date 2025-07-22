#ifndef fs_h
#define fs_h

#include <stdbool.h>
#include "os/osdefs.h"
#include "mmgt.h"
#include "err_handling.h"
#include "str.h"

typedef enum {
	SCF_IS_READABLE = 1,
	SCF_IS_WRITABLE = 2,
	SCF_IS_TRAVERSABLE = 4,
	SCF_IS_LINK = 8
} scf_io_capabilities;

struct scf_io_object;
struct scf_io_traverser;

typedef scf_err_info(*SCF_EXTERNAL scf_readfunc)(const scf_io_object*, scf_buffer*, size_t);
typedef scf_err_info(*SCF_EXTERNAL scf_writefunc)(const scf_io_object*, const scf_buffer*);

typedef struct scf_io_object {
	int type;
	int capabilities;
	scf_file_size size;
	void* object_info;
	scf_readfunc read;
	scf_writefunc write;
} scf_io_object;

scf_err_info scf_read(scf_io_object source, scf_buffer *target, size_t count);

scf_err_info scf_write(scf_io_object target, const scf_buffer* source);

scf_err_info 

#endif
