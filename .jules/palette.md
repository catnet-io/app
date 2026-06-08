## 2024-06-08 - Keyboard Accessible Sortable Table Headers
**Learning:** Custom sortable table headers (like those in `.cyber-table`) lack built-in accessibility features such as keyboard navigation and screen reader state indication.
**Action:** Always add keyboard support (`tabIndex={0}`, `onKeyDown` handling 'Enter'/'Space'), focus styling (`:focus-visible`), and proper ARIA attributes (`role="columnheader"`, `aria-sort`) to custom sortable table headers.
