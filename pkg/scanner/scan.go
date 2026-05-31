package scanner

import (
	"context"
	"fmt"
	"sync"
	"sync/atomic"
)

type DeviceInfo struct {
	IP             string `json:"ip"`
	IsAlive        bool   `json:"isAlive"`
	Hostname       string `json:"hostname"`
	MAC            string `json:"mac"`
	OpenPortsCount int    `json:"openPortsCount"`
	OpenPorts      []int  `json:"openPorts"`
}

type ScanConfig struct {
	DefaultPorts  []int `json:"defaultPorts"`
	PortTimeoutMs int   `json:"portTimeoutMs"`
	PingTimeoutMs int   `json:"pingTimeoutMs"`
	MaxThreads    int   `json:"maxThreads"`
}

// Global state for parallel scanning
var (
	isScanning atomic.Bool
	scanMu     sync.Mutex
	cancelScan context.CancelFunc
)

func DefaultConfig() ScanConfig {
	return ScanConfig{
		DefaultPorts:  []int{22, 80, 443, 139, 445, 3389},
		PortTimeoutMs: 500,
		PingTimeoutMs: 1000,
		MaxThreads:    64,
	}
}

// StartScan parallelizes the scanning of an IP range.
// It sends results back via the onResult callback.
func StartScan(ips []string, cfg ScanConfig, onResult func(DeviceInfo), onProgress func(float64)) error {
	if !isScanning.CompareAndSwap(false, true) {
		return fmt.Errorf("scan already in progress")
	}
	defer isScanning.Store(false)

	ctx, cancel := context.WithCancel(context.Background())
	scanMu.Lock()
	cancelScan = cancel
	scanMu.Unlock()
	
	defer func() {
		scanMu.Lock()
		cancelScan = nil
		scanMu.Unlock()
		cancel()
	}()

	total := len(ips)
	if total == 0 {
		return nil
	}

	ipChan := make(chan string, total)
	for _, ip := range ips {
		ipChan <- ip
	}
	close(ipChan)

	var wg sync.WaitGroup
	threads := cfg.MaxThreads
	if threads <= 0 {
		threads = 16
	}

	const maxAllowedThreads = 256
	if threads > maxAllowedThreads {
		threads = maxAllowedThreads
	}

	var processed int32

	for i := 0; i < threads; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for ip := range ipChan {
				select {
				case <-ctx.Done():
					return
				default:
					di := DeviceInfo{IP: ip}
					di.IsAlive = Ping(ip, cfg.PingTimeoutMs)

					if di.IsAlive {
						di.Hostname = ReverseDNS(ip)
						di.MAC = GetMAC(ip)
						di.OpenPorts = ScanPorts(ip, cfg.DefaultPorts, cfg.PortTimeoutMs)
						di.OpenPortsCount = len(di.OpenPorts)
					}

					if onResult != nil {
						onResult(di)
					}

					curr := atomic.AddInt32(&processed, 1)
					if onProgress != nil {
						onProgress(float64(curr) / float64(total))
					}
				}
			}
		}()
	}

	wg.Wait()
	return nil
}

// StopScan cancels the ongoing scan
func StopScan() {
	scanMu.Lock()
	defer scanMu.Unlock()
	if cancelScan != nil {
		cancelScan()
	}
}

// Helper para converter sub-rede em lista de IPs pode ser adicionado em um utils.go
