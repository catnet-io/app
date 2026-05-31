## 2026-05-31 - Pre-calculate static text measurements
**Learning:** In immediate-mode GUIs like Raygui, calling MeasureTextEx for static UI labels inside the render loop causes significant CPU overhead because it forces font glyph lookups every frame.
**Action:** Always pre-calculate and cache static text measurements (using MeasureTextEx) outside of the main loop (e.g. before `while (!WindowShouldClose())`) and use the cached Vector2 structs inside the render loop.
