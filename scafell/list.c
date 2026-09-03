//
//  list.c
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#include <string.h>
#include <stdbool.h>

#include "list.h"
#include "err_handling.h"

static void ensure_capacity(scf_list *list, size_t minimum_capacity) {
    if (list->capacity >= minimum_capacity) {
        return;
    }
    
    size_t new_capacity = 2 * list->capacity;
    if (new_capacity < minimum_capacity) {
        new_capacity = minimum_capacity;
    }
    
    list->items = scf_realloc(list->items, list->element_size * new_capacity);
    list->capacity = new_capacity;
}

scf_list scf_list_create(scf_operation *operation, size_t element_size, size_t initial_capacity) {
    char *items = scf_alloc(operation, element_size * initial_capacity);
    scf_list result = {0, element_size, initial_capacity, items};
    return result;
}

void scf_list_add(scf_list *list, void *new_item) {
    ensure_capacity(list, list->size + 1);
    size_t element_size = list->element_size;
    memcpy(list->items + list->size * element_size, new_item, element_size);
    list->size++;
}

void scf_list_append(scf_list *list1, const scf_list *list2) {
    size_t element_size = list1->element_size;
    if (element_size != list2->element_size) {
        scf_raise_error(SCF_LOGIC_ERROR, "Can't combine lists with different element size!");
    }
    
    ensure_capacity(list1, list1->size + list2->size);
    memcpy(list1->items + list1->size * element_size, list2->items, list2->size * element_size);
    list1->size += list2->size;
}

void scf_list_insert(scf_list *list, void *new_item, size_t before) {
    if (before < 0 || before > list->size) {
        scf_raise_error(SCF_BAD_INDEX, "Invalid index");
    }
    
    size_t items_to_shift = list->size - before;
    if (items_to_shift == 0) {
        scf_list_add(list, new_item);
        return;
    }
    
    ensure_capacity(list, list->size + 1);
    size_t element_size = list->element_size;
    memmove(list->items + (before + 1) * element_size, list->items + before * element_size, items_to_shift * element_size);
    memcpy(list->items + before * element_size, new_item, element_size);
    list->size++;
}

void scf_list_remove(scf_list *list, size_t index) {
    if (index < 0 || index >= list->size) {
        scf_raise_error(SCF_BAD_INDEX, "Invalid index");
    }
    
    size_t items_to_shift = (list->size - 1) - index;
    size_t element_size = list->element_size;
    if (items_to_shift > 0) {
        memmove(list->items + index * element_size, list->items + (index + 1) * element_size, items_to_shift * element_size);
    }
    
    list->size--;
}

void scf_push(scf_list *list, void *item) {
    scf_list_add(list, item);
}

bool scf_pop(scf_list *list, void *item) {
    if (list->size == 0) {
        return false;
    } else {
        scf_list_get(list, list->size - 1, item);
        list->size--;
        return true;
    }
}

void scf_list_get(const scf_list *list, size_t index, void *item) {
    if (index < 0 || index >= list->size) {
        scf_raise_error(SCF_BAD_INDEX, "Invalid index passed to scf_list_get");
    }
    
    memcpy(item, list->items + list->element_size * index, list->element_size);
}

void scf_list_clear(scf_list *list) {
    list->size = 0;
}

