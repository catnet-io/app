/* tests/test_raylib_shims.c */
#include "unity/unity.h"
#include "../src/raylib_shims.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

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

static void test_mem_alloc_free(void)
{
    void* ptr = MemAlloc(100);
    TEST_ASSERT_NOT_NULL(ptr);
    /* Should be zero initialized due to calloc */
    char zeros[100] = {0};
    TEST_ASSERT_EQUAL_MEMORY(zeros, ptr, 100);

    ptr = MemRealloc(ptr, 200);
    TEST_ASSERT_NOT_NULL(ptr);

    MemFree(ptr);
}

static void test_file_data_save_load(void)
{
    const char* filename = "test_data.bin";
    unsigned char data[] = { 0x01, 0x02, 0x03, 0x00, 0xFF };
    int size = sizeof(data);

    bool saved = SaveFileData(filename, data, size);
    TEST_ASSERT_TRUE(saved);

    int loaded_size = 0;
    unsigned char* loaded_data = LoadFileData(filename, &loaded_size);
    TEST_ASSERT_NOT_NULL(loaded_data);
    TEST_ASSERT_EQUAL_INT(size, loaded_size);
    TEST_ASSERT_EQUAL_MEMORY(data, loaded_data, size);

    UnloadFileData(loaded_data);
    remove(filename);
}

static void test_load_file_data_invalid(void)
{
    int loaded_size = -1;
    unsigned char* loaded_data = LoadFileData("nonexistent_file_xyz123.bin", &loaded_size);
    TEST_ASSERT_NULL(loaded_data);
    TEST_ASSERT_EQUAL_INT(0, loaded_size);
}

static void test_save_file_data_invalid(void)
{
    bool saved = SaveFileData(NULL, "data", 4);
    TEST_ASSERT_FALSE(saved);
}

static void test_file_text_save_load(void)
{
    const char* filename = "test_text.txt";
    const char* text = "Hello, Raylib Shims!\nThis is a test.";

    bool saved = SaveFileText(filename, text);
    TEST_ASSERT_TRUE(saved);

    char* loaded_text = LoadFileText(filename);
    TEST_ASSERT_NOT_NULL(loaded_text);
    TEST_ASSERT_EQUAL_STRING(text, loaded_text);

    UnloadFileText(loaded_text);
    remove(filename);
}

static void test_load_file_text_invalid(void)
{
    char* loaded_text = LoadFileText("nonexistent_file_xyz123.txt");
    TEST_ASSERT_NULL(loaded_text);
}

static void test_save_file_text_invalid(void)
{
    bool saved = SaveFileText(NULL, "text");
    TEST_ASSERT_FALSE(saved);
}

static void test_tracelog(void)
{
    /* Just verify it doesn't crash. Since we can't easily capture stdout in a portable way
       without more code, simply calling it exercises the va_list and printf logic. */
    TraceLog(3, "Test log message %d", 123);
}

void run_test_raylib_shims(void)
{
    RUN_TEST(test_SaveFileText_null_filename);
    RUN_TEST(test_SaveFileText_null_text);
    RUN_TEST(test_SaveFileText_null_both);
    RUN_TEST(test_mem_alloc_free);
    RUN_TEST(test_file_data_save_load);
    RUN_TEST(test_load_file_data_invalid);
    RUN_TEST(test_save_file_data_invalid);
    RUN_TEST(test_file_text_save_load);
    RUN_TEST(test_load_file_text_invalid);
    RUN_TEST(test_save_file_text_invalid);
    RUN_TEST(test_tracelog);
}
