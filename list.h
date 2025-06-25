//
//  list.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef list_h
#define list_h

#include <stdlib.h>
#include "datum.h"
#include "mmgt.h"

typedef struct {
    size_t size;
    size_t capacity;
    scf_datum *items;
} scf_list;

scf_list scf_list_create(scf_operation *operation, size_t initial_capacity);

void scf_list_add(scf_list *list, scf_datum new_item);

void scf_list_append(scf_list *list1, const scf_list *list2);

void scf_list_insert(scf_list *list, scf_datum new_item, size_t before);

void scf_list_remove(scf_list *list, size_t index);

void scf_push(scf_list *list, scf_datum item);

scf_datum scf_pop(scf_list *list);

void scf_list_clear(scf_list *list);

typedef bool (*scf_for_each_func)(scf_datum *datum, void *iteration_context);

bool scf_list_for_each(scf_list *list, scf_for_each_func callback, void *iteration_context);

#endif /* list_h */
