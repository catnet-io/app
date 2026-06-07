## 2024-06-07 - Make sortable table headers keyboard accessible
**Learning:** In custom tables, making headers sortable via `onClick` often leaves out keyboard users. Adding `tabIndex={0}`, handling Enter/Space keys in `onKeyDown`, and using `aria-sort` are crucial for table a11y.
**Action:** When implementing sortable columns in custom UI components, always ensure they are fully navigable and operable via keyboard, and clearly indicate focus state using `:focus-visible`.
