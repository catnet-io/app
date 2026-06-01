# Changelog

All notable changes to CatNet Scanner will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.4.1] - 2026-06-01

### Security
- Fixed race condition on `cancelScan` — now protected by `sync.Mutex`.
- Fixed XML injection in `ExportXML` via `encoding/xml` marshalling.
- Fixed CSV injection in `ExportCSV` via `encoding/csv` + `sanitizeCSVField`.
- Added `validateIPv4` gate to all public network functions.
- Added hard cap of 256 on `MaxThreads` in `StartScan`.
- Sanitized export file path with `filepath.Clean` + `filepath.Ext`.
- Validated `ScanConfig` fields in `app.go` before delegating to scanner.

### Added
- `pkg/exporter` package with `ExportJSON`, `ExportCSV`, `ExportXML`.
- `pkg/scanner/os_windows.go` and `os_posix.go` build-tag separation.
- Go test suite: `utils_test.go`, `net_test.go`, `scan_test.go`,
  `exporter_test.go`.
- CI workflow: `go vet` + `go test -race` on Windows with CGO.
- Release workflow: multiplataform binaries on tag push.
- `govulncheck` weekly security scan workflow.
- Snyk security scan workflow for Go and frontend dependencies.
- Dependabot for Go modules, npm/Bun, and GitHub Actions.
- `LICENSE` (MIT), `CONTRIBUTING.md`, `SECURITY.md`.
- Inline IP Range validation in frontend with accessibility attributes.
- `docs/ci_troubleshooting/` CI/CD debugging history.
- `CHANGELOG.md` (this file).

### Changed
- Export format detection uses `filepath.Ext` (case-insensitive).
- Default export format changed to JSON.
- Go toolchain: 1.23.0 → 1.25.10.
- `golang.org/x/crypto`: v0.33.0 → v0.52.0.
- `golang.org/x/net`: v0.35.0 → v0.54.0.
- `golang.org/x/sys`: v0.30.0 → v0.45.0.
- `golang.org/x/text`: v0.22.0 → v0.37.0.
- React: 18.3.1 → 19.2.6.
- Vite: 5.4.21 → 8.0.14.
- TypeScript: 5.9.3 → 6.0.3.
- `@vitejs/plugin-react`: 4.7.0 → 6.0.2.
- `tsconfig.json`: `moduleResolution` → `Bundler`, `esModuleInterop: true`.
- `validateIPv4`: loopback (127.x.x.x) now explicitly allowed.
- `scan_test.go`: `time.Sleep` replaced by `select`+`time.After` timeout.
- `wails.json`: author email anonymized to `contact@catnet-scanner.dev`.

### Removed
- `MANUAL.md` (consolidated into `ARCHITECTURE.md`).
- `docs/PR_DRAFT_v0.2.0.md`.
- `frontend/package.json.md5`.
- Commented-out local `replace` directive from `go.mod`.
- Broken `@font-face` Nunito reference from `style.css`.
- Dead `Activity` import from `App.tsx`.
- Inline CSV/XML/TXT formatting from `app.go`.

## [0.4.0] - 2026-05-31

### Added
- Initial public release: Go/Wails/React architecture.
- Parallel scan engine with goroutines.
- Cyberpunk glassmorphism UI with Nyan Cat progress bar.
- Smart auto-detect of primary network interface subnet.
- Export to JSON, CSV, and XML.
- Multiplataform release binaries via GitHub Actions.

[Unreleased]: https://github.com/mendsec/catnet_scanner/compare/v0.4.1...HEAD
[0.4.1]: https://github.com/mendsec/catnet_scanner/compare/v0.4.0...v0.4.1
[0.4.0]: https://github.com/mendsec/catnet_scanner/releases/tag/v0.4.0
