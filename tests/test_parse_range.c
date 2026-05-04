/* tests/test_parse_range.c */
#include "unity/unity.h"
#include <string.h>
#include "../src/range_parser.h"
#include "../src/utils.h"

static void test_simple_range_last_octet(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("192.168.1.1-254", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("192.168.1.1", s);
    TEST_ASSERT_EQUAL_STRING("192.168.1.254", e);
}

static void test_full_range(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("10.0.0.1-10.0.0.100", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("10.0.0.1", s);
    TEST_ASSERT_EQUAL_STRING("10.0.0.100", e);
}

static void test_cidr_24(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("192.168.1.0/24", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("192.168.1.0", s);
    TEST_ASSERT_EQUAL_STRING("192.168.1.255", e);
}

static void test_cidr_32_host(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("10.0.0.1/32", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("10.0.0.1", s);
    TEST_ASSERT_EQUAL_STRING("10.0.0.1", e);
}

static void test_cidr_prefix_too_large(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("192.168.1.0/33", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_NOT_NULL(strstr(err, "CIDR prefix must be between 0 and 32"));
}

static void test_cidr_slash_zero_exceeds_limit(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("10.0.0.0/0", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_NOT_NULL(strstr(err, "exceeds maximum"));
}

static void test_invalid_format_no_separator(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("not-an-ip", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    /* "not-an-ip" contains a '-' so it will be treated as a range; ip_to_uint("not") will fail */
    TEST_ASSERT_FALSE(ok);
}

static void test_start_greater_than_end(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("192.168.1.100-10.0.0.1", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_NOT_NULL(strstr(err, "Start IP must be less than or equal to end IP"));
}

static void test_invalid_octet_in_cidr(void)
{
    char s[64], e[64], err[128];
    bool ok = parse_range("192.168.1.300/24", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_FALSE(ok);
}

static void test_exactly_65536_addresses(void)
{
    /* 10.0.0.0/16 = 65536 addresses — should be accepted */
    char s[64], e[64], err[128];
    bool ok = parse_range("10.0.0.0/16", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_STRING("10.0.0.0", s);
    TEST_ASSERT_EQUAL_STRING("10.0.255.255", e);
}

static void test_exceeds_65536_addresses(void)
{
    /* 10.0.0.0/15 = 131072 addresses — should be rejected */
    char s[64], e[64], err[128];
    bool ok = parse_range("10.0.0.0/15", s, sizeof(s), e, sizeof(e), err, sizeof(err));
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_NOT_NULL(strstr(err, "exceeds maximum"));
}

void run_test_parse_range(void)
{
    RUN_TEST(test_simple_range_last_octet);
    RUN_TEST(test_full_range);
    RUN_TEST(test_cidr_24);
    RUN_TEST(test_cidr_32_host);
    RUN_TEST(test_cidr_prefix_too_large);
    RUN_TEST(test_cidr_slash_zero_exceeds_limit);
    RUN_TEST(test_invalid_format_no_separator);
    RUN_TEST(test_start_greater_than_end);
    RUN_TEST(test_invalid_octet_in_cidr);
    RUN_TEST(test_exactly_65536_addresses);
    RUN_TEST(test_exceeds_65536_addresses);
}
