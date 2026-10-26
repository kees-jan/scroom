---
name: update-docker-containers
description: Update this repository's Ubuntu build containers for supported and future Ubuntu releases and verify they build Scroom. Use for the recurring task "Update docker containers".
---

Maintain Ubuntu build environments for Scroom. Include future/development Ubuntu releases once their official Docker images are available to catch API changes early; the goal is clean, modern, future-proof application code. Windows is part of the application's platform goals, but this workflow updates the Ubuntu containers in this repository.

## Repository conventions

- `Dockerfile.in` is the maintained template. Adjust it when build dependencies need changes; do not hand-edit generated Dockerfiles.
- Avoid release-specific differences where possible. If necessary, use an `if` in the template; its Git history contains examples.
- `supported-ubuntu-releases` lists Ubuntu release codenames, one per line. Include releases in Ubuntu standard support plus future/development releases whose official `ubuntu:<codename>` Docker image is available. Skip future releases whose images are not yet published; do not substitute another release or a moving tag. Exclude Jammy (22.04): the maintainer considers it too old. Once Jammy reaches the end of standard support (currently May 2027; verify when running), remove this exception and its cleanup reminder from this skill because the normal support rule then suffices.
- `regenerate-dockerfiles` regenerates every listed release's Dockerfile, creates new release directories, and stages generated Dockerfiles with `git add`.
- `test-dockerfiles SOURCE_DIRECTORY` builds images and then configures, builds, and tests Scroom from that source directory. Without this argument it only builds images. Application failures are reported per release and written to `<codename>.log`, but do not cause a nonzero script exit. Inspect every release's result and log.

## Workflow

1. Obtain the current `next` branch of this same repository in a temporary workspace, keeping the container checkout intact. Refresh it from origin before testing. Use a full Git checkout and initialize recursive submodules: Scroom needs Git version information and the `cmake-modules` submodule, so a plain `git archive` is insufficient. Use the same source revision for baseline and final validation.
2. Before changing the release list or containers, run `./test-dockerfiles SOURCE_DIRECTORY`. Verify all current images and Scroom builds/tests pass. If the baseline fails, stop and ask the user for help; do not start updating releases or fixing baseline container failures. Resolve execution permission barriers before classifying a run as a container failure.
3. Consult current authoritative Ubuntu release/lifecycle information and update `supported-ubuntu-releases` to supported and future/development releases. Use Ubuntu lifecycle information to determine support, and check official Docker image availability separately for future/development releases. A confirmed missing image means skip that future release; authentication or network errors do not establish that an image is unavailable. If standard support versus extended support affects the selection and the project convention does not resolve it, ask the user. When removing a codename, also delete its generated Dockerfile directory. New directories are created automatically.
4. Run `./regenerate-dockerfiles`.
5. Run `./test-dockerfiles SOURCE_DIRECTORY` again and inspect every release's result. If a failure exposes an API change that requires Scroom source changes, stop and ask the user for help; do not patch application code or hide the change with compatibility workarounds. For container environment/dependency failures, adjust `Dockerfile.in`, regenerate all Dockerfiles, and repeat validation until all pass. If progress requires user input, report the specific failure.
6. Review the resulting diff and report release additions/removals, template changes, the tested Scroom revision, and validation results. Do not claim success based solely on the script's exit code.

7. Commit the completed changes. Check recent Git history for message conventions; prefer a concise subject such as "Add Stonking, remove Questing". Credit Codex as the commit author. Push to GitHub, then verify the GitHub Actions build for that exact commit succeeds, including Scroom tests and container publication. If CI fails, inspect its logs and fix container issues within this workflow; stop and ask for help for application API changes. Report the commit and build URL.
