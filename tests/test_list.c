/* tests/test_list.c */
#include "unity/unity.h"
#include <string.h>
#include <stdio.h>
#include "../src/app.h"
#include "../src/utils.h"

static void test_push_single(void)
{
    DeviceList list;
    device_list_init(&list);
    DeviceInfo di;
    memset(&di, 0, sizeof(di));
    safe_strcpy(di.ip, sizeof(di.ip), "192.168.1.1");
    di.is_alive = 1;
    device_list_push(&list, &di);
    TEST_ASSERT_EQUAL_size_t(1, list.count);
    TEST_ASSERT_EQUAL_STRING("192.168.1.1", list.items[0].ip);
    device_list_clear(&list);
}

static void test_push_hundred(void)
{
    DeviceList list;
    device_list_init(&list);
    for (int i = 0; i < 100; ++i) {
        DeviceInfo di;
        memset(&di, 0, sizeof(di));
        snprintf(di.ip, sizeof(di.ip), "10.0.0.%d", i);
        device_list_push(&list, &di);
    }
    TEST_ASSERT_EQUAL_size_t(100, list.count);
    device_list_clear(&list);
}

static void test_clear(void)
{
    DeviceList list;
    device_list_init(&list);
    DeviceInfo di;
    memset(&di, 0, sizeof(di));
    device_list_push(&list, &di);
    device_list_clear(&list);
    TEST_ASSERT_EQUAL_size_t(0, list.count);
    TEST_ASSERT_NULL(list.items);
}

void run_test_list(void)
{
    RUN_TEST(test_push_single);
    RUN_TEST(test_push_hundred);
    RUN_TEST(test_clear);
}
