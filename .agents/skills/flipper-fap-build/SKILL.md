---
name: flipper-fap-build
description: Build, verify, and troubleshoot the Flipp Pomodoro .fap for the standard or Unleashed firmware channel in the dev container. Use when a change must be compiled, when a build fails with API, APPCHK, or toolchain errors, or when a release artifact has to be produced.
---

# Flipper FAP build

## Workflow

1. Pick the channel: `standard` for official firmware, `unleashed` for DarkFlippers. A change that touches platform APIs builds for both.
2. Run `bash tools/dev/build.sh <channel>`. It selects `~/.ufbt/<channel>`, calls `tools/dev/sdk.sh` to bootstrap the SDK when absent, builds, and copies the result to `dist/flipp_pomodoro_<channel>_<api>.fap`.
3. Read the `Target: 7, API: <version>` line from the output. Record the API when the artifact is for release.
4. Confirm the artifact exists and is non-empty with `stat -c %s`. A missing or zero-byte file is a failed build even when ufbt exits 0.
5. Leave flashing to a host with the device attached.

## Failures

| Symptom | Cause | Fix |
|---|---|---|
| Device rejects the artifact | Built against a different firmware API | Rebuild for the channel that matches the device firmware; compare the `API:` line |
| Unleashed build reports the official API | A `ufbt update` ran without `--index-url` in that state directory | `bash tools/dev/sdk.sh unleashed --force` (or `just sdk-unleashed`), then rebuild |
| `APPCHK` rejects a symbol | The manifest lacks a required API group | Add it to `requires` in `flipp_pomodoro/application.fam` |
| Toolchain binary missing | SDK bootstrap incomplete or state corrupted | `UFBT_HOME=~/.ufbt/<channel> ufbt clean`, then rerun the build script |
| Icon conversion fails | An asset named by `fap_icon_assets` is missing or unreadable | Restore the asset under `flipp_pomodoro/images/` |

## Rules

- Build both channels before producing release artifacts.
- Never edit `dist/`; it is generated.
- Build only through `tools/dev/build.sh`; the legacy `fbt` driver is gone.
