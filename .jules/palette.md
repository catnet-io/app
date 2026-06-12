## 2024-06-12 - Accessible Sortable Table Headers

**Learning:** When making `<th>` headers interactive for sorting in React/TypeScript, adding `role="button"` overrides the native `columnheader` role, which invalidates the `aria-sort` attribute for screen readers.

**Action:** To maintain accessibility, keep the native `<th>` element without overriding the role. Instead, manually add interactivity using `tabIndex={0}`, an `onKeyDown` handler (for `Enter` and `Space` keys), and the `aria-sort` attribute. Use CSS attribute selectors (e.g., `th[aria-sort]`) to apply interactive styles and focus outlines only to sortable headers.