#ifndef PARALLEL_SCAN_H
#define PARALLEL_SCAN_H

#include "app.h"
#include "scan.h"

typedef enum {
    SCAN_IDLE     = 0,
    SCAN_RUNNING  = 1,
    SCAN_STOPPING = 2
} ScanStateMachine;

#ifdef __cplusplus
extern "C" {
#endif

/* Starts a parallel scan across [start_ip_uint, end_ip_uint] inclusive.
 * Returns 1 on success, 0 if already running/stopping or on init failure. */
int parallel_scan_start(unsigned long start_ip_uint,
                        unsigned long end_ip_uint,
                        const ScanConfig* cfg,
                        ScanLogFn logger,
                        void* logger_ctx);

/* Requests cancellation, transitions to SCAN_STOPPING, waits for workers,
 * then transitions to SCAN_IDLE. Idempotent: no-op if already SCAN_IDLE. */
void parallel_scan_stop(void);

/* Copies current results snapshot into 'out'. Thread-safe. */
void parallel_scan_snapshot(DeviceList* out);

/* Returns 1 if state == SCAN_RUNNING. */
int parallel_scan_is_running(void);

/* Returns the current state machine value. */
ScanStateMachine parallel_scan_get_state(void);

/* Returns the fraction [0.0, 1.0] of addresses processed. */
void parallel_scan_progress(float* out_fraction);

#ifdef __cplusplus
}
#endif

#endif /* PARALLEL_SCAN_H */
