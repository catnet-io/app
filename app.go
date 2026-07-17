package main

import (
	"context"

	"github.com/catnet-io/app/handlers"
)

// App struct is the main binder for Wails.
type App struct {
	*handlers.AppHandlers
}

// NewApp creates a new App application struct.
func NewApp() *App {
	return &App{
		AppHandlers: handlers.NewAppHandlers(),
	}
}

// startup is called when the app starts. The context is saved
// so we can call the runtime methods.
func (a *App) startup(ctx context.Context) {
	a.AppHandlers.Startup(ctx)
}
