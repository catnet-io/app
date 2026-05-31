# CatNet Scanner

<p align="center">
  <img src="frontend/src/assets/nyan.png" width="200" alt="CatNet Scanner Logo">
</p>

**CatNet Scanner** is an incredibly fast and stylish network scanner, built for anyone who wants the agility of a command-line tool with the beauty of a Cyberpunk/SOC dashboard.

Built on the robust **Go** ecosystem and packaged via **Wails** using **React/TypeScript**, CatNet delivers massively parallel detection without UI lag. The goal? To offer a modern, high-performance alternative to classic tools like Angry IP Scanner—without Java, in a single standalone binary.

---

## 🌟 Features (v0.4.0)

- ⚡ **Brutally Parallel Scan**: Built entirely with Goroutines, scanning entire subnets in a fraction of a second.
- 🎨 **Cyberpunk UI**: A modern React interface using translucent glass (*Glassmorphism*), neon lines, responsive grids, and subtle animations. And of course, with Nyan Cat pulling your progress bar.
- 📡 **Smart Auto-Detect**: Instantly loads the `/24` subnet of your main network interface on startup.
- 💾 **Practical Export**: Save reports in `CSV`, `TXT`, or `XML` format with a single click.
- 🛠️ **Decoupled Architecture**: Secure Go network engine and rich, decoupled React UI, packaged into a single standalone `.exe` file.

## 🚀 Upcoming Features

Check our [`ROADMAP.md`](ROADMAP.md) to see where we're heading. Features like Scan Profiles, Custom Openers (SSH, RDP), and Vendor OUI Resolution are already in the works.

## ⚙️ Building and Development

CatNet Scanner uses Wails. You don't need to install heavy React or Node dependencies if you only want to compile the backend.

### Prerequisites
- [Go 1.20+](https://go.dev/dl/)
- [Bun](https://bun.sh/) (fast frontend package manager)
- [Wails CLI](https://wails.io/docs/gettingstarted/installation)

### Installing Wails
```bash
go install github.com/wailsapp/wails/v2/cmd/wails@latest
```

### Running in Development Environment (Live Reload)
To modify the UI or Go code with live-reload, use:
```bash
wails dev
```

### Generating the Production Executable
To compile the final standalone version, run:
```bash
wails build -clean
```
The final executable will be available at `build/bin/catnet.exe`.

---

## 🤝 Contributing
Suggestions are welcome. Feel free to open Issues for new tools you'd like to see in the "Quick Tools" tab or report UI bugs.

> Copyright © 2026 Mendsec. Created by Fábio Mendes.
