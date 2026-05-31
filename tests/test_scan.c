/* tests/test_scan.c */
#include "unity/unity.h"
#include <string.h>
#include "../src/scan.h"
#include "../src/net.h"
#include "../src/app.h"

// Stub out network functions so we don't need Windows APIs or actual networking
int net_init(void) { return 1; }
void net_cleanup(void) {}
int net_ping_ipv4(const char* ip) { return 0; }
int net_reverse_dns(const char* ip, char* hostname, size_t hostsz) { return 0; }
int net_get_mac(const char* ip, char* macbuf, size_t macsz) { return 0; }
int net_scan_ports(const char* ip, const int* ports, int ports_count, int timeout_ms, int* open_ports, int* open_count) { return 0; }
int net_get_primary_subnet(SubnetV4* out) { return 0; }

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
    RUN_TEST(test_scan_range_end_less_than_start);
}
