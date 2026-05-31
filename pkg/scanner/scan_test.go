package scanner

import (
	"sync/atomic"
	"testing"
	"time"
)

func TestScanConcurrency(t *testing.T) {
	// A simple test to ensure StartScan and StopScan do not cause race conditions.
	// Since we mock network calls in a real scenario, here we just want to ensure
	// the mutex and globals don't panic or data-race.
	
	ips := []string{"192.0.2.1", "192.0.2.2"} // TEST-NET-1, reserved, won't respond
	cfg := DefaultConfig()
	cfg.PingTimeoutMs = 10
	cfg.PortTimeoutMs = 10
	
	var count int32
	onResult := func(di DeviceInfo) {
		atomic.AddInt32(&count, 1)
	}
	
	// Run StartScan in a goroutine
	done := make(chan struct{})
	go func() {
		_ = StartScan(ips, cfg, onResult, nil)
		close(done)
	}()
	
	// Give it a tiny bit of time to start and acquire lock
	time.Sleep(5 * time.Millisecond)
	
	// StopScan should safely cancel without data races
	StopScan()
	
	<-done
	
	// No panic, no race condition detected (if run with -race)
}
