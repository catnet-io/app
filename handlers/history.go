package handlers

import (
	"fmt"

	"github.com/catnet-io/app/internal/diff"
	"github.com/catnet-io/app/internal/store"
	"github.com/catnet-io/engine/pkg/results"
)

// GetScans returns the history of scans
func (a *AppHandlers) GetScans() ([]store.ScanSummary, error) {
	if a.store == nil {
		return nil, fmt.Errorf("database not initialized")
	}
	return a.store.GetScans()
}

// GetScanReport returns the details of a specific scan
func (a *AppHandlers) GetScanReport(scanID int64) (*results.ScanReport, error) {
	if a.store == nil {
		return nil, fmt.Errorf("database not initialized")
	}
	return a.store.GetReport(scanID)
}

// DeleteScan removes a scan from history
func (a *AppHandlers) DeleteScan(scanID int64) error {
	if a.store == nil {
		return fmt.Errorf("database not initialized")
	}
	return a.store.DeleteScan(scanID)
}

// CompareScans compares two scans and returns the differences
func (a *AppHandlers) CompareScans(oldID, newID int64) ([]diff.HostDiff, error) {
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
