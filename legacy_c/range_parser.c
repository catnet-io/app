#include "range_parser.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static void set_err(char* errbuf, size_t errsz, const char* msg)
{
    if (errbuf && errsz > 0)
    {
        strncpy(errbuf, msg, errsz - 1);
        errbuf[errsz - 1] = '\0';
    }
}

bool parse_range(const char* in,
                 char* out_start, size_t sz_start,
                 char* out_end,   size_t sz_end,
                 char* errbuf,    size_t errsz)
{
    if (!in || !out_start || sz_start == 0 || !out_end || sz_end == 0)
    {
        set_err(errbuf, errsz, "Invalid range format: expected A.B.C.D-E or A.B.C.D/N");
        return false;
    }

    /* Detect CIDR format (contains '/') */
    const char* slash = NULL;
    for (const char* p = in; *p; ++p)
    {
        if (*p == '/')
        {
            slash = p;
            break;
        }
    }

    if (slash)
    {
        /* Parse prefix */
        int prefix = atoi(slash + 1);
        if (prefix < 0 || prefix > 32)
        {
            set_err(errbuf, errsz, "CIDR prefix must be between 0 and 32");
            return false;
        }

        /* Extract IP part */
        size_t ip_len = (size_t)(slash - in);
        if (ip_len == 0 || ip_len >= sz_start)
        {
            set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
            return false;
        }
        char ip_buf[64];
        if (ip_len >= sizeof(ip_buf))
        {
            set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
            return false;
        }
        strncpy(ip_buf, in, ip_len);
        ip_buf[ip_len] = '\0';

        /* Parse base IP */
        unsigned long base = 0;
        if (!ip_to_uint(ip_buf, &base))
        {
            set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
            return false;
        }

        /* Compute network and broadcast */
        unsigned long mask = (prefix == 0) ? 0UL : (0xFFFFFFFFUL << (32 - prefix));
        unsigned long network   = base & mask;
        unsigned long broadcast = network | (~mask & 0xFFFFFFFFUL);

        /* Check host count — use 64-bit arithmetic to avoid overflow for /0 */
        unsigned long long count = (unsigned long long)broadcast - network + 1ULL;
        if (count > RANGE_MAX_HOSTS)
        {
            set_err(errbuf, errsz, "Range exceeds maximum of 65536 addresses");
            return false;
        }

        /* Fill outputs */
        char s_buf[64] = {0};
        char e_buf[64] = {0};
        uint_to_ip(network,    s_buf, sizeof(s_buf));
        uint_to_ip(broadcast,  e_buf, sizeof(e_buf));
        safe_strcpy(out_start, sz_start, s_buf);
        safe_strcpy(out_end,   sz_end,   e_buf);
        return true;
    }

    /* Detect range format (contains '-') */
    const char* dash = NULL;
    for (const char* p = in; *p; ++p)
    {
        if (*p == '-')
        {
            dash = p;
            break;
        }
    }

    if (!dash)
    {
        set_err(errbuf, errsz, "Invalid range format: expected A.B.C.D-E or A.B.C.D/N");
        return false;
    }

    /* Parse left side (start IP) */
    size_t left_len = (size_t)(dash - in);
    if (left_len == 0 || left_len >= sz_start)
    {
        set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
        return false;
    }
    char start_buf[64];
    if (left_len >= sizeof(start_buf))
    {
        set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
        return false;
    }
    strncpy(start_buf, in, left_len);
    start_buf[left_len] = '\0';

    unsigned long start_uint = 0;
    if (!ip_to_uint(start_buf, &start_uint))
    {
        set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
        return false;
    }

    /* Parse right side (end IP) */
    const char* right = dash + 1;

    /* Check if right side has a dot (full IP) or just last octet */
    bool right_has_dot = false;
    for (const char* p = right; *p; ++p)
    {
        if (*p == '.')
        {
            right_has_dot = true;
            break;
        }
    }

    char end_buf[64] = {0};
    if (!right_has_dot)
    {
        /* Append last octet to prefix of start IP */
        const char* last_dot = NULL;
        for (const char* p = start_buf; *p; ++p)
        {
            if (*p == '.')
                last_dot = p;
        }
        if (!last_dot)
        {
            set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
            return false;
        }
        size_t prefix_len = (size_t)(last_dot - start_buf) + 1; /* includes '.' */
        if (prefix_len >= sizeof(end_buf))
        {
            set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
            return false;
        }
        strncpy(end_buf, start_buf, prefix_len);
        end_buf[prefix_len] = '\0';
        strncat(end_buf, right, sizeof(end_buf) - strlen(end_buf) - 1);
    }
    else
    {
        safe_strcpy(end_buf, sizeof(end_buf), right);
    }

    unsigned long end_uint = 0;
    if (!ip_to_uint(end_buf, &end_uint))
    {
        set_err(errbuf, errsz, "Invalid IP format: octet out of range [0, 255]");
        return false;
    }

    /* Validate ordering */
    if (start_uint > end_uint)
    {
        set_err(errbuf, errsz, "Start IP must be less than or equal to end IP");
        return false;
    }

    /* Validate host count */
    unsigned long count = end_uint - start_uint + 1;
    if (count > RANGE_MAX_HOSTS)
    {
        set_err(errbuf, errsz, "Range exceeds maximum of 65536 addresses");
        return false;
    }

    /* Fill outputs */
    safe_strcpy(out_start, sz_start, start_buf);
    safe_strcpy(out_end,   sz_end,   end_buf);
    return true;
}
