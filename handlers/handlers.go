package handlers

import (
	"context"
	"os"
	"path/filepath"

	"github.com/catnet-io/app/internal/store"
	"github.com/catnet-io/engine/pkg/scan"
	"github.com/wailsapp/wails/v2/pkg/runtime"
)

// AppHandlers orchestrates backend operations for the frontend.
type AppHandlers struct {
	ctx    context.Context
	engine *scan.Engine
	store  store.ScanStore
}

// NewAppHandlers creates a new AppHandlers instance.
func NewAppHandlers() *AppHandlers {
	return &AppHandlers{
		engine: scan.NewEngine(),
	}
}

// Startup initializes the handlers and database.
func (a *AppHandlers) Startup(ctx context.Context) {
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
