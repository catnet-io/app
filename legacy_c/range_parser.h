#ifndef RANGE_PARSER_H
#define RANGE_PARSER_H

#include <stddef.h>
#include <stdbool.h>

/* Maximum number of addresses allowed in a scan range */
#define RANGE_MAX_HOSTS 65536

/*
 * Parse an IP range/CIDR string into start and end IP strings.
 * Accepted formats:
 *   "A.B.C.D-E"         (last octet of end only)
 *   "A.B.C.D-E.F.G.H"   (full range)
 *   "A.B.C.D/N"         (CIDR, N in [0,32])
 *
 * Returns true on success; false on any error.
 * On error, errbuf (if not NULL) receives a descriptive message.
 */
bool parse_range(const char* in,
                 char* out_start, size_t sz_start,
                 char* out_end,   size_t sz_end,
                 char* errbuf,    size_t errsz);

#endif /* RANGE_PARSER_H */
