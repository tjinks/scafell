//
//  mmgt.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef mmgt_h
#define mmgt_h

#include <stdlib.h>

typedef void (*scf_exhaustion_handler)(void);
typedef void (*scf_cleanup_func)(void *);

extern scf_exhaustion_handler exhaustion_handler;

struct scf_operation;

typedef struct scf_mem_block {
    struct scf_operation *operation;
    struct scf_mem_block *next;
    scf_cleanup_func cleanup;
    char data[1];
} scf_mem_block;

typedef struct scf_operation {
    struct scf_mem_block *first;
} scf_operation;

#define SCF_OPERATION(name) scf_operation name = {NULL}

typedef struct {
    size_t size;
    size_t capacity;
    unsigned char *data;
} scf_buffer;

void *scf_alloc(scf_operation *operation, size_t required);
void *scf_alloc_with_cleanup(scf_operation *operation, scf_cleanup_func cleanup, size_t required);
void *scf_realloc(void *p, size_t required);
void scf_complete(scf_operation *operation);
scf_operation *scf_get_operation(const void *p);

scf_buffer scf_buffer_create(scf_operation *operation, size_t initial_capacity);
void scf_buffer_append_bytes(scf_buffer *buffer, const void *bytes_to_append, size_t byte_count);
void scf_buffer_insert_bytes(scf_buffer *buffer, const void *bytes_to_insert, size_t before, size_t byte_count);
void scf_buffer_remove(scf_buffer *buffer, size_t starting_from, size_t byte_count);
scf_buffer scf_buffer_extract(const scf_buffer *buffer, size_t starting_from, size_t byte_count);

inline void scf_buffer_append(scf_buffer *buf1, const scf_buffer *buf2) {
    scf_buffer_append_bytes(buf1, buf2->data, buf2->size);
}

inline void scf_buffer_insert(scf_buffer *buf1, scf_buffer *buf2, size_t before) {
    scf_buffer_insert_bytes(buf1, buf2->data, before, buf2->size);
}

#endif /* mmgt_h */
