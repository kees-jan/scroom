---
name: commits-and-merge-requests
description: Apply Scroom's authorship and CI verification requirements when committing changes, pushing to GitHub, or creating or updating merge requests.
---

# Commits and merge requests

## Commit identity

Create commits as OpenAI Codex <codex@openai.com>, using that identity for both
Git author and committer.

## CI after pushing

After pushing to GitHub, including updates to a merge request, wait for the CI
build and relevant merge-request checks for the latest pushed revision to finish.
Inspect failures, fix their causes, commit and push the corrections, and verify
CI again until it succeeds. Keep corrections within the authorized task scope.

If verification or repair is blocked by access, infrastructure, or a decision
requiring user input, report the blocker and the outstanding checks. Report CI
success only once the latest revision's checks have passed.
