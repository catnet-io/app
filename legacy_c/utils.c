#include "utils.h"
#include <string.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

int ip_to_uint(const char* ip, unsigned long* out) {
    struct in_addr addr;
    if (InetPton(AF_INET, ip, &addr) != 1) {
        return 0;
    }
    *out = ntohl(addr.S_un.S_addr);
    return 1;
}

void uint_to_ip(unsigned long ip, char* buf, size_t buflen) {
    struct in_addr addr;
    addr.S_un.S_addr = htonl((u_long)ip);
    if (InetNtop(AF_INET, &addr, buf, buflen) == NULL) {
        if (buflen > 0) buf[0] = '\0';
    }
}

void trim_newline(char* s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

void safe_strcpy(char* dst, size_t dstsz, const char* src) {
    if (!dst || dstsz == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dstsz - 1);
    dst[dstsz - 1] = '\0';
}

void filter_digits(char* buf, int max_len)
{
    if (!buf || max_len <= 0) return;
    int w = 0;
    for (int r = 0; buf[r] != '\0' && w < max_len - 1; ++r)
        if (buf[r] >= '0' && buf[r] <= '9')
            buf[w++] = buf[r];
    buf[w] = '\0';
}

int parse_octet(const char* s)
{
    if (!s || !s[0]) return -1;
    int v = 0;
    int i = 0;
    for (; s[i] && i < 3; ++i) {
        if (s[i] < '0' || s[i] > '9') return -1;
        v = v * 10 + (s[i] - '0');
    }
    if (s[i] != '\0') return -1; /* more than 3 chars */
    return (v >= 0 && v <= 255) ? v : -1;
}

