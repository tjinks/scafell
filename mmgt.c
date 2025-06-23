//
//  mmgt.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdio.h>
#include <stddef.h>
#include "mmgt.h"

static const int HEADER_SIZE = offsetof(scf_mem_block, data);

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

static scf_mem_block *get_block(void *p) {
    char *cp = p;
    cp -= HEADER_SIZE;
    return (scf_mem_block *) cp;
}

static void add_block(scf_operation *operation, scf_mem_block *block) {
    block->operation = operation;
    block->next = operation->first;
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

scf_operation *scf_get_operation(void *p) {
    return get_block(p)->operation;
}
