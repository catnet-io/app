package main

import (
	"context"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"strings"

	"github.com/mendsec/catnet-core/pkg/events"
	"github.com/mendsec/catnet-core/pkg/export"
	"github.com/mendsec/catnet-core/pkg/profile"
	"github.com/mendsec/catnet-core/pkg/results"
	"github.com/mendsec/catnet-core/pkg/scan"
	"github.com/mendsec/catnet-core/pkg/targets"
	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// App struct
type App struct {
	ctx    context.Context
	engine *scan.Engine
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
}

// StartScan wrapper for frontend
func (a *App) StartScan(ips []string, cfg profile.ScanProfile) error {
	// Sanitizar configuração recebida do frontend
	if cfg.Concurrency <= 0 || cfg.Concurrency > 256 {
		cfg.Concurrency = 16
	}
	if cfg.TimeoutMs <= 0 || cfg.TimeoutMs > 10000 {
		cfg.TimeoutMs = 1000
	}

	eventChan := make(chan events.Event)

	// Goroutine to listen for events from the core engine and proxy them to Wails UI
	go func() {
		for ev := range eventChan {
			switch ev.Type {
			case events.ScanStarted:
				runtime.EventsEmit(a.ctx, "scan_started")
			case events.HostDiscovered:
				data, ok := ev.Data.(events.HostDiscoveredData)
				if ok {
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
	}()

	err := a.engine.ScanStream(context.Background(), ips, cfg, eventChan)
	close(eventChan)
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

// GetLocalIPRange attempts to find the primary network interface and returns its CIDR.
func (a *App) GetLocalIPRange() string {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		return "192.168.1.0/24"
	}

	var fallback string

	for _, addr := range addrs {
		if ipnet, ok := addr.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
			ip := ipnet.IP.To4()
			if ip != nil {
				// Ignore APIPA (169.254.x.x)
				if ip[0] == 169 && ip[1] == 254 {
					continue
				}

				mask := ipnet.Mask
				network := net.IP{ip[0] & mask[0], ip[1] & mask[1], ip[2] & mask[2], ip[3] & mask[3]}
				ones, _ := mask.Size()
				cidr := fmt.Sprintf("%s/%d", network.String(), ones)

				// Prefer standard private IPs
				if ip[0] == 192 || ip[0] == 10 || ip[0] == 172 {
					return cidr
				}

				if fallback == "" {
					fallback = cidr
				}
			}
		}
	}

	if fallback != "" {
		return fallback
	}
	return "192.168.1.0/24"
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

	// Sanitizar e validar o caminho retornado pelo diálogo
	cleanPath := filepath.Clean(savePath)
	if cleanPath != savePath {
		return "", fmt.Errorf("caminho de arquivo inválido")
	}

	dir := filepath.Dir(cleanPath)
	if dir == "" || dir == "." {
		return "", fmt.Errorf("diretório de destino inválido")
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
