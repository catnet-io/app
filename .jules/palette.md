## 2023-10-25 - Custom Table Header Accessibility
**Learning:** When turning non-interactive elements (like `<th>`) into sortable/interactive controls in this application, they do not inherently receive focus or keyboard events. They lack default focus outlines, making it confusing for keyboard users to navigate. Applying `role="button"` directly on `<th>` overrides the native `columnheader` role, confusing screen readers.
**Action:** When implementing custom interactivity on elements like `<th>`, manually assign `tabIndex={0}` to make it focusable, attach an `onKeyDown` handler to capture 'Enter' or 'Space' keys (and call `e.preventDefault()`), provide explicit focus visibility logic (e.g., `:focus-visible` CSS rules), and use ARIA properties like `aria-sort` to maintain semantic meaning without destroying native structure.

## 2023-10-25 - Status Indicator Accessibility
**Learning:** Purely visual CSS status indicators (like colored dots for 'Online' and 'Offline' states) are ignored by screen readers if they are implemented as empty `<span>` tags. Furthermore, sighted users might not immediately understand what the colors mean without a legend.
**Action:** Always add `role="img"` or `role="status"`, a descriptive `aria-label` (e.g., "Device is online"), and a native `title` tooltip (e.g., "Online") to purely visual status elements to make them accessible and user-friendly for all.

## 2023-10-25 - Confirmation Dialog for Destructive Actions
**Learning:** Destructive actions, like deleting scan history, lack a confirmation step, which can lead to accidental data loss. Furthermore, icon-only buttons in table rows (e.g., Export, Delete) are lacking explicit `aria-label` attributes, making them inaccessible to screen readers.
**Action:** Always include a `window.confirm` dialog or custom confirmation modal before executing destructive actions. Ensure icon-only buttons have descriptive `aria-label` attributes to provide context to assistive technologies.
