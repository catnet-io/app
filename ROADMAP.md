# Roadmap - CatNet Scanner

The **CatNet Scanner** is undergoing a massive evolution. After stabilizing the new core architecture in **Go + Wails + React**, replacing the old C stack, the software has entered its maturation phase to become the most stylish, robust, and efficient network scanner for the corporate and enthusiast market.

This document traces the path from current stabilization to competitive commercial consolidation (v1.0.0).

---

## 🎯 v0.4.0 — Stabilization & Foundation (Current Release)
**Goal**: Change the base architecture, stabilize the GUI, and ensure brutal speed.

- [x] **Concurrent Scan Engine**: Rewrite ping/ICMP engine using native `Goroutines`, achieving massive parallelism.
- [x] **Cyberpunk UI/UX**: Abandon rigid TUI interfaces for a modern, high-contrast look (*glassmorphism*, neon), built in React and TypeScript.
- [x] **Decoupled Architecture**: Implement Wails to serve the compiled binary without depending on external Node.js servers.
- [x] **Auto-detect IP Range**: Smart and automatic detection of the active interface's subnet (`/24`).
- [x] **Base Export**: Export scans in JSON, CSV, and XML formats.
- [x] **Table Sorting (Sort)**: Click on column headers (IP, Status, Hostname) to sort data alphabetically or numerically.

---

## 🛠️ v0.5.0 — Usable Product (Next Sprints)
**Goal**: Expand essential day-to-day features that SysAdmins already expect.

- [ ] **Scan Profiles**: Allow saving "Favorite Ranges" for 1-click access (e.g., Guest Network, Servers Network).
- [ ] **Vendor OUI Lookup**: Resolve network card manufacturers offline based on the first octets of the MAC Address.
- [ ] **Host Side Panel**: Upon clicking a specific IP in the main table, open a side/bottom panel displaying more details (full MAC, ping rate, manufacturer).
- [ ] **Audio Feedback (Optional)**: Small discrete sounds for scan started and finished (cyberpunk theme).

---

## 🚀 v0.6.0 — Differentiation and Power User Tools
**Goal**: Provide fast productivity features to dethrone native OS scanners.

- [ ] **Custom Openers**: Real integration to invoke system calls on the target IP with just one click:
  - SSH (`ssh user@ip`)
  - RDP (`mstsc /v:ip`)
  - HTTP/HTTPS (open in default browser)
- [ ] **Basic Port Identification**: Option to quickly probe 5-10 common ports (80, 443, 22, 3389) in the background without the slowness of Nmap.
- [ ] **NetBIOS/SMB Enrichment**: Pull NetBIOS names and workgroups in Windows local networks.
- [ ] **Dynamic Filters**: Toggle buttons for "Only alive hosts" or "Hide hosts without a name".
- [ ] **Local History and Comparison**: Visually identify if new hosts appeared or vanished compared to the last scan on the same network.

---

## 👑 v1.0.0 — "The Angry IP Killer" (Competitive Release)
**Goal**: Production-level packaging, corporate extensibility, and focused marketing.

- [ ] **Plugin API (Extensibility)**: Allow injection of small Go scripts or modules for exotic detection tasks (e.g., check SNMP response, test default credentials).
- [ ] **Exhaustive Documentation**: Comprehensive user manuals and troubleshooting guides.
- [ ] **Multi-platform Releases**: Automated installers via CI/CD (.MSI for Windows, .APP/DMG for macOS, AppImage for Linux).
- [ ] **Official Benchmark**: Documentation page publishing load tests proving that CatNet achieves superior performance (Goroutines vs the Java competitor's limited Threads).
- [ ] **Landing Page**: Institutional website for product download with UI demonstration.
