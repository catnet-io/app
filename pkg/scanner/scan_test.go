package scanner

import (
	"sync"
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
	started := make(chan struct{})
	var once sync.Once

	onResult := func(di DeviceInfo) {
		atomic.AddInt32(&count, 1)
		once.Do(func() { close(started) })
	}
	
	// Run StartScan in a goroutine
	done := make(chan struct{})
	go func() {
		_ = StartScan(ips, cfg, onResult, nil)
		close(done)
	}()
	
	// Dar tempo para o scan iniciar, depois cancelar
	time.Sleep(20 * time.Millisecond)
	StopScan()
	
	select {
	case <-done:
		// ok
	case <-time.After(5 * time.Second):
		t.Fatal("StartScan did not return after StopScan within timeout")
	}
}
