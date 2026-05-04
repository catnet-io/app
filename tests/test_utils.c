/* tests/test_utils.c */
#include "unity/unity.h"
#include <string.h>
#include "../src/utils.h"

/* --- ip_to_uint tests --- */
static void test_ip_to_uint_valid(void)
{
    unsigned long out = 0;
    int r = ip_to_uint("192.168.1.1", &out);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_UINT32(0xC0A80101UL, out);
}

static void test_ip_to_uint_broadcast(void)
{
    unsigned long out = 0;
    int r = ip_to_uint("255.255.255.255", &out);
    TEST_ASSERT_EQUAL_INT(1, r);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFUL, out);
}

static void test_ip_to_uint_invalid(void)
{
    unsigned long out = 0;
    int r = ip_to_uint("not-an-ip", &out);
    TEST_ASSERT_EQUAL_INT(0, r);
}

/* --- uint_to_ip tests --- */
static void test_uint_to_ip_known(void)
{
    char buf[64];
    uint_to_ip(0xC0A80101UL, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("192.168.1.1", buf);
}

static void test_uint_to_ip_roundtrip(void)
{
    const char* original = "10.0.0.1";
    unsigned long val = 0;
    ip_to_uint(original, &val);
    char buf[64];
    uint_to_ip(val, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING(original, buf);
}

/* --- filter_digits tests --- */
static void test_filter_digits_mixed(void)
{
    char buf[8];
    safe_strcpy(buf, sizeof(buf), "1a2b3");
    filter_digits(buf, 4);
    TEST_ASSERT_EQUAL_STRING("123", buf);
}

static void test_filter_digits_all_alpha(void)
{
    char buf[8];
    safe_strcpy(buf, sizeof(buf), "abc");
    filter_digits(buf, 4);
    TEST_ASSERT_EQUAL_STRING("", buf);
}

static void test_filter_digits_empty(void)
{
    char buf[8];
    safe_strcpy(buf, sizeof(buf), "");
    filter_digits(buf, 4);
    TEST_ASSERT_EQUAL_STRING("", buf);
}

static void test_filter_digits_truncate(void)
{
    char buf[8];
    safe_strcpy(buf, sizeof(buf), "1234567");
    filter_digits(buf, 4);
    TEST_ASSERT_EQUAL_STRING("123", buf);
}

/* --- parse_octet tests --- */
static void test_parse_octet_valid(void)
{
    TEST_ASSERT_EQUAL_INT(192, parse_octet("192"));
    TEST_ASSERT_EQUAL_INT(0,   parse_octet("0"));
    TEST_ASSERT_EQUAL_INT(255, parse_octet("255"));
}

static void test_parse_octet_out_of_range(void)
{
    TEST_ASSERT_EQUAL_INT(-1, parse_octet("256"));
}

static void test_parse_octet_alpha(void)
{
    TEST_ASSERT_EQUAL_INT(-1, parse_octet("abc"));
}

static void test_parse_octet_empty(void)
{
    TEST_ASSERT_EQUAL_INT(-1, parse_octet(""));
    TEST_ASSERT_EQUAL_INT(-1, parse_octet(NULL));
}

void run_test_utils(void)
{
    RUN_TEST(test_ip_to_uint_valid);
    RUN_TEST(test_ip_to_uint_broadcast);
    RUN_TEST(test_ip_to_uint_invalid);
    RUN_TEST(test_uint_to_ip_known);
    RUN_TEST(test_uint_to_ip_roundtrip);
    RUN_TEST(test_filter_digits_mixed);
    RUN_TEST(test_filter_digits_all_alpha);
    RUN_TEST(test_filter_digits_empty);
    RUN_TEST(test_filter_digits_truncate);
    RUN_TEST(test_parse_octet_valid);
    RUN_TEST(test_parse_octet_out_of_range);
    RUN_TEST(test_parse_octet_alpha);
    RUN_TEST(test_parse_octet_empty);
}
