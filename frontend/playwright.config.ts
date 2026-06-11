import { defineConfig, devices } from '@playwright/test';

export default defineConfig({
  testDir: './',
  testMatch: 'verify_ui.spec.ts',
  use: {
    video: 'on',
  },
  webServer: {
    command: 'bun run dev',
    port: 5173,
    reuseExistingServer: true,
  },
});
