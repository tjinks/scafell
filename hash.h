//
//  hash.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef hash_h
#define hash_h

#include <stdbool.h>
#include "datum.h"
#include "mmgt.h"
#include "list.h"

typedef int (*scf_hash_func)(scf_datum key);
typedef bool (*scf_comparison_func)(scf_datum k1, scf_datum k2);

typedef struct {
    scf_datum key;
    scf_datum value;
} scf_dictionary_item;

typedef struct {
    scf_hash_func hash_func;
    scf_comparison_func comparison_func;
    int size;
    int capacity;
    int max_collisions;
    scf_dictionary_item *items;
} scf_dictionary;

typedef struct {
    scf_dictionary dictionary;
} scf_set;

scf_dictionary scf_dictionary_create(scf_operation *operation, scf_hash_func, scf_comparison_func, int initial_capacity);

scf_datum scf_dictionary_add(scf_dictionary *, scf_datum key, scf_datum value);

scf_datum scf_dictionary_remove(scf_dictionary *, scf_datum key);

scf_datum *scf_dictionary_lookup(const scf_dictionary *, scf_datum key);

scf_list scf_dictionary_get_items(scf_operation *operation, const scf_dictionary *);

scf_set scf_set_create(scf_operation *operation, scf_hash_func, scf_comparison_func, int initial_capacity);

bool scf_set_add(scf_set *s, scf_datum item);

bool scf_set_remove(scf_set *s, scf_datum item);

bool scf_set_contains(const scf_set *s, scf_datum item);



#endif /* hash_h */
