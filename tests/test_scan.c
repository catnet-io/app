/* tests/test_scan.c — Unit tests for src/scan.c
 */
#include "unity/unity.h"
#include "../src/scan.h"

static void test_scan_config_init(void)
{
    ScanConfig cfg;
    scan_config_init(&cfg);

    TEST_ASSERT_EQUAL_INT(6, cfg.default_ports_count);
    TEST_ASSERT_EQUAL_INT(22, cfg.default_ports[0]);
    TEST_ASSERT_EQUAL_INT(80, cfg.default_ports[1]);
    TEST_ASSERT_EQUAL_INT(443, cfg.default_ports[2]);
    TEST_ASSERT_EQUAL_INT(139, cfg.default_ports[3]);
    TEST_ASSERT_EQUAL_INT(445, cfg.default_ports[4]);
    TEST_ASSERT_EQUAL_INT(3389, cfg.default_ports[5]);
    TEST_ASSERT_EQUAL_INT(500, cfg.port_timeout_ms);
}

void run_test_scan(void)
{
    RUN_TEST(test_scan_config_init);
}
