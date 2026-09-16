//
//  main.c
//  dirlist
//
//  Created by Tony on 12/09/2026.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "mmgt.h"
#include "str.h"

static SCF_OPERATION(op);

scf_stringlist *get_paths(const char *root) {
    scf_stringlist *pending = scf_stringlist_create(&op);
    scf_stringlist *result = scf_stringlist_create(&op);
    scf_stringlist_add_cstr(pending, root);
    while (scf_stringlist_size(pending)) {
        scf_string *path = scf_stringlist_pop(pending);
        scf_stringlist_push(result, path);
        const char *cpath = scf_string_to_cstr(path);
        struct stat statbuf;
        if (lstat(cpath, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                DIR *dir = opendir(cpath);
                while (dir) {
                    struct dirent *entry = readdir(dir);
                    if (entry == NULL) {
                        break;
                    }
                    
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                        continue;
                    }
                    
                    scf_string *new_path = scf_string_clone(NULL, path);
                    scf_string_append_ascii(new_path, '/');
                    scf_string_append_cstr(new_path, entry->d_name);
                    scf_stringlist_push(pending, new_path);
                }
            }
        }
    }
    
    return result;
}

void print_paths(scf_stringlist *paths) {
    scf_stringlist_sort(paths, NULL);
    for (size_t i = 0; i < scf_stringlist_size(paths); i++) {
        printf("%s\n", scf_string_to_cstr(scf_stringlist_get(paths, i)));
    }
}

int main(int argc, char **argv) {
    scf_stringlist *paths = get_paths("/Users/tony/Documents/XCode/Common/Scafell/scafell.xcodeproj");
    print_paths(paths);
    scf_complete(&op);
    return 0;
}
