# CatNet Scanner - Functions and Modules Manual

## Index
- Architecture Overview
- Go Backend Modules
- React/Wails Frontend
- API Bindings

This manual describes the network diagnostic modules and the React-based GUI structure powered by Wails. The legacy C/Raylib version is obsolete.

## Architecture Overview
The application uses the Wails v2 framework. The backend is written in Go (handling highly concurrent network operations) and the frontend is written in React + TypeScript (providing the user interface). Communication happens via Wails IPC.

## Go Backend Modules (`pkg/scanner`)

### `net.go` (Network primitives)
- `func Ping(ip string, timeoutMs int) bool`
  - Sends a native ICMP ping. On Windows, it delegates to the OS `ping` executable with hidden windows to bypass the Raw Socket administrative constraint.
- `func ReverseDNS(ip string) string`
  - Resolves a hostname from an IP address.
- `func GetMAC(ip string) string`
  - Fetches the MAC address of an IP in the local subnet. Uses native Windows `iphlpapi.dll` and `SendARP` via Syscalls for high performance.
- `func ScanPorts(ip string, ports []int, timeoutMs int) []int`
  - Concurrently probes specified TCP ports using `net.DialTimeout`.

### `scan.go` (Concurrency & State)
- `type DeviceInfo`
  - Fields: IP, IsAlive, Hostname, MAC, OpenPortsCount, OpenPorts.
- `func StartScan(...)`
  - Orchestrates a parallel scan using Go channels and WaitGroups (`MaxThreads` goroutines). It emits progress and results back to the frontend synchronously via callbacks.
- `func StopScan()`
  - Cancels the context for the ongoing parallel scan.

### `utils.go` (Parsers)
- `func ParseRange(input string) ([]string, error)`
  - Parses CIDR notations (`192.168.1.0/24`) and dash-separated ranges (`192.168.1.1-254`). Generates a flat slice of target IPs.

## Frontend (React + Vite)
- **`App.tsx`**: Main component handling state (devices array, search inputs, active sorting).
- **`index.css`**: Defines the "Glassmorphism Cyberpunk" design system using CSS Variables.

## API Bindings (`app.go`)
The `App` struct exposes Go functions to the JS runtime:
- `StartScan`, `StopScan`, `ParseRange`, `ExportCSV`.
- The `ExportCSV` function triggers the native `SaveFileDialog` and writes the results locally without requiring browser hacks.