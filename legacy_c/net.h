#ifndef NET_H
#define NET_H

#include <stddef.h>
#include "app.h"
// Avoid including Windows SDK headers here; keep this header lightweight
// to prevent symbol conflicts in UI translation units.


typedef struct {
    unsigned long network;
    unsigned long start_ip;
    unsigned long end_ip;
    unsigned long mask;
} SubnetV4;

#ifdef __cplusplus
extern "C" {
#endif
int net_init(void);
void net_cleanup(void);

int net_ping_ipv4(const char* ip);
int net_reverse_dns(const char* ip, char* hostname, size_t hostsz);
int net_get_mac(const char* ip, char* macbuf, size_t macsz);
int net_scan_ports(const char* ip, const int* ports, int ports_count, int timeout_ms, int* open_ports, int* open_count);

int net_get_primary_subnet(SubnetV4* out);
#ifdef __cplusplus
}
#endif

#endif // NET_H