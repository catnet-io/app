import { test, expect } from '@playwright/test';

test('Verify table header accessibility and focus styles', async ({ page }) => {
  // Mock Wails API
  await page.addInitScript(() => {
    window.go = {
      main: {
        App: {
          GetLocalIPRange: async () => '192.168.1.1-254',
          ParseRange: async () => ['192.168.1.1', '192.168.1.2'],
          StartScan: async () => {},
          StopScan: () => {},
          ExportResults: async () => '/tmp/export.json',
        }
      }
    };
    window.runtime = {
      EventsOn: () => {},
      EventsOff: () => {},
      EventsOnMultiple: () => {},
      LogInfo: () => {},
      LogWarning: () => {},
      LogError: () => {},
    };
  });

  await page.goto('http://localhost:5173');

  // Focus on the first sortable header
  await page.keyboard.press('Tab'); // Need to tab a few times to get to the table
  await page.keyboard.press('Tab');
  await page.keyboard.press('Tab');
  await page.keyboard.press('Tab');
  await page.keyboard.press('Tab');

  // Try to find the hostname column and tab to it
  const hostnameHeader = page.locator('th:has-text("Hostname")');
  await hostnameHeader.focus();

  // Press Space to sort (should change text to include sort arrow and aria-sort)
  await page.keyboard.press('Space');

  // Assert attributes
  await expect(hostnameHeader).toHaveAttribute('aria-sort', 'ascending');

  // Press Enter to reverse sort
  await page.keyboard.press('Enter');

  // Assert reversed attributes
  await expect(hostnameHeader).toHaveAttribute('aria-sort', 'descending');

  // Take screenshot with focus visible
  await hostnameHeader.focus();
  await page.screenshot({ path: '/home/jules/verification/screenshots/table_focus.png' });
});
