//
//  list_tests.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include <stdio.h>
#include <scuts.h>
#include "list.h"
#include "datum.h"

static scf_list list;
static SCF_OPERATION(op);

BEGIN_TEST_GROUP(list_tests)
    INIT(list_init)
    CLEANUP(list_cleanup)
    TEST(test_add)
    TEST(test_append)
    TEST(test_insert_at_end)
    TEST(test_insert_in_middle)
    TEST(test_insert_at_start)
    TEST(test_remove_at_end)
    TEST(test_remove_in_middle)
    TEST(test_remove_at_start)
    TEST(test_for_each)
END_TEST_GROUP

void list_init(void) {
    list = scf_list_create(&op, 10);
}

void list_cleanup(void) {
    scf_complete(&op);
}

bool test_add(void) {
    for (int i = 0; i < 11; i++) {
        scf_list_add(&list, dt_int(i));
    }
    
    bool result =
        ASSERT_EQ(11, list.size)
        && ASSERT_EQ(20, list.capacity);
    for (int i = 0; i < 11; i++) {
        result = result && ASSERT_EQ(i, list.items[i].i_value);
    }
    
    return result;
}

bool test_append(void) {
    for (int i = 0; i < 6; i++) {
        scf_list_add(&list, dt_int(i));
    }

    scf_list list2 = scf_list_create(&op, 10);
    for (int i = 6; i < 11; i++) {
        scf_list_add(&list2, dt_int(i));
    }
    
    scf_list_append(&list, &list2);
    bool result =
        ASSERT_EQ(11, list.size)
        && ASSERT_EQ(20, list.capacity);
    for (int i = 0; i < 11; i++) {
        result = result && ASSERT_EQ(i, list.items[i].i_value);
    }
    
    return result;
}

bool test_insert_at_end(void) {
    for (int i = 0; i < 3; i++) {
        scf_list_add(&list, dt_int(i));
    }

    scf_list_insert(&list, dt_int(99), 3);

    bool result =
    ASSERT_EQ(4, list.size)
    && ASSERT_EQ(0, list.items[0].i_value)
    && ASSERT_EQ(1, list.items[1].i_value)
    && ASSERT_EQ(2, list.items[2].i_value)
    && ASSERT_EQ(99, list.items[3].i_value);

    return result;
}

bool test_insert_in_middle(void) {
    for (int i = 0; i < 3; i++) {
        scf_list_add(&list, dt_int(i));
    }

    scf_list_insert(&list, dt_int(99), 1);

    bool result =
    ASSERT_EQ(4, list.size)
    && ASSERT_EQ(0, list.items[0].i_value)
    && ASSERT_EQ(99, list.items[1].i_value)
    && ASSERT_EQ(1, list.items[2].i_value)
    && ASSERT_EQ(2, list.items[3].i_value);

    return result;
}


bool test_insert_at_start(void) {
    for (int i = 0; i < 3; i++) {
        scf_list_add(&list, dt_int(i));
    }

    scf_list_insert(&list, dt_int(99), 0);

    bool result =
    ASSERT_EQ(4, list.size)
    && ASSERT_EQ(99, list.items[0].i_value)
    && ASSERT_EQ(0, list.items[1].i_value)
    && ASSERT_EQ(1, list.items[2].i_value)
    && ASSERT_EQ(2, list.items[3].i_value);

    return result;
}

bool test_remove_at_end(void) {
    for (int i = 0; i < 4; i++) {
        scf_list_add(&list, dt_int(i));
    }

    scf_list_remove(&list, 3);

    bool result =
    ASSERT_EQ(3, list.size)
    && ASSERT_EQ(0, list.items[0].i_value)
    && ASSERT_EQ(1, list.items[1].i_value)
    && ASSERT_EQ(2, list.items[2].i_value);

    return result;

}

bool test_remove_in_middle(void) {
    for (int i = 0; i < 4; i++) {
        scf_list_add(&list, dt_int(i));
    }

    scf_list_remove(&list, 1);

    bool result =
    ASSERT_EQ(3, list.size)
    && ASSERT_EQ(0, list.items[0].i_value)
    && ASSERT_EQ(2, list.items[1].i_value)
    && ASSERT_EQ(3, list.items[2].i_value);

    return result;

}

bool test_remove_at_start(void) {
    for (int i = 0; i < 4; i++) {
        scf_list_add(&list, dt_int(i));
    }

    scf_list_remove(&list, 0);

    bool result =
    ASSERT_EQ(3, list.size)
    && ASSERT_EQ(1, list.items[0].i_value)
    && ASSERT_EQ(2, list.items[1].i_value)
    && ASSERT_EQ(3, list.items[2].i_value);

    return result;

}

static bool callback(scf_datum *datum, void *ctx) {
    ASSERT_EQ(DT_INT, datum->type);
    int *sum = (int *)ctx;
    *sum += datum->i_value;
    return true;
}

bool test_for_each(void) {
    for (int i = 0; i < 4; i++) {
        scf_list_add(&list, dt_int(i));
    }
    
    int sum = 0;
    scf_list_for_each(&list, callback, &sum);
    
    return ASSERT_EQ(6, sum);
}
