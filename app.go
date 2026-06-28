package main

import (
	"context"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/catnet-io/engine/pkg/diff"
	"github.com/catnet-io/engine/pkg/events"
	"github.com/catnet-io/engine/pkg/export"
	"github.com/catnet-io/engine/pkg/profile"
	"github.com/catnet-io/engine/pkg/results"
	"github.com/catnet-io/engine/pkg/scan"
	"github.com/catnet-io/engine/pkg/store"
	"github.com/catnet-io/engine/pkg/targets"
	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// App struct
type App struct {
	ctx    context.Context
	engine *scan.Engine
	store  store.ScanStore
}

// NewApp creates a new App application struct
func NewApp() *App {
	return &App{
		engine: scan.NewEngine(),
	}
}

// startup is called when the app starts. The context is saved
// so we can call the runtime methods
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx

	// Initialize the SQLite store
	appDir, err := os.UserConfigDir()
	if err != nil {
		appDir = "."
	} else {
		appDir = filepath.Join(appDir, "catnet")
	}

	dbPath := filepath.Join(appDir, "store.db")
	dbStore, err := store.NewSQLiteStore(dbPath)
	if err != nil {
		runtime.LogErrorf(ctx, "Failed to initialize store: %v", err)
	} else {
		a.store = dbStore
	}
}

// StartScan wrapper for frontend
func (a *App) StartScan(ips []string, cfg profile.ScanProfile) error {
	// Sanitizar configuraÃ§Ã£o recebida do frontend
	if cfg.Concurrency <= 0 || cfg.Concurrency > 256 {
		cfg.Concurrency = 16
	}
	if cfg.TimeoutMs <= 0 || cfg.TimeoutMs > 10000 {
		cfg.TimeoutMs = 1000
	}

	eventChan := make(chan events.Event)
	done := make(chan struct{})

	report := results.NewScanReport()
	targetStr := "Local Network"
	if len(ips) > 0 {
		if len(ips) > 3 {
			targetStr = fmt.Sprintf("%s... (%d IPs)", ips[0], len(ips))
		} else {
			targetStr = strings.Join(ips, ", ")
		}
	}

	// Goroutine to listen for events from the core engine and proxy them to Wails UI
	go func() {
		for ev := range eventChan {
			switch ev.Type {
			case events.ScanStarted:
				runtime.EventsEmit(a.ctx, "scan_started")
			case events.HostDiscovered:
				data, ok := ev.Data.(events.HostDiscoveredData)
				if ok {
					deviceInfo := data.Host.ToDeviceInfo()
					report.Devices = append(report.Devices, deviceInfo)
					if data.Host.Alive {
						report.Alive++
					}
					// Adapt for the current frontend expectation if necessary
					runtime.EventsEmit(a.ctx, "scan_result", data.Host)
				}
			case events.ScanProgress:
				data, ok := ev.Data.(events.ProgressData)
				if ok {
					runtime.EventsEmit(a.ctx, "scan_progress", data.Ratio)
				}
			case events.ScanCompleted:
				runtime.EventsEmit(a.ctx, "scan_finished")
			}
		}
		done <- struct{}{}
	}()

	err := a.engine.ScanStream(context.Background(), ips, cfg, eventChan)
	close(eventChan)
	<-done // Wait for the event processing to finish

	// Save report to database
	if a.store != nil {
		report.EndTime = time.Now()
		report.Total = len(ips)
		if report.Total == 0 {
			report.Total = len(report.Devices)
		}
		_, saveErr := a.store.SaveReport(targetStr, report)
		if saveErr != nil {
			runtime.LogErrorf(a.ctx, "Failed to save report: %v", saveErr)
		}
	}

	return err
}

// StopScan wrapper
func (a *App) StopScan() {
	if a.engine != nil {
		a.engine.Stop()
	}
}

// Ping wrapper for Quick Tools
func (a *App) Ping(ip string) bool {
	return scan.Ping(ip, 1000)
}

// ReverseDNS wrapper
func (a *App) ReverseDNS(ip string) string {
	return scan.ReverseDNS(ip)
}

// GetMAC wrapper
func (a *App) GetMAC(ip string) string {
	return scan.GetMAC(ip)
}

// ScanPorts wrapper
func (a *App) ScanPorts(ip string, ports []int) []int {
	return scan.ScanPorts(ip, ports, 500)
}

// ParseRange expands an IP range string (e.g. 192.168.1.1-254) into a list of IPs.
func (a *App) ParseRange(input string) ([]string, error) {
	return targets.ParseRange(input)
}

// GetLocalIPRange attempts to find the primary network interface and returns its CIDR or range.
func (a *App) GetLocalIPRange() string {
	// Use UDP dialing to find the preferred outbound IP address
	conn, err := net.Dial("udp", "8.8.8.8:80")
	if err == nil {
		defer conn.Close()
		localAddr := conn.LocalAddr().(*net.UDPAddr)
		
		addrs, _ := net.InterfaceAddrs()
		for _, addr := range addrs {
			if ipnet, ok := addr.(*net.IPNet); ok && ipnet.IP.To4() != nil {
				if ipnet.IP.Equal(localAddr.IP) {
					ip := ipnet.IP.To4()
					mask := ipnet.Mask
					network := net.IP{ip[0] & mask[0], ip[1] & mask[1], ip[2] & mask[2], ip[3] & mask[3]}
					
					// If it's a standard /24 subnet, format it nicely as 192.168.X.1-254
					ones, _ := mask.Size()
					if ones == 24 {
						return fmt.Sprintf("%d.%d.%d.1-254", network[0], network[1], network[2])
					}
					return fmt.Sprintf("%s/%d", network.String(), ones)
				}
			}
		}
	}

	// Fallback to loop over interfaces
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		return "192.168.1.1-254"
	}

	for _, addr := range addrs {
		if ipnet, ok := addr.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
			ip := ipnet.IP.To4()
			if ip != nil {
				if ip[0] == 169 && ip[1] == 254 {
					continue
				}

				if ip[0] == 192 || ip[0] == 10 || ip[0] == 172 {
					mask := ipnet.Mask
					ones, _ := mask.Size()
					if ones == 24 {
						return fmt.Sprintf("%d.%d.%d.1-254", ip[0], ip[1], ip[2])
					}
					network := net.IP{ip[0] & mask[0], ip[1] & mask[1], ip[2] & mask[2], ip[3] & mask[3]}
					return fmt.Sprintf("%s/%d", network.String(), ones)
				}
			}
		}
	}

	return "192.168.1.1-254"
}

// ExportResults asks the user for a save location and exports the results
func (a *App) ExportResults(devices []results.HostResult) (string, error) {
	options := runtime.SaveDialogOptions{
		DefaultFilename: "catnet_results.json",
		Title:           "Export Scan Results",
		Filters: []runtime.FileFilter{
			{DisplayName: "JSON Files (*.json)", Pattern: "*.json"},
			{DisplayName: "CSV Files (*.csv)", Pattern: "*.csv"},
		},
	}

	savePath, err := runtime.SaveFileDialog(a.ctx, options)
	if err != nil || savePath == "" {
		return "", err
	}

	// Sanitizar e validar o caminho retornado pelo diÃ¡logo
	cleanPath := filepath.Clean(savePath)
	if cleanPath != savePath {
		return "", fmt.Errorf("caminho de arquivo invÃ¡lido")
	}

	dir := filepath.Dir(cleanPath)
	if dir == "" || dir == "." {
		return "", fmt.Errorf("diretÃ³rio de destino invÃ¡lido")
	}

	var data []byte
	var formatErr error

	if strings.ToLower(filepath.Ext(savePath)) == ".json" {
		data, formatErr = export.ExportJSON(devices)
	} else {
		data, formatErr = export.ExportCSV(devices)
	}

	if formatErr != nil {
		return "", formatErr
	}

	err = os.WriteFile(savePath, data, 0644)
	return savePath, err
}

// GetScans returns the history of scans
func (a *App) GetScans() ([]store.ScanSummary, error) {
	if a.store == nil {
		return nil, fmt.Errorf("database not initialized")
	}
	return a.store.GetScans()
}

// GetScanReport returns the details of a specific scan
func (a *App) GetScanReport(scanID int64) (*results.ScanReport, error) {
	if a.store == nil {
		return nil, fmt.Errorf("database not initialized")
	}
	return a.store.GetReport(scanID)
}

// DeleteScan removes a scan from history
func (a *App) DeleteScan(scanID int64) error {
	if a.store == nil {
		return fmt.Errorf("database not initialized")
	}
	return a.store.DeleteScan(scanID)
}

// CompareScans compares two scans and returns the differences
func (a *App) CompareScans(oldID, newID int64) ([]diff.HostDiff, error) {
	if a.store == nil {
		return nil, fmt.Errorf("database not initialized")
	}
	
	oldReport, err := a.store.GetReport(oldID)
	if err != nil {
		return nil, fmt.Errorf("failed to get old report: %w", err)
	}
	
	newReport, err := a.store.GetReport(newID)
	if err != nil {
		return nil, fmt.Errorf("failed to get new report: %w", err)
	}
	
	return diff.Compare(oldReport, newReport), nil
}
