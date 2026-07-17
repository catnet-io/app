package handlers

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/catnet-io/engine/pkg/export"
	"github.com/catnet-io/engine/pkg/results"
	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// ExportResults asks the user for a save location and exports the results
func (a *AppHandlers) ExportResults(devices []results.HostResult) (string, error) {
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

	// Sanitize and validate the path returned by the dialog
	cleanPath := filepath.Clean(savePath)
	if cleanPath != savePath {
		return "", fmt.Errorf("invalid file path")
	}

	dir := filepath.Dir(cleanPath)
	if dir == "" || dir == "." {
		return "", fmt.Errorf("invalid destination directory")
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
