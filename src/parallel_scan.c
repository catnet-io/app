#include "parallel_scan.h"
#include "scan.h"
#include "net.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>

typedef struct {
    unsigned long   start_ip;
    unsigned long   end_ip;
    volatile LONG   next_ip;        /* atomic counter, incremented per worker */
    volatile LONG   cancel;
    ScanStateMachine state;         /* protected by state_lock */
    CRITICAL_SECTION state_lock;
    int             state_lock_initialized;
    ScanConfig      cfg;
    DeviceList      results;
    CRITICAL_SECTION results_lock;
    int             results_lock_initialized;
    ScanLogFn       logger;
    void*           logger_ctx;
    int             num_threads;
    HANDLE          threads[64];
    volatile LONG   rate_count;
    ULONGLONG       rate_window_start;
    LONG            rate_limit;
} ScanState;

static ScanState g_state;
static int g_state_initialized = 0;  /* tracks if g_state has ever been set up */

/* ---- Worker thread ---- */
static DWORD WINAPI worker_proc(LPVOID lpParam)
{
    ScanState* st = (ScanState*)lpParam;
    for (;;) {
        if (st->cancel) break;
        LONG ip_index = InterlockedIncrement(&st->next_ip) - 1;
        if ((unsigned long)ip_index > st->end_ip) break;

        /* Rate limiting */
        for (;;) {
            ULONGLONG now = GetTickCount64();
            if (now - st->rate_window_start >= 1000) {
                st->rate_window_start = now;
                InterlockedExchange(&st->rate_count, 0);
            }
            LONG cur = st->rate_count;
            if (cur < st->rate_limit) { InterlockedIncrement(&st->rate_count); break; }
            Sleep(1);
            if (st->cancel) break;
        }
        if (st->cancel) break;

        DeviceInfo di;
        memset(&di, 0, sizeof(di));
        uint_to_ip((unsigned long)ip_index, di.ip, sizeof(di.ip));

        if (st->logger) {
            char msg[96];
            snprintf(msg, sizeof(msg), "Scanning %s", di.ip);
            st->logger(msg, st->logger_ctx);
        }

        /* Pass logger explicitly — no global state */
        identify_device(&di, &st->cfg, st->logger, st->logger_ctx);

        EnterCriticalSection(&st->results_lock);
        device_list_push(&st->results, &di);
        LeaveCriticalSection(&st->results_lock);
    }
    return 0;
}

/* ---- Public API ---- */

int parallel_scan_start(unsigned long start_ip_uint,
                        unsigned long end_ip_uint,
                        const ScanConfig* cfg,
                        ScanLogFn logger,
                        void* logger_ctx)
{
    /* Atomically transition IDLE -> RUNNING; fail if not IDLE */
    if (g_state_initialized) {
        /* Use state_lock to check and transition */
        EnterCriticalSection(&g_state.state_lock);
        if (g_state.state != SCAN_IDLE) {
            LeaveCriticalSection(&g_state.state_lock);
            return 0;
        }
        g_state.state = SCAN_RUNNING;
        LeaveCriticalSection(&g_state.state_lock);
        /* Safe teardown of previous run's resources (results, locks) */
        if (g_state.results_lock_initialized) {
            device_list_clear(&g_state.results);
            DeleteCriticalSection(&g_state.results_lock);
            g_state.results_lock_initialized = 0;
        }
    } else {
        /* First ever call — initialize state_lock */
        memset(&g_state, 0, sizeof(g_state));
        InitializeCriticalSection(&g_state.state_lock);
        g_state.state_lock_initialized = 1;
        g_state.state = SCAN_RUNNING;
        g_state_initialized = 1;
    }

    /* Initialize fields individually (no memset on live struct) */
    g_state.start_ip        = start_ip_uint;
    g_state.end_ip          = end_ip_uint;
    g_state.next_ip         = (LONG)start_ip_uint;
    g_state.cancel          = 0;
    g_state.logger          = logger;
    g_state.logger_ctx      = logger_ctx;
    g_state.rate_count      = 0;
    g_state.rate_window_start = GetTickCount64();
    g_state.rate_limit      = 200;
    if (cfg) g_state.cfg = *cfg; else scan_config_init(&g_state.cfg);

    device_list_init(&g_state.results);
    InitializeCriticalSection(&g_state.results_lock);
    g_state.results_lock_initialized = 1;

    if (!net_init()) {
        if (logger) logger("Network init failed", logger_ctx);
        /* Transition back to IDLE */
        EnterCriticalSection(&g_state.state_lock);
        g_state.state = SCAN_IDLE;
        LeaveCriticalSection(&g_state.state_lock);
        device_list_clear(&g_state.results);
        DeleteCriticalSection(&g_state.results_lock);
        g_state.results_lock_initialized = 0;
        return 0;
    }

    /* Determine thread count */
    int desired = 16;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    int hw = (int)si.dwNumberOfProcessors;
    if (hw > 0) desired = hw * 2;
    if (desired > 64) desired = 64;
    g_state.num_threads = desired;

    for (int i = 0; i < g_state.num_threads; ++i) {
        g_state.threads[i] = CreateThread(NULL, 0, worker_proc, &g_state, 0, NULL);
    }

    if (logger) {
        char msg[96];
        snprintf(msg, sizeof(msg), "Workers started: %d", g_state.num_threads);
        logger(msg, logger_ctx);
    }
    return 1;
}

void parallel_scan_stop(void)
{
    if (!g_state_initialized) return;

    EnterCriticalSection(&g_state.state_lock);
    if (g_state.state == SCAN_IDLE) {
        LeaveCriticalSection(&g_state.state_lock);
        return; /* idempotent */
    }
    g_state.state = SCAN_STOPPING;
    LeaveCriticalSection(&g_state.state_lock);

    /* Signal workers to stop */
    InterlockedExchange(&g_state.cancel, 1);

    /* Wait for all workers */
    if (g_state.num_threads > 0) {
        WaitForMultipleObjects(g_state.num_threads, g_state.threads, TRUE, 5000);
        for (int i = 0; i < g_state.num_threads; ++i) {
            if (g_state.threads[i]) {
                CloseHandle(g_state.threads[i]);
                g_state.threads[i] = NULL;
            }
        }
        g_state.num_threads = 0;
    }

    net_cleanup();

    EnterCriticalSection(&g_state.state_lock);
    g_state.state = SCAN_IDLE;
    LeaveCriticalSection(&g_state.state_lock);
}

void parallel_scan_snapshot(DeviceList* out)
{
    if (!out || !g_state_initialized || !g_state.results_lock_initialized) return;
    EnterCriticalSection(&g_state.results_lock);
    device_list_clear(out);
    for (size_t i = 0; i < g_state.results.count; ++i)
        device_list_push(out, &g_state.results.items[i]);
    LeaveCriticalSection(&g_state.results_lock);
}

int parallel_scan_is_running(void)
{
    if (!g_state_initialized) return 0;
    EnterCriticalSection(&g_state.state_lock);
    int running = (g_state.state == SCAN_RUNNING);
    LeaveCriticalSection(&g_state.state_lock);
    return running;
}

ScanStateMachine parallel_scan_get_state(void)
{
    if (!g_state_initialized) return SCAN_IDLE;
    EnterCriticalSection(&g_state.state_lock);
    ScanStateMachine s = g_state.state;
    LeaveCriticalSection(&g_state.state_lock);
    return s;
}

void parallel_scan_progress(float* out_fraction)
{
    if (!out_fraction) return;
    if (!g_state_initialized) { *out_fraction = 0.0f; return; }

    unsigned long total = g_state.end_ip - g_state.start_ip + 1;
    if (total == 0) { *out_fraction = 1.0f; return; }

    LONG processed = g_state.next_ip - (LONG)g_state.start_ip;
    if (processed < 0) processed = 0;
    if ((unsigned long)processed > total) processed = (LONG)total;

    float f = (float)processed / (float)total;
    if (f < 0.0f) f = 0.0f;
    if (f > 1.0f) f = 1.0f;
    *out_fraction = f;
}
