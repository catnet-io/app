# Histórico de Pull Requests

Abaixo consta o histórico de todas as PRs analisadas para referência de troubleshooting:

### PR #31: build(deps-dev): bump typescript from 5.9.3 to 6.0.3 in /frontend
- **Status:** closed
- **Descrição:** Bumps [typescript](https://github.com/microsoft/TypeScript) from 5.9.3 to 6.0.3....

### PR #30: build(deps-dev): bump @types/react-dom from 18.3.7 to 19.2.3 in /frontend
- **Status:** closed
- **Descrição:** Bumps [@types/react-dom](https://github.com/DefinitelyTyped/DefinitelyTyped/tree/HEAD/types/react-do...

### PR #29: build(deps): bump react from 18.3.1 to 19.2.6 in /frontend
- **Status:** closed
- **Descrição:** Bumps [react](https://github.com/facebook/react/tree/HEAD/packages/react) from 18.3.1 to 19.2.6....

### PR #28: build(deps-dev): bump @vitejs/plugin-react from 4.7.0 to 6.0.2 in /frontend
- **Status:** closed
- **Descrição:** Bumps [@vitejs/plugin-react](https://github.com/vitejs/vite-plugin-react/tree/HEAD/packages/plugin-r...

### PR #27: build(deps): bump react-dom from 18.3.1 to 19.2.6 in /frontend
- **Status:** closed
- **Descrição:** Bumps [react-dom](https://github.com/facebook/react/tree/HEAD/packages/react-dom) from 18.3.1 to 19....

### PR #26: build(deps): bump actions/download-artifact from 4 to 8
- **Status:** open
- **Descrição:** Bumps [actions/download-artifact](https://github.com/actions/download-artifact) from 4 to 8....

### PR #25: build(deps): bump actions/upload-artifact from 4 to 7
- **Status:** open
- **Descrição:** Bumps [actions/upload-artifact](https://github.com/actions/upload-artifact) from 4 to 7....

### PR #24: build(deps): bump actions/setup-go from 5 to 6
- **Status:** open
- **Descrição:** Bumps [actions/setup-go](https://github.com/actions/setup-go) from 5 to 6....

### PR #23: build(deps): bump oven-sh/setup-bun from 1 to 2
- **Status:** closed
- **Descrição:** Bumps [oven-sh/setup-bun](https://github.com/oven-sh/setup-bun) from 1 to 2....

### PR #22: build(deps): bump actions/checkout from 4 to 6
- **Status:** closed
- **Descrição:** Bumps [actions/checkout](https://github.com/actions/checkout) from 4 to 6....

### PR #21: chore: integrate security scanners and resolve technical debts
- **Status:** closed
- **Descrição:** This PR merges recent develop changes into main:...

### PR #20: fix: resolve critical technical debts (tests, docs, frontend)
- **Status:** closed
- **Descrição:** This PR consolidates critical improvements to quality, documentation, and stability for the CatNet S...

### PR #19: 🛡️ Sentinel: [HIGH] Fix CSV Injection in Export
- **Status:** closed
- **Descrição:** 🚨 **Severity:** HIGH...

### PR #18: ⚡ Bolt: Cache static text measurements to reduce render loop overhead
- **Status:** closed
- **Descrição:** 💡 What: Moved `MeasureTextEx` calculations for static UI labels out of the main render loop and into...

### PR #17: 🎨 Palette: Add tooltips to icon-only buttons
- **Status:** closed
- **Descrição:** - 💡 What: Enabled global tooltips in Raygui and added tooltips to the icon-only 'Help' and 'Settings...

### PR #16: ⚡ Optimize Render Loop by Caching Text Measurements
- **Status:** closed
- **Descrição:** 💡 **What:** ...

### PR #15: 🧪 Add tests for SaveFileText shim
- **Status:** closed
- **Descrição:** 🎯 **What:** Added unit test coverage for the previously untested `SaveFileText` function in `src/ray...

### PR #14: ⚡ Perf: Optimize static text measurements in render loop
- **Status:** closed
- **Descrição:** 💡 **What:**...

### PR #13: 🧪 [Testing Improvement] Add unit test for scan_range edge case
- **Status:** closed
- **Descrição:** 🎯 **What:** The `scan_range` function in `src/scan.c` had a logical block that would return `0` if `...

### PR #12: 🔒 Fix Information Disclosure in Network Logs
- **Status:** closed
- **Descrição:** 🎯 **What:** Removed specific error codes (WSA and return codes) from being directly logged to `stder...

### PR #11: 🔒 Fix CSV Injection Vulnerability in Export Results
- **Status:** closed
- **Descrição:** 🎯 **What:** The vulnerability fixed...

### PR #10: 🧪 Add tests for scan_config_init
- **Status:** closed
- **Descrição:** 🎯 **What:** Tested `scan_config_init` function in `src/scan.c`. It initializes the default scan conf...

### PR #9: ⚡ Optimize O(N^2) string concatenation in render loop
- **Status:** closed
- **Descrição:** 💡 **What:** Replaced the `strncat` + `strlen` approach inside loops with direct `snprintf` calls tha...

### PR #8: ⚡ perf: optimize inefficient string concatenations in port lists
- **Status:** closed
- **Descrição:** 💡 **What:** Replaced inefficient string concatenations (`strncat` with repeated `strlen`) with a fas...

### PR #7: 🧪 Add unit test for scan_config_init
- **Status:** closed
- **Descrição:** 🎯 **What:** `src/scan.c` had missing tests, particularly for its default configuration logic in `sca...

### PR #6: 🧪 Add missing error tests in SaveFileText
- **Status:** closed
- **Descrição:** 🎯 **What:** This adds test coverage for `SaveFileText` in `src/raylib_shims.c` to handle missing fil...

### PR #5: 🧹 [Code Health] Remove dead `gui_log_snapshot` code from `src/main_raygui.c`
- **Status:** closed
- **Descrição:** 🎯 **What:** Removed the uninvoked `gui_log_snapshot` function from `src/main_raygui.c`....

### PR #4: 🧹 Remove dead code: teardown_state
- **Status:** closed
- **Descrição:** 🎯 **What:** Removed the unused static function `teardown_state` in `src/parallel_scan.c`. Added `<st...

### PR #3: 🧪 Add test coverage for raylib_shims.c
- **Status:** closed
- **Descrição:** 🎯 **What:** Missing tests for the Raylib compatibility shims (`src/raylib_shims.c`)....

### PR #2: UI improvements: Quick Tools toggle, collapsible panel, log clearing
- **Status:** closed
- **Descrição:** This PR brings several UI enhancements:...

### PR #1: Release/v0.2.0 docs
- **Status:** closed
- **Descrição:** ## Summary...

