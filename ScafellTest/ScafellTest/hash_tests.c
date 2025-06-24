//
//  hash_tests.c
//  ScafellTest
//
//  Created by Tony on 18/06/2025.
//


//
//  mmgt_tests.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include <stdio.h>
#include <scuts.h>
#include "hash.h"

static SCF_OPERATION(op);
static scf_dictionary dict;

static int hash(scf_datum key) {
    return key.i_value % 4;
}

static bool cmp(scf_datum k1, scf_datum k2) {
    if (k1.type == DT_INT && k2.type == DT_INT) {
        return k1.i_value == k2.i_value;
    } else {
        return false;
    }
}

void hash_tests_init(void) {
    dict = scf_dictionary_create(&op, hash, cmp, 13);
}

void hash_tests_cleanup(void *p) {
    scf_complete(&op);
}

BEGIN_TEST_GROUP(hash_tests)
    INIT(hash_tests_init)
    CLEANUP(hash_tests_cleanup)
    TEST(test_initial_size)
    TEST(test_add_and_retrieve)
    TEST(test_add_existing)
    TEST(test_key_not_found)
    TEST(test_dictionary_remove)
    TEST(test_dictionary_remove_not_present)
    TEST(test_dictionary_get_items)
    TEST(test_collisions)
END_TEST_GROUP

bool test_initial_size(void) {
    return ASSERT_EQ(16, dict.capacity);
}

bool test_add_and_retrieve(void) {
    for (int i = 0; i < 13; i++) {
        scf_datum key = dt_int(i);
        scf_datum value = dt_int(2 * i);
        scf_dictionary_add(&dict, key, value);
    }
    
    bool result = ASSERT_EQ(32, dict.capacity) && ASSERT_EQ(13, dict.size);
    
    for (int i = 0; i < 13; i++) {
        scf_datum key = dt_int(i);
        scf_datum *value = scf_dictionary_lookup(&dict, key);
        result &= ASSERT_TRUE(value != NULL) && ASSERT_TRUE(value->i_value == 2 * i);
    }
    
    return result;
}

bool test_add_existing(void) {
    for (int i = 0; i < 13; i++) {
        scf_datum key = dt_int(i);
        scf_datum value = dt_int(2 * i);
        scf_dictionary_add(&dict, key, value);
    }
    
    scf_datum key = dt_int(7);
    scf_datum value = dt_int(99);
    scf_dictionary_add(&dict, key, value);
    scf_datum *retrieved = scf_dictionary_lookup(&dict, key);
    return ASSERT_FALSE(retrieved == NULL)
        && ASSERT_EQ(DT_INT, retrieved->type) && ASSERT_EQ(99, retrieved->i_value)
        && ASSERT_EQ(13, dict.size);
}

bool test_key_not_found(void) {
    for (int i = 0; i < 13; i++) {
        scf_datum key = dt_int(i);
        scf_datum value = dt_int(2 * i);
        scf_dictionary_add(&dict, key, value);
    }
    
    scf_datum key = dt_int(13);
    scf_datum *value = scf_dictionary_lookup(&dict, key);
    return ASSERT_TRUE(value == NULL);
}

bool test_dictionary_remove(void) {
    for (int i = 0; i < 13; i++) {
        scf_datum key = dt_int(i);
        scf_datum value = dt_int(2 * i);
        scf_dictionary_add(&dict, key, value);
    }
    
    scf_datum key = dt_int(7);
    scf_datum removed = scf_dictionary_remove(&dict, key);
    return ASSERT_EQ(14, removed.i_value) && ASSERT_EQ(12, dict.size);
}

bool test_dictionary_remove_not_present(void) {
    for (int i = 0; i < 13; i++) {
        scf_datum key = dt_int(i);
        scf_datum value = dt_int(2 * i);
        scf_dictionary_add(&dict, key, value);
    }
    
    scf_datum key = dt_int(99);
    scf_datum removed = scf_dictionary_remove(&dict, key);
    return ASSERT_EQ(DT_NONE, removed.type) && ASSERT_EQ(13, dict.size);
}

static bool contains_item(const scf_list list, int key, int value) {
    for (int i = 0; i < list.size; i++) {
        scf_dictionary_item *item = list.items[i].p_value;
        if (item->key.i_value == key && item->value.i_value == value) {
            return true;
        }
    }
    
    return false;
}

bool test_dictionary_get_items(void) {
    for (int i = 0; i < 13; i++) {
        scf_datum key = dt_int(i);
        scf_datum value = dt_int(2 * i);
        scf_dictionary_add(&dict, key, value);
    }
    
    scf_list result = scf_dictionary_get_items(&op, &dict);
    ASSERT_EQ(13, result.size);
    for (int i = 0; i < 13; i++) {
        if (!ASSERT_TRUE(contains_item(result, i, i * 2))) return false;
    }
    
    return true;
}

static int identity_hash(scf_datum key) {
    return 0;
}

bool test_collisions(void) {
    dict = scf_dictionary_create(&op, identity_hash, cmp, 32);
    for (int i = 0; i < 17; i++) {
        scf_dictionary_add(&dict, dt_int(i), dt_int(i));
    }
    
    return ASSERT_EQ(16, dict.max_collisions);
}

