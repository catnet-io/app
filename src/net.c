/*
 * net.c — Network operations for CatNet Scanner
 *
 * Tasks 9.1–9.4: Removed LoadLibraryA/GetProcAddress/FreeLibrary for ICMP,
 * replaced inet_addr with InetPton, replaced gethostbyaddr with getnameinfo,
 * added WSAGetLastError logging on key failure paths.
 *
 * Linked libraries: Ws2_32.lib, Iphlpapi.lib (already in build.ps1)
 */

#include "net.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>

int net_init(void)
{
    WSADATA wsa;
    int r = WSAStartup(MAKEWORD(2, 2), &wsa);
    return r == 0;
}

void net_cleanup(void)
{
    WSACleanup();
}

/* Task 9.1: Direct ICMP linkage — no LoadLibraryA/GetProcAddress/FreeLibrary.
 * Task 9.2: InetPton replaces inet_addr; no INADDR_NONE comparison.
 * Task 9.4: Log GetLastError() to stderr when IcmpCreateFile fails. */
int net_ping_ipv4(const char *ip)
{
    /* Task 9.1: Call IcmpCreateFile directly (linked via Iphlpapi.lib). */
    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE)
    {
        /* Task 9.4: Log failure with GetLastError to stderr. */
        fprintf(stderr, "net_ping_ipv4: IcmpCreateFile failed, WSA error %lu\n", GetLastError());
        return 0;
    }

    /* Task 9.2: Use InetPton instead of inet_addr. */
    IPAddr addr = 0;
    if (InetPton(AF_INET, ip, &addr) != 1)
    {
        IcmpCloseHandle(hIcmp);
        return 0;
    }

    char SendData[] = "ping";
    DWORD ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    char *ReplyBuffer = (char *)malloc(ReplySize);
    if (!ReplyBuffer)
    {
        IcmpCloseHandle(hIcmp);
        return 0;
    }

    DWORD dwRet = IcmpSendEcho(hIcmp, addr, SendData, (WORD)sizeof(SendData),
                               NULL, ReplyBuffer, ReplySize, 1000);
    free(ReplyBuffer);

    /* Task 9.1: Close handle on both success and failure paths. */
    if (dwRet == 0)
    {
        /* Task 9.1: IcmpSendEcho failed — close handle before returning 0. */
        IcmpCloseHandle(hIcmp);
        return 0;
    }

    IcmpCloseHandle(hIcmp);
    return 1;
}

/* Task 9.3: getnameinfo replaces gethostbyaddr.
 * Task 9.2: InetPton replaces inet_addr.
 * Task 9.4: Log getnameinfo return code to stderr on failure. */
int net_reverse_dns(const char *ip, char *hostname, size_t hostsz)
{
    /* Task 9.2: Parse address with InetPton. */
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    if (InetPton(AF_INET, ip, &sa.sin_addr) != 1)
    {
        hostname[0] = '\0';
        return 0;
    }

    /* Task 9.3: Use getnameinfo instead of gethostbyaddr. */
    int rc = getnameinfo((struct sockaddr *)&sa, sizeof(sa),
                         hostname, (DWORD)hostsz,
                         NULL, 0, NI_NOFQDN);
    if (rc != 0)
    {
        /* Task 9.4: Log getnameinfo failure code to stderr. */
        fprintf(stderr, "net_reverse_dns: getnameinfo failed, code %d\n", rc);
        hostname[0] = '\0';
        return 0;
    }

    return 1;
}

/* Task 9.2: InetPton replaces inet_addr; no INADDR_NONE comparison. */
int net_get_mac(const char *ip, char *macbuf, size_t macsz)
{
    struct in_addr inaddr;
    if (InetPton(AF_INET, ip, &inaddr) != 1)
        return 0;

    IPAddr destIp = inaddr.S_un.S_addr;

    DWORD macLen = 6;
    BYTE mac[6] = {0};
    DWORD r = SendARP(destIp, 0, mac, &macLen);
    if (r == NO_ERROR && macLen == 6)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%02X-%02X-%02X-%02X-%02X-%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        safe_strcpy(macbuf, macsz, buf);
        return 1;
    }
    return 0;
}

/* Task 9.2: InetPton replaces inet_addr in connect_with_timeout.
 * Task 9.4: Log WSAGetLastError to stderr when socket() fails. */
static int connect_with_timeout(const char *ip, int port, int timeout_ms)
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        /* Task 9.4: Log socket creation failure. */
        fprintf(stderr, "connect_with_timeout: socket() failed, WSA error %d\n",
                WSAGetLastError());
        return 0;
    }

    u_long mode = 1; /* non-blocking */
    ioctlsocket(s, FIONBIO, &mode);

    /* Task 9.2: Use InetPton instead of inet_addr. */
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((u_short)port);
    if (InetPton(AF_INET, ip, &sa.sin_addr) != 1)
    {
        closesocket(s);
        return 0;
    }

    int r = connect(s, (struct sockaddr *)&sa, sizeof(sa));
    if (r == 0)
    {
        closesocket(s);
        return 1;
    }

    if (WSAGetLastError() == WSAEWOULDBLOCK)
    {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(s, &wfds);
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        r = select(0, NULL, &wfds, NULL, &tv);
        if (r > 0 && FD_ISSET(s, &wfds))
        {
            int err = 0;
            int len = sizeof(err);
            getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
            closesocket(s);
            return err == 0;
        }
    }
    closesocket(s);
    return 0;
}

int net_scan_ports(const char *ip, const int *ports, int ports_count,
                   int timeout_ms, int *open_ports, int *open_count)
{
    int found = 0;
    for (int i = 0; i < ports_count; ++i)
    {
        if (connect_with_timeout(ip, ports[i], timeout_ms))
        {
            if (open_ports && open_count)
            {
                open_ports[*open_count] = ports[i];
                (*open_count)++;
            }
            found++;
        }
    }
    return found;
}

int net_get_primary_subnet(SubnetV4 *out)
{
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG size = 0;
    GetAdaptersAddresses(AF_UNSPEC, flags, NULL, NULL, &size);
    IP_ADAPTER_ADDRESSES *addrs = (IP_ADAPTER_ADDRESSES *)malloc(size);
    if (!addrs)
        return 0;
    if (GetAdaptersAddresses(AF_UNSPEC, flags, NULL, addrs, &size) != NO_ERROR)
    {
        free(addrs);
        return 0;
    }

    IP_ADAPTER_ADDRESSES *cur = addrs;
    while (cur)
    {
        if (cur->OperStatus == IfOperStatusUp)
        {
            IP_ADAPTER_UNICAST_ADDRESS *uni = cur->FirstUnicastAddress;
            while (uni)
            {
                if (uni->Address.lpSockaddr->sa_family == AF_INET)
                {
                    struct sockaddr_in *sin = (struct sockaddr_in *)uni->Address.lpSockaddr;
                    DWORD prefix = uni->OnLinkPrefixLength;
                    unsigned long ip_val = ntohl(sin->sin_addr.S_un.S_addr);
                    unsigned long mask = (prefix == 0) ? 0 : 0xFFFFFFFFUL << (32 - prefix);
                    unsigned long network = ip_val & mask;
                    unsigned long start_ip = network + 1;
                    unsigned long end_ip = (network | (~mask)) - 1;
                    out->network = network;
                    out->mask = mask;
                    out->start_ip = start_ip;
                    out->end_ip = end_ip;
                    free(addrs);
                    return 1;
                }
                uni = uni->Next;
            }
        }
        cur = cur->Next;
    }
    free(addrs);
    return 0;
}
