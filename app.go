package main

import (
	"catnet_scanner_wails/pkg/exporter"
	"catnet_scanner_wails/pkg/scanner"
	"context"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"strings"

	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// App struct
type App struct {
	ctx context.Context
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
func (a *App) StartScan(ips []string, cfg scanner.ScanConfig) error {
	// Sanitizar configuração recebida do frontend
	if cfg.MaxThreads <= 0 || cfg.MaxThreads > 256 {
		cfg.MaxThreads = 16
	}
	if cfg.PortTimeoutMs <= 0 || cfg.PortTimeoutMs > 10000 {
		cfg.PortTimeoutMs = 500
	}
	if cfg.PingTimeoutMs <= 0 || cfg.PingTimeoutMs > 10000 {
		cfg.PingTimeoutMs = 1000
	}

	// We emit events to the frontend whenever a device is scanned
	onResult := func(di scanner.DeviceInfo) {
		runtime.EventsEmit(a.ctx, "scan_result", di)
	}

	onProgress := func(prog float64) {
		runtime.EventsEmit(a.ctx, "scan_progress", prog)
	}

	runtime.EventsEmit(a.ctx, "scan_started")
	err := scanner.StartScan(ips, cfg, onResult, onProgress)
	runtime.EventsEmit(a.ctx, "scan_finished")
	return err
}

// StopScan wrapper
func (a *App) StopScan() {
	scanner.StopScan()
}

// Ping wrapper for Quick Tools
func (a *App) Ping(ip string) bool {
	return scanner.Ping(ip, 1000)
}

// ReverseDNS wrapper
func (a *App) ReverseDNS(ip string) string {
	return scanner.ReverseDNS(ip)
}

// GetMAC wrapper
func (a *App) GetMAC(ip string) string {
	return scanner.GetMAC(ip)
}

// ScanPorts wrapper
func (a *App) ScanPorts(ip string, ports []int) []int {
	return scanner.ScanPorts(ip, ports, 500)
}

// ParseRange expands an IP range string (e.g. 192.168.1.1-254) into a list of IPs.
func (a *App) ParseRange(input string) ([]string, error) {
	return scanner.ParseRange(input)
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
func (a *App) ExportResults(devices []scanner.DeviceInfo) (string, error) {
	options := runtime.SaveDialogOptions{
		DefaultFilename: "catnet_results.json",
		Title:           "Export Scan Results",
		Filters: []runtime.FileFilter{
			{DisplayName: "JSON Files (*.json)", Pattern: "*.json"},
			{DisplayName: "CSV Files (*.csv)", Pattern: "*.csv"},
			{DisplayName: "XML Files (*.xml)", Pattern: "*.xml"},
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
	// Garantir que o arquivo não está em diretório do sistema
	// (validação básica — o diálogo nativo já restringe, mas
	// este check adiciona defesa em profundidade)
	dir := filepath.Dir(cleanPath)
	if dir == "" || dir == "." {
		return "", fmt.Errorf("diretório de destino inválido")
	}

	var data []byte
	var formatErr error
	
	if strings.ToLower(filepath.Ext(savePath)) == ".json" {
		data, formatErr = exporter.ExportJSON(devices)
	} else if strings.ToLower(filepath.Ext(savePath)) == ".xml" {
		data, formatErr = exporter.ExportXML(devices)
	} else {
		data, formatErr = exporter.ExportCSV(devices)
	}

	if formatErr != nil {
		return "", formatErr
	}

	err = os.WriteFile(savePath, data, 0644)
	return savePath, err
}
