package handlers

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/catnet-io/engine/pkg/events"
	"github.com/catnet-io/engine/pkg/profile"
	"github.com/catnet-io/engine/pkg/results"
	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// StartScan wrapper for frontend
func (a *AppHandlers) StartScan(ips []string, cfg profile.ScanProfile) error {
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
func (a *AppHandlers) StopScan() {
	if a.engine != nil {
		a.engine.Stop()
	}
}
