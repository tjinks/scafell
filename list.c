//
//  list.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <string.h>
#include "list.h"
#include "err_handling.h"

static void ensure_capacity(scf_list *list, int minimum_capacity) {
    if (list->capacity >= minimum_capacity) {
        return;
    }
    
    int new_capacity = 2 * list->capacity;
    if (new_capacity < minimum_capacity) {
        new_capacity = minimum_capacity;
    }
    
    list->items = scf_realloc(list->items, SCF_DATUM_SIZE * new_capacity);
    list->capacity = new_capacity;
}

scf_list scf_list_create(scf_operation *operation, int initial_capacity) {
    scf_datum *items = scf_alloc(operation, SCF_DATUM_SIZE * initial_capacity);
    scf_list result = {0, initial_capacity, items};
    return result;
}

void scf_list_add(scf_list *list, scf_datum new_item) {
    ensure_capacity(list, list->size + 1);
    list->items[list->size++] = new_item;
}

void scf_list_append(scf_list *list1, const scf_list *list2) {
    ensure_capacity(list1, list1->size + list2->size);
    memcpy(list1->items + list1->size, list2->items, SCF_DATUM_SIZE * list2->size);
    list1->size += list2->size;
}

void scf_list_insert(scf_list *list, scf_datum new_item, int before) {
    if (before < 0 || before > list->size) {
        SCF_FATAL_ERROR("Invalid index");
    }
    
    int items_to_shift = list->size - before;
    if (items_to_shift == 0) {
        scf_list_add(list, new_item);
        return;
    }
    
    ensure_capacity(list, list->size + 1);
    memmove(&list->items[before + 1], &list->items[before], SCF_DATUM_SIZE * items_to_shift);
    list->items[before] = new_item;
    list->size++;
}

void scf_list_remove(scf_list *list, int index) {
    if (index < 0 || index >= list->size) {
        SCF_FATAL_ERROR("Invalid index");
    }
    
    int items_to_shift = (list->size - 1) - index;
    if (items_to_shift > 0) {
        memmove(&list->items[index], &list->items[index + 1], SCF_DATUM_SIZE * items_to_shift);
    }
    
    list->size--;
}

void scf_push(scf_list *list, scf_datum item) {
    scf_list_add(list, item);
}

scf_datum scf_pop(scf_list *list) {
    if (list->size == 0) {
        return dt_none();
    } else {
        list->size--;
        return list->items[list->size];
    }
}

void scf_list_clear(scf_list *list) {
    list->size = 0;
}

bool scf_list_for_each(scf_list *list, scf_for_each_func callback, void *iteration_context) {
    for (int i = 0; i < list->size; i++) {
        if (!callback(list->items + i, iteration_context)) {
            return false;
        }
    }
    
    return true;
}

