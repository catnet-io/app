## 2024-05-18 - Prevent Default on Space Keydown handlers
**Learning:** When adding keyboard accessibility to interactive components like table headers that use custom `onKeyDown` handlers for `Enter` and `Space` keys, failing to call `e.preventDefault()` on the spacebar keydown event allows the default browser behavior (scrolling the page down) to trigger, ruining the user experience.
**Action:** Always include `e.preventDefault()` in custom keydown handlers for the `Space` key on interactive custom UI components.
