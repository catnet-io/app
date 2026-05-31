/* tests/test_main.c — Unity test runner entry point */
#include "unity/unity.h"

/* Forward declarations for test suite runners */
void run_test_utils(void);
void run_test_list(void);
void run_test_parse_range(void);
void run_test_export(void);
void run_test_scan(void);

/* Global setUp/tearDown required by Unity when compiling multiple test files */
void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();
    run_test_utils();
    run_test_list();
    run_test_parse_range();
    run_test_export();
    run_test_scan();
    return UNITY_END();
}
