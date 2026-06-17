## 2024-06-17 - Keyboard Accessible Table Headers
**Learning:** When making custom React table headers (`<th>`) interactive for sorting, adding `tabIndex={0}`, `onKeyDown` (for Enter/Space), and `aria-sort` is required. Do not override the native `columnheader` role with `role="button"`, as it breaks the `aria-sort` association.
**Action:** Always include keyboard handlers and appropriate ARIA attributes without modifying the implicit role when enhancing native semantic elements like `<th>`.
