/* tests/test_scan.c */
#include "unity/unity.h"
#include <string.h>
#include "../src/scan.h"
#include "../src/net.h"
#include "../src/app.h"

static void test_scan_config_init(void)
{
    ScanConfig cfg;
    memset(&cfg, 0, sizeof(cfg)); // ensure it's clean before init

    scan_config_init(&cfg);

    TEST_ASSERT_EQUAL_INT(6, cfg.default_ports_count);

    // Check default ports
    TEST_ASSERT_EQUAL_INT(22, cfg.default_ports[0]);
    TEST_ASSERT_EQUAL_INT(80, cfg.default_ports[1]);
    TEST_ASSERT_EQUAL_INT(443, cfg.default_ports[2]);
    TEST_ASSERT_EQUAL_INT(139, cfg.default_ports[3]);
    TEST_ASSERT_EQUAL_INT(445, cfg.default_ports[4]);
    TEST_ASSERT_EQUAL_INT(3389, cfg.default_ports[5]);
    TEST_ASSERT_EQUAL_INT(500, cfg.port_timeout_ms);
}

static void test_scan_range_end_less_than_start(void)
{
    DeviceList out;
    device_list_init(&out);
    ScanConfig cfg;
    scan_config_init(&cfg);

    int result = scan_range(&out, &cfg, "10.0.0.100", "10.0.0.1", NULL, NULL);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_size_t(0, out.count);

    device_list_clear(&out);
}

void run_test_scan(void)
{
    RUN_TEST(test_scan_config_init);
    RUN_TEST(test_scan_range_end_less_than_start);
}
