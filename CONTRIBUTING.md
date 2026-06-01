# Contributing to CatNet Scanner

First off, thank you for considering contributing to CatNet Scanner! It's people like you that make CatNet such a great tool.

## Where do I go from here?

If you've noticed a bug or have a feature request, make sure to check our [Issues](../../issues) if one already exists. If not, go ahead and create a new issue!

## Building locally

CatNet Scanner is built with **Go**, **Wails**, and **React**.

1. **Install Prerequisites**:
   - Go (1.23+)
   - Bun (for frontend dependencies)
   - Wails CLI (`go install github.com/wailsapp/wails/v2/cmd/wails@latest`)
2. **Run Development Mode**:
   ```bash
   wails dev
   ```
   This will start the frontend development server and compile the Go backend, connecting them automatically.
3. **Build Production Binary**:
   ```bash
   wails build
   ```

## Pull Request Process

1. Fork the repo and create your branch from `develop`.
2. If you've added code that should be tested, add tests.
3. If you've changed APIs, update the documentation.
4. Ensure the test suite passes (`go test ./...`).
5. Issue that pull request!

## Code Style

- Go: We use standard `gofmt`.
- React/TypeScript: We use `eslint` and `prettier`. Keep components small and functional.

Thank you!
