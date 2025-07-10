//
//  hash.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "mmgt.h"
#include "list.h"

//#ifdef FALSE

#define ITEM_SIZE (sizeof(scf_dictionary_item))

static const double MIN_FREE_PERCENTAGE = 25;

static size_t hash(scf_dictionary *d, scf_datum key) {
    size_t h = d->hash_func(key);
    return h & (d->capacity - 1);
}

typedef enum {
    MATCH, NO_MATCH, TRY_AGAIN
} match_status;

static match_status is_match(scf_dictionary *d, scf_datum key, bool inserting, size_t index, int collisions) {
    scf_dictionary_item item = d->items[index];
    bool found = d->comparison_func(item.key, key);
    if (inserting) {
        found = found || item.key.type == DT_NONE;
        if (found && collisions > d->max_collisions) d->max_collisions = collisions;
    } else {
        if (!found && collisions > d->max_collisions) {
            return NO_MATCH;
        }
    }
    
    return found ? MATCH : TRY_AGAIN;
}

static size_t next_inc(scf_dictionary *d, int collision_count) {
    size_t mask = d->capacity - 1;
    size_t factor = mask >> 1;
    size_t n = (collision_count * factor) & mask;
    size_t a = n;
    size_t b = n + 1;
    if ((a & 1) == 0) {
        a = (a >> 1) & mask;
        b &= mask;
    } else {
        a &= mask;
        b = (b >> 1) & mask;
    }
    
    return (a * b) & mask;
}

static size_t lookup(scf_dictionary *d, scf_datum key, bool inserting) {
    size_t original_index = hash(d, key);
    
    int collisions = 0;
    size_t current_index = original_index;
    for (;;) {
        size_t inc;
        switch (is_match(d, key, inserting, current_index, collisions)) {
            case MATCH:
                return current_index;
            case NO_MATCH:
                return -1;
            case TRY_AGAIN:
                collisions++;
                inc = next_inc(d, collisions);
                current_index = (original_index + inc) & (d->capacity - 1);
        }
    }
}

static scf_dictionary_item *copy_items(scf_operation *operation, const scf_dictionary *d) {
    scf_dictionary_item *result = scf_alloc(operation, ITEM_SIZE * d->size);
    int j = 0;
    for (int i = 0; i < d->capacity; i++) {
        if (d->items[i].key.type != DT_NONE) {
            result[j++] = d->items[i];
        }
    }
    
    return result;
}

static void rehash(scf_dictionary *d, size_t capacity) {
    SCF_OPERATION(rehashing);
    size_t size = d->size;
    scf_dictionary_item *copy = copy_items(&rehashing, d);
    
    d->items = scf_realloc(d->items, ITEM_SIZE * capacity);
    memset(d->items, 0, ITEM_SIZE * capacity);
    d->capacity = capacity;
    d->max_collisions = 0;

    for (int i = 0; i < size; i++) {
        size_t index = lookup(d, copy[i].key, true);
        d->items[index].key = copy[i].key;
        d->items[index].value = copy[i].value;
    }
    
    scf_complete(&rehashing);
}

static void ensure_capacity(scf_dictionary *d) {
    double new_size = d->size + 1;
    double percent_free = 100.0 * (d->capacity - new_size) / d->capacity;
    if (percent_free < MIN_FREE_PERCENTAGE) {
        rehash(d, d->capacity * 2);
    }
}

/*
 * Find the smallest power of 2 > n.
 */
static size_t round_up(size_t n) {
    int result = 2;
    while (result < n) {
        result *= 2;
    }
    
    return result;
}

static const int MIN_CAPACITY = 16;

scf_dictionary scf_dictionary_create(
                                     scf_operation *operation,
                                     scf_hash_func hash_func,
                                     scf_comparison_func comparison_func,
                                     size_t initial_capacity) {
    if (initial_capacity < MIN_CAPACITY) initial_capacity = MIN_CAPACITY;
    initial_capacity = round_up(initial_capacity);
    scf_dictionary result;
    result.size = 0;
    result.capacity = initial_capacity;
    result.comparison_func = comparison_func;
    result.hash_func = hash_func;
    result.max_collisions = 0;
    result.items = scf_alloc(operation, ITEM_SIZE * initial_capacity);
    memset(result.items, 0, ITEM_SIZE * initial_capacity);
    return result;
}

scf_datum scf_dictionary_add(scf_dictionary *d, scf_datum key, scf_datum value) {
    ensure_capacity(d);
    size_t index = lookup(d, key, true);
    scf_dictionary_item *item = d->items + index;
    if (item->key.type == DT_NONE) {
        d->size++;
    }
    
    scf_datum original_value = item->value;
    item->key = key;
    item->value = value;

    return original_value;
}

scf_datum scf_dictionary_remove(scf_dictionary *d, scf_datum key) {
    size_t index = lookup(d, key, false);
    if (index == -1) {
        return dt_none();
    }
    
    scf_dictionary_item *item = d->items + index;
    scf_datum original_value = item->value;
    item->key = dt_none();
    item->value = dt_none();
    d->size--;
    return original_value;
}

scf_datum *scf_dictionary_lookup(const scf_dictionary *d, scf_datum key) {
    size_t index = lookup((scf_dictionary *)d, key, false);
    if (index == -1) {
        return NULL;
    } else {
        return &d->items[index].value;
    }
}

scf_list scf_dictionary_get_items(scf_operation *operation, const scf_dictionary *d) {
    scf_list result = scf_list_create(operation, d->size);
    scf_dictionary_item *items = copy_items(operation, d);
    for (int i = 0; i < d->size; i++) {
        scf_datum item = {DT_PTR, .p_value = items + i};
        scf_list_add(&result, item);
    }
    
    return result;
}

scf_set scf_set_create(scf_operation *operation, scf_hash_func hash_func, scf_comparison_func comparison_func, size_t initial_capacity) {
    scf_set result;
    result.dictionary = scf_dictionary_create(operation, hash_func, comparison_func, initial_capacity);
    return result;
}

bool scf_set_add(scf_set *s, scf_datum item) {
    scf_datum result = scf_dictionary_add(&s->dictionary, item, dt_true());
    return result.type == DT_NONE;
}

bool scf_set_remove(scf_set *s, scf_datum item) {
    scf_datum result = scf_dictionary_remove(&s->dictionary, item);
    return result.type != DT_NONE;
}

bool scf_set_contains(const scf_set *s, scf_datum item) {
    return scf_dictionary_lookup(&s->dictionary, item) != NULL;
}

