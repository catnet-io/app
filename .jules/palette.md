## 2025-02-23 - Handle asynchronous state in empty components
**Learning:** Tables or lists should provide clear visual feedback when an asynchronous operation (like network scanning) is active, otherwise the "empty state" message can mislead users into thinking the application is idle or stuck.
**Action:** When creating empty states, check if there is an active loading or processing state, and display a specific "Loading..." or "Processing..." message instead of the default "Ready" or "No results" message.
