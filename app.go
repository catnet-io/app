package main

import (
	"context"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"strings"

	"github.com/mendsec/catnet-core/pkg/engine"
	"github.com/mendsec/catnet-core/pkg/exporter"
	"github.com/mendsec/catnet-core/pkg/results"
	"github.com/mendsec/catnet-core/pkg/targets"
	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// App struct
type App struct {
	ctx    context.Context
	cancel context.CancelFunc
}

// NewApp creates a new App application struct
func NewApp() *App {
	return &App{}
}

// startup is called when the app starts. The context is saved
// so we can call the runtime methods
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx
}

// StartScan wrapper for frontend
func (a *App) StartScan(ips []string, cfg engine.ScanConfig) error {
	// Defensively cancel any existing scan
	a.StopScan()

	var ctx context.Context
	ctx, a.cancel = context.WithCancel(context.Background())

	go func() {
		engine.StartScan(ctx, ips, cfg, func(event engine.ScanEvent) {
			switch event.Type {
			case engine.EventLifecycleStart:
				runtime.EventsEmit(a.ctx, "scan_started")
			case engine.EventResult:
				if event.Device != nil {
					runtime.EventsEmit(a.ctx, "scan_result", event.Device)
				}
			case engine.EventProgress:
				runtime.EventsEmit(a.ctx, "scan_progress", event.Progress)
			case engine.EventLifecycleComplete, engine.EventLifecycleCancel:
				runtime.EventsEmit(a.ctx, "scan_finished")
			}
		})
	}()

	return nil
}

// StopScan wrapper
func (a *App) StopScan() {
	if a.cancel != nil {
		a.cancel()
		a.cancel = nil
	}
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
func (a *App) ExportResults(devices []results.DeviceInfo) (string, error) {
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

	report := &results.ScanReport{
		Devices: devices,
		Total: len(devices),
	}

	if strings.ToLower(filepath.Ext(savePath)) == ".json" {
		data, formatErr = exporter.ExportJSON(report)
	} else {
		data, formatErr = exporter.ExportCSV(report)
	}

	if formatErr != nil {
		return "", formatErr
	}

	err = os.WriteFile(savePath, data, 0644)
	return savePath, err
}
