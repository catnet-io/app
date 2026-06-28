## 2023-10-25 - Custom Table Header Accessibility
**Learning:** When turning non-interactive elements (like `<th>`) into sortable/interactive controls in this application, they do not inherently receive focus or keyboard events. They lack default focus outlines, making it confusing for keyboard users to navigate. Applying `role="button"` directly on `<th>` overrides the native `columnheader` role, confusing screen readers.
**Action:** When implementing custom interactivity on elements like `<th>`, manually assign `tabIndex={0}` to make it focusable, attach an `onKeyDown` handler to capture 'Enter' or 'Space' keys (and call `e.preventDefault()`), provide explicit focus visibility logic (e.g., `:focus-visible` CSS rules), and use ARIA properties like `aria-sort` to maintain semantic meaning without destroying native structure.

## 2023-10-25 - Status Indicator Accessibility
**Learning:** Purely visual CSS status indicators (like colored dots for 'Online' and 'Offline' states) are ignored by screen readers if they are implemented as empty `<span>` tags. Furthermore, sighted users might not immediately understand what the colors mean without a legend.
**Action:** Always add `role="img"` or `role="status"`, a descriptive `aria-label` (e.g., "Device is online"), and a native `title` tooltip (e.g., "Online") to purely visual status elements to make them accessible and user-friendly for all.

## 2023-10-25 - Icon-only Buttons and Destructive Actions
**Learning:** Icon-only buttons (like those for Export or Delete actions) often lack programmatic descriptions, causing accessibility issues for screen readers. Destructive actions without a confirmation mechanism can lead to accidental data loss.
**Action:** Consistently add `aria-label` attributes to icon-only buttons to convey their purpose programmatically. For destructive actions (like deleting a scan), implement an intermediate confirmation step (such as `window.confirm`) to ensure user intent.
