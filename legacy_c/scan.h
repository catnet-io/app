#ifndef SCAN_H
#define SCAN_H

#include "app.h"

typedef struct {
    int default_ports[16];
    int default_ports_count;
    int port_timeout_ms;
} ScanConfig;

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*ScanLogFn)(const char* msg, void* ctx);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
void scan_config_init(ScanConfig* cfg);

// Returns 1 on success, 0 on failure
int scan_subnet(DeviceList* out, const ScanConfig* cfg, ScanLogFn logger, void* logger_ctx);
// Returns 1 on success, 0 on failure
int scan_range(DeviceList* out, const ScanConfig* cfg, const char* start_ip, const char* end_ip, ScanLogFn logger, void* logger_ctx);
void identify_device(DeviceInfo* info, const ScanConfig* cfg, ScanLogFn logger, void* logger_ctx);
#ifdef __cplusplus
}
#endif

#endif // SCAN_H