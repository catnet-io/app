#include "unity/unity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* The shim declarations */
bool SaveFileText(const char* fileName, const char* text);
char* LoadFileText(const char* fileName);

static void test_save_file_text_valid(void)
{
    const char* test_file = "tmp_test_save_file.txt";
    const char* test_text = "Hello, world! This is a test.\nLine 2.";

    /* Ensure file does not exist initially */
    remove(test_file);

    /* Test writing */
    bool result = SaveFileText(test_file, test_text);
    TEST_ASSERT_TRUE(result);

    /* Read back using LoadFileText (which is also a shim we could rely on,
       but standard IO is safer for strict testing. We'll use standard IO.) */
    /* We open in "rt" (text mode) or just "r" to read back to normalize CRLF, since
       SaveFileText uses "wt" which on Windows converts \n to \r\n. */
    FILE* f = fopen(test_file, "rt");
    TEST_ASSERT_NOT_NULL(f);

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);

    char* buf = (char*)malloc(sz + 1);
    TEST_ASSERT_NOT_NULL(buf);

    size_t rd = fread(buf, 1, sz, f);
    buf[rd] = '\0';
    fclose(f);

    TEST_ASSERT_EQUAL_STRING(test_text, buf);

    /* Cleanup */
    free(buf);
    remove(test_file);
}

void run_test_raylib_shims(void)
{
    RUN_TEST(test_save_file_text_valid);
}
