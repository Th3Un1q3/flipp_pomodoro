# Flipp Pomodoro — agent guide

Flipper Zero application (FAP) written in C against the furi and gui APIs. The build runs in the dev container with ufbt; the application source is `flipp_pomodoro/`.

## Layout

| Path | Contents |
|---|---|
| `flipp_pomodoro/application.fam` | FAP manifest: app id, entry point, required records, icon |
| `flipp_pomodoro/*.c`, `*.h` | App lifetime, event callbacks, view wiring |
| `flipp_pomodoro/modules/` | Timer state, settings persistence, statistics |
| `flipp_pomodoro/scenes/` | Scene handlers, registered in `scenes/config/flipp_pomodoro_scene_config.h` |
| `flipp_pomodoro/views/` | Custom `View` implementations: timer, config, info |
| `flipp_pomodoro/helpers/` | Time, notifications, notification manager |
| `flipp_pomodoro/images/` | Icon assets behind `fap_icon_assets` |
| `tools/dev/` | Build and verify scripts used by the container |
| `dist/` | Built `.fap` artifacts (gitignored) |

## Build and verify

| Command | Result |
|---|---|
| `just build` | standard-firmware FAP in `dist/flipp_pomodoro_standard_<api>.fap` |
| `just build unleashed` | Unleashed FAP |
| `just build-all` | both channels |
| `just verify` | tool, SDK, and clean-build gate |
| `bash tools/dev/build.sh <channel>` | same build without `just` |

`just verify` must exit 0 before a change is reported as done. Name the channel you built; a change that touches platform APIs builds for both.

Build only through `tools/dev/build.sh` (or its `just` recipes). The legacy `fbt` driver and the firmware submodules have been retired.

## SDK channels

ufbt keeps one SDK per state directory:

| Channel | State directory | Source |
|---|---|---|
| standard | `~/.ufbt/standard` | official release channel |
| unleashed | `~/.ufbt/unleashed` | `https://up.unleashedflip.com/directory.json` |

A bare `ufbt` uses `UFBT_HOME`, which the container sets to `~/.ufbt/standard`. `tools/dev/sdk.sh` is the single place that selects the directory per channel and applies the Unleashed index URL; `tools/dev/build.sh` calls it.

Refresh Unleashed through `bash tools/dev/sdk.sh unleashed --force` (or `just sdk-unleashed`); a bare `ufbt update` would replace it with the official SDK.

## Code conventions

- C17, 4-space indent, braces on their own line.
- Headers use `#pragma once`.
- Public module functions take the `flipp_pomodoro__` prefix, view functions `flipp_pomodoro_view_*`, scene handlers `flipp_pomodoro_scene_<name>_*`.
- A new scene is one `ADD_SCENE(...)` line in `scenes/config/flipp_pomodoro_scene_config.h` plus `scenes/flipp_pomodoro_scene_<name>.c` with its on_enter, on_event, and on_exit handlers.
- A platform service the app uses belongs in `requires` in `application.fam`; `APPCHK` rejects symbols outside the declared set.
- `furi_assert` every context pointer before dereferencing it.

## Boundaries

- Build only. `ufbt flash` and `ufbt launch` need a USB device passed into the container; treat it as absent and leave hardware steps to the maintainer.
- Do not commit `dist/`, `.ufbt/`, or generated editor files; they are gitignored.
- Do not reformat files you did not otherwise change.
- Keep the release workflow's build step in the dev container, pointing at `tools/dev/build.sh`.
