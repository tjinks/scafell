//
//  mmgt.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "mmgt.h"
#include "err_handling.h"

static const int HEADER_SIZE = offsetof(scf_mem_block, data);
static const int MIN_CAPACITY = 4;

static void default_exhaustion_handler(void) {
    fprintf(stderr, "Out of memory!\n");
    exit(1);
}

scf_exhaustion_handler exhaustion_handler = default_exhaustion_handler;

static void *alloc_raw(void *original, size_t required) {
    void *result = realloc(original, required);
    if (result) {
        return result;
    }
    
    exhaustion_handler();
    abort();
}

static scf_mem_block *get_block(const void *p) {
    char *cp = (char *)p;
    cp -= HEADER_SIZE;
    return (scf_mem_block *) cp;
}

static void add_block(scf_operation *operation, scf_mem_block *block) {
    block->operation = operation;
    block->next = operation->first;
    block->cleanup = NULL;
    operation->first = block;
}

static void remove_block(scf_mem_block *block) {
    scf_operation *operation = block->operation;
    if (block == operation->first) {
        operation->first = block->next;
        return;
    }
    
    for (scf_mem_block *current = operation->first; current; current = current->next) {
        if (current->next == block) {
            current->next = block->next;
            return;
        }
    }
}

void *scf_alloc(scf_operation *operation, size_t required) {
    required += HEADER_SIZE;
    scf_mem_block *block = alloc_raw(NULL, required);
    add_block(operation, block);
    return block->data;
}

void *scf_alloc_with_cleanup(scf_operation *operation, scf_cleanup_func cleanup, size_t required) {
    void *result = scf_alloc(operation, required);
    get_block(result)->cleanup = cleanup;
    return result;
}

void *scf_realloc(void *p, size_t required) {
    scf_mem_block *original_block = get_block(p);
    scf_operation *operation = original_block->operation;
    remove_block(original_block);
    scf_mem_block *new_block = alloc_raw(original_block, required + HEADER_SIZE);
    add_block(operation, new_block);
    return new_block->data;
}

void scf_complete(scf_operation *operation) {
    scf_mem_block *block;
    for (block = operation->first; block; block = block->next) {
        if (block->cleanup) {
            block->cleanup(block->data);
        }
    }

    block = operation->first;
    while (block) {
        scf_mem_block *next = block->next;
        free(block);
        block = next;
    }
    
    operation->first = NULL;
}

scf_operation *scf_get_operation(const void *p) {
    return get_block(p)->operation;
}

static void ensure_capacity(scf_buffer *buffer, size_t required) {
    if (buffer->capacity < required) {
        size_t new_capacity = 2 * buffer->capacity;
        if (new_capacity < required) new_capacity = required;
        buffer->data = scf_realloc(buffer->data, new_capacity);
        buffer->capacity = new_capacity;
    }
}

scf_buffer scf_buffer_create(scf_operation *operation, size_t initial_capacity) {
    if (initial_capacity < MIN_CAPACITY) initial_capacity = MIN_CAPACITY;
    scf_buffer result = {0, initial_capacity, scf_alloc(operation, initial_capacity)};
    return result;
}

void scf_buffer_append_bytes(scf_buffer *buffer, const void *bytes_to_append, size_t byte_count) {
    ensure_capacity(buffer, buffer->size + byte_count);
    memcpy(buffer->data + buffer->size, bytes_to_append, byte_count);
    buffer->size += byte_count;
}

void scf_buffer_insert_bytes(scf_buffer *buffer, const void *bytes_to_insert, size_t before, size_t byte_count) {
    if (before > buffer->size) scf_fatal_error("Invalid index");

    if (byte_count == 0) return;
    
    ensure_capacity(buffer, buffer->size + byte_count);
    memmove(buffer->data + before + byte_count, buffer->data + before, buffer->size - before);
    memcpy(buffer->data + before, bytes_to_insert, byte_count);
    buffer->size += byte_count;
}

void scf_buffer_remove(scf_buffer *buffer, size_t starting_from, size_t byte_count) {
    if (starting_from + byte_count > buffer->size) scf_fatal_error("Attempting to remove more data than is present!");

    if (byte_count == 0) return;
    
    memmove(buffer->data + starting_from, buffer->data + starting_from + byte_count, buffer->size - (starting_from + byte_count));
    buffer->size -= byte_count;
}

scf_buffer scf_buffer_extract(const scf_buffer *buffer, size_t starting_from, size_t byte_count) {
    if (starting_from + byte_count > buffer->size) scf_fatal_error("Attempting to extract more data than is present!");

    scf_buffer result = scf_buffer_create(scf_get_operation(buffer->data), byte_count);
    memcpy(result.data, buffer->data + starting_from, byte_count);
    result.size = byte_count;
    return result;
}

extern void scf_buffer_append(scf_buffer *buf1, const scf_buffer *buf2);

extern void scf_buffer_insert(scf_buffer *buf1, scf_buffer *buf2, size_t before);



