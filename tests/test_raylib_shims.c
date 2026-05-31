/* tests/test_raylib_shims.c */
#include "unity/unity.h"
#include "../src/raylib_shims.h"
#include <stddef.h>

static void test_SaveFileText_null_filename(void)
{
    bool result = SaveFileText(NULL, "Some text");
    TEST_ASSERT_FALSE(result);
}

static void test_SaveFileText_null_text(void)
{
    bool result = SaveFileText("dummy.txt", NULL);
    TEST_ASSERT_FALSE(result);
}

static void test_SaveFileText_null_both(void)
{
    bool result = SaveFileText(NULL, NULL);
    TEST_ASSERT_FALSE(result);
}

void run_test_raylib_shims(void)
{
    RUN_TEST(test_SaveFileText_null_filename);
    RUN_TEST(test_SaveFileText_null_text);
    RUN_TEST(test_SaveFileText_null_both);
}
