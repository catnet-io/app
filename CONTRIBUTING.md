# Contributing

Thank you for your interest in contributing to CatNet.

## Before you start
- Open an issue before large changes.
- Keep changes focused and small.
- Align with repository scope.
- Prefer incremental pull requests.

## Pull request checklist
- Code builds successfully.
- Tests pass locally.
- Documentation is updated when applicable.
- No unrelated files were changed.
- The PR description explains what changed and why.

## Scope rule
UI repositories must not duplicate core scanning logic.

## DevSecOps Guide

### CI/CD and GitHub Actions Standards

1. **Pin third-party actions by commit SHA**
   - Every `uses:` directive referencing a third-party action must use a full commit SHA (40 hex chars), optionally followed by a `# vX.Y.Z` comment for readability.
   - Floating tags (`@v1`, `@v2`, `@master`) are not permitted.
   - Dependabot is configured to open PRs for action updates — reviewers must ensure SHA pinning is maintained.

2. **Explicit workflow permissions**
   - Every workflow file must declare a top-level `permissions:` block.
   - Use the least-privilege scope:
     ```yaml
     permissions:
       contents: read
     ```
   - Only jobs that need write access (e.g., creating releases) should override this.

3. **Security scanning baseline**
   The following tools are mandatory in CI:
   - **Govulncheck** — Go vulnerability scanning
   - **Snyk** — Go and npm dependency scanning
   - **Semgrep** — SAST for Go and TypeScript
   - **Dependabot** — automated dependency bump PRs

4. **Avoid dangerous triggers**
   - `pull_request_target` must not be used unless absolutely necessary, and only with explicit checkout of the base ref.
   - `schedule` triggers on security scans should run at most once per day.

5. **Build matrix and caching**
   - Use build matrices for cross-platform releases (already done in `release.yml`).
   - Cache Go modules (`actions/cache` or `setup-go cache`) to reduce CI time.
   - Cache bun dependencies similarly.

### Frontend / Accessibility Standards

1. **Accessibility checklist**
   - All interactive elements must be keyboard accessible (`tabIndex`, `onKeyDown` for Enter/Space).
   - Sortable table headers must include `aria-sort` (`ascending` / `descending` / `none`).
   - Do not override native ARIA roles (e.g., `role="button"` on `<th>` removes `columnheader`).
   - Focus indicators must use `:focus-visible` (not `:focus`) to avoid cluttering mouse users.
   - Color contrast must meet WCAG AA (4.5:1 for normal text, 3:1 for large text).
   - Error states must include `aria-invalid`, `aria-describedby`, and `role="alert"` where appropriate.

2. **Reusable components**
   - Do not duplicate accessibility logic across inline JSX. Extract shared patterns into components under `frontend/src/components/`.
   - Each component must have a corresponding unit test file.

### General PR Standards

1. **PR size**
   - Maximum recommended: ~300 lines changed. Larger PRs should be split into logical feature branches.
   - Exceptions: dependency bumps (can be mechanical across many files).

2. **Branch naming**
   - `feature/<description>` for new features
   - `fix/<description>` for bug fixes
   - Avoid machine-generated random suffixes in branch names. Use semantic, human-readable names.
