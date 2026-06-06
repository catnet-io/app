
## 2024-05-24 - Interactive Table Headers Accessibility
**Learning:** When creating custom interactive table headers (e.g. for sorting columns) that use `onClick`, they are opaque to screen readers and keyboard users. Native `<th>` elements do not receive focus or trigger events for Space/Enter natively.
**Action:** Always add `tabIndex={0}` and an `onKeyDown` handler (for `Enter` and `Space` keys) to make them operable via keyboard. Additionally, add `aria-sort` (with values "ascending", "descending", or "none") and visual focus indicators (`:focus-visible` styling) so users know they are interactive and understand their current state.
