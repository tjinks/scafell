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

static int hash(scf_dictionary *d, scf_datum key) {
    int h = d->hash_func(key);
    if (h < 0) {
        h = ~h;
    }
    
    return h % d->capacity;
}

typedef enum {
    MATCH, NO_MATCH, TRY_AGAIN
} match_status;

static match_status is_match(scf_dictionary *d, scf_datum key, bool inserting, int index, int collisions) {
    scf_dictionary_item item = d->items[index];
    bool found = d->comparison_func(item.key, key);
    if (inserting) {
        found = found || item.key.type == DT_NONE;
    } else {
        if (!found && collisions > d->max_collisions) {
            return NO_MATCH;
        }
    }
    
    return found ? MATCH : TRY_AGAIN;
}

static int lookup(scf_dictionary *d, scf_datum key, bool inserting) {
    int index = hash(d, key);
    
    int inc = 0;
    int collisions = 0;
    for (;;) {
        switch (is_match(d, key, inserting, index, collisions)) {
            case MATCH:
                return index;
            case NO_MATCH:
                return -1;
            case TRY_AGAIN:
                inc++;
                collisions++;
                index = (index + inc) % d->capacity;
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

static void rehash(scf_dictionary *d, int capacity) {
    SCF_OPERATION(rehashing);
    int size = d->size;
    scf_dictionary_item *copy = copy_items(&rehashing, d);
    
    d->items = scf_realloc(d->items, ITEM_SIZE * capacity);
    memset(d->items, 0, ITEM_SIZE * capacity);
    d->capacity = capacity;
    d->max_collisions = 0;

    for (int i = 0; i < size; i++) {
        int index = lookup(d, copy[i].key, true);
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
static int round_up(int n) {
    int result = 2;
    while (result < n) {
        result *= 2;
    }
    
    return result;
}

scf_dictionary scf_dictionary_create(
                                     scf_operation *operation,
                                     scf_hash_func hash_func,
                                     scf_comparison_func comparison_func,
                                     int initial_capacity) {
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
    int index = lookup(d, key, true);
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
    int index = lookup(d, key, false);
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
    int index = lookup((scf_dictionary *)d, key, false);
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

scf_set scf_set_create(scf_operation *operation, scf_hash_func hash_func, scf_comparison_func comparison_func, int initial_capacity) {
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

