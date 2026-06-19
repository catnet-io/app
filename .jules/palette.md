# Palette's UX Journal

## 2026-06-19 - [Accessibility] Accessible Table Sort Headers
**Learning:** React elements serving as interactive table headers (e.g., `<th>` with an onClick handler for sorting) require manual implementation of keyboard accessibility properties including `tabIndex={0}` and an `onKeyDown` handler to actuate actions on 'Enter' and 'Space'. Applying semantic `aria-sort` attributes correctly is necessary, as is adding `:focus-visible` styling specifically for the `<th>` tags so that visual focus representation occurs naturally without conflicting custom CSS.
**Action:** When adding interactivity to non-interactive elements, ensure comprehensive keyboard handlers, tabindexing, visible focus styling, and correct semantic ARIA properties are defined immediately.
