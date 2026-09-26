//
//  osunix.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef osunix_h
#define osunix_h

#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>

#define SCF_OS_UNIX

typedef int scf_os_error_code;

typedef int scf_os_file_handle;

typedef off_t scf_filesize_type;

typedef struct {
    dev_t device;
    ino_t inode;
} scf_fs_entity_id;

inline bool scf_entity_id_equals(scf_fs_entity_id id1, scf_fs_entity_id id2) {
    if (id1.inode != id2.inode) return false;
    if (id1.device != id2.device) return false;
    return true;
}

#endif /* osunix_h */
