//
//  list_tests.c
//  ScafellTest
//
//  Created by Tony on 17/06/2025.
//

#include <stdio.h>
#include "scuts.h"
#include "list.h"

static int items[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

static scf_list list;
static SCF_OPERATION(op);

static bool check_item(int index, int expected) {
    int value;
    scf_list_get(&list, index, &value);
    return ASSERT_EQ(expected, value);
}

void list_init(void) {
    list = scf_list_create(&op, sizeof(int), 10);
}

void list_cleanup(void) {
    scf_complete(&op);
}

bool test_add(void) {
    for (int i = 0; i < 11; i++) {
        scf_list_add(&list, items + i);
    }
    
    bool result =
        ASSERT_EQ(11, list.size)
        && ASSERT_EQ(20, list.capacity);
    for (int i = 0; i < 11; i++) {
        result = result && check_item(i, i);
    }
    
    return result;
}

bool test_append(void) {
    for (int i = 0; i < 6; i++) {
        scf_list_add(&list, items + i);
    }

    scf_list list2 = scf_list_create(&op, sizeof(int), 10);
    for (int i = 6; i < 11; i++) {
        scf_list_add(&list2, items + i);
    }
    
    scf_list_append(&list, &list2);
    bool result =
        ASSERT_EQ(11, list.size)
        && ASSERT_EQ(20, list.capacity);
    for (int i = 0; i < 11; i++) {
        result = result && check_item(i, i);
    }
    
    return result;
}

bool test_insert_at_end(void) {
    for (int i = 0; i < 3; i++) {
        scf_list_add(&list, items + i);
    }

    int n = 99;
    scf_list_insert(&list, &n, 3);

    bool result =
    ASSERT_EQ(4, list.size)
    && check_item(0, 0)
    && check_item(1, 1)
    && check_item(2, 2)
    && check_item(3, 99);

    return result;
}

bool test_insert_in_middle(void) {
    for (int i = 0; i < 3; i++) {
        scf_list_add(&list, items + i);
    }

    int n = 99;
    scf_list_insert(&list, &n, 1);

    bool result =
    ASSERT_EQ(4, list.size)
    && check_item(0, 0)
    && check_item(1, 99)
    && check_item(2, 1)
    && check_item(3, 2);

    return result;
}


bool test_insert_at_start(void) {
    for (int i = 0; i < 3; i++) {
        scf_list_add(&list, items + i);
    }

    int n = 99;
    scf_list_insert(&list, &n, 0);

    bool result =
    ASSERT_EQ(4, list.size)
    && check_item(0, 99)
    && check_item(1, 0)
    && check_item(2, 1)
    && check_item(3, 2);

    return result;
}

bool test_remove_at_end(void) {
    for (int i = 0; i < 4; i++) {
        scf_list_add(&list, items + i);
    }

    scf_list_remove(&list, 3);

    bool result =
    ASSERT_EQ(3, list.size)
    && check_item(0, 0)
    && check_item(1, 1)
    && check_item(2, 2);

    return result;

}

bool test_remove_in_middle(void) {
    for (int i = 0; i < 4; i++) {
        scf_list_add(&list, items + i);
    }

    scf_list_remove(&list, 1);

    bool result =
    ASSERT_EQ(3, list.size)
    && check_item(0, 0)
    && check_item(1, 2)
    && check_item(2, 3);

    return result;

}

bool test_remove_at_start(void) {
    for (int i = 0; i < 4; i++) {
        scf_list_add(&list, items + i);
    }

    scf_list_remove(&list, 0);

    bool result =
    ASSERT_EQ(3, list.size)
    && check_item(0, 1)
    && check_item(1, 2)
    && check_item(2, 3);

    return result;
}

static int cmp(const void *item1, const void *item2) {
    int i1 = SCF_DEREF(int, item1);
    int i2 = SCF_DEREF(int, item2);
    return i1 - i2;
}

bool test_sort(void) {
    for (int i = 10; i >= 0; i--) {
        scf_list_add(&list, items + i);
    }
    
    scf_list_sort(&list, cmp);
    
    bool result = true;
    for (int i = 0; i <= 10; i++) {
        int item;
        scf_list_get(&list, i, &item);
        result = result && ASSERT_EQ(i, item);
    }
    
    return result;
}

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
    TEST(test_sort)
END_TEST_GROUP

