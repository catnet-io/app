
## 2024-06-09 - Sortable Table Header Accessibility
**Learning:** Interactive custom components like `.cyber-table` sortable headers need manual accessibility features. Relying purely on `onClick` omits keyboard and screen reader users.
**Action:** When creating sortable columns or interactive non-button elements, always implement `tabIndex={0}`, `onKeyDown` (for Enter/Space), `:focus-visible` styling, and appropriate ARIA attributes like `aria-sort`.
