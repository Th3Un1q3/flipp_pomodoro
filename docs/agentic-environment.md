# Agentic development environment

The repository builds inside a Dev Container that carries the C toolchain, ufbt, both Flipper SDK channels, and OpenCode.

## What the image provides

| Layer | Contents |
|---|---|
| Base | `mcr.microsoft.com/devcontainers/base:noble`, user `vscode` |
| OS packages | `build-essential`, `file`, `jq`, `libusb-1.0-0`, `pipx`, `python3`, `shellcheck`, `unzip`, `xz-utils` |
| Dev Container Features | git, GitHub CLI, Node 22, `just` |
| Python tool | `ufbt`, installed with `pipx` |
| Agent | OpenCode, installed by `https://opencode.ai/v2/install` into `~/.opencode/bin` |

ufbt downloads its own Python and `arm-none-eabi` toolchain on first use, so the distro Python only bootstraps it.

## Persistent volumes

| Volume | Mount | Holds |
|---|---|---|
| `flipp-ufbt` | `/home/vscode/.ufbt` | SDKs and toolchains for both channels |
| `flipp-opencode-data` | `/home/vscode/.local/share/opencode` | Agent sessions and provider credentials |

`docker volume rm flipp-ufbt` forces a clean SDK download on the next start.

## SDK channels

| Channel | State directory | Source |
|---|---|---|
| standard | `~/.ufbt/standard` | official firmware release channel |
| unleashed | `~/.ufbt/unleashed` | `https://up.unleashedflip.com/directory.json` |

`UFBT_HOME` defaults to the standard directory, so a bare `ufbt` builds the standard channel. `tools/dev/build.sh` sets the directory per channel through `tools/dev/sdk.sh`, the only script that runs `ufbt update`.

Updating the Unleashed directory without `--index-url` replaces it with the official SDK. Use `just sdk-unleashed` (or `bash tools/dev/sdk.sh unleashed --force`), which passes the URL.

## API change detection

Firmware APIs are no longer tracked with git submodules. `.github/workflows/sdk-drift.yml` runs nightly and can also be started manually with **Actions → SDK drift → Run workflow**. It refreshes both SDKs, rebuilds both channels inside the dev container, and compares the deployed firmware versions and APIs with the tracked JSON baseline in `tools/dev/sdk-baseline.json`. On a change it opens a pull request that updates that JSON; `ci.yml` gates the PR, and merging it runs `release.yml`. The release rebuilds both channels and re-checks the baseline with `tools/dev/check-sdk-drift.sh --check-only`, so it cannot publish FAPs built against an unrecorded SDK.

A no-drift run exits successfully, creates no pull request, and writes a **No SDK drift detected** job summary; it does not fail. A tool or SDK failure fails the job. `tools/dev/test-sdk-drift.sh` (or `just drift-test`) exercises no-drift, drift, check-only, and schema-failure paths with synthetic SDK state. `docs/environment-baseline.md` is background for humans and is not parsed by automation. This preserves the old Dependabot submodule-bump trigger without keeping multi-gigabyte firmware trees in the repository.

## Dependency updates

`.github/dependabot.yml` checks weekly for updates to workflow actions, public Dev Container Features, and the Dockerfile base image. Action updates are grouped into one PR; Feature updates also refresh `devcontainer-lock.json`. Pull request titles use `chore(deps):` so squash merging them to `master` does not create a SemVer release. `ci.yml` watches `.github/workflows/**` and `.github/dependabot.yml` so dependency-only PRs still run the build gate.

`ufbt` is pinned by `UFBT_VERSION`, OpenCode is installed by a shell installer, apt packages are unpinned, and firmware SDKs come from their release indexes. Those are not Dependabot ecosystems; SDK changes are handled by the drift workflow above.

## Commands

| Command | Result |
|---|---|
| `just verify` | tool, SDK, drift-check, and clean-build gate |
| `just drift-test` | synthetic drift-detection checks |
| `just build` | standard FAP in `dist/` |
| `just build unleashed` | Unleashed FAP in `dist/` |
| `just build-all` | both channels |
| `just sdk-unleashed` | refresh the Unleashed SDK only |
| `just lint` | shellcheck the build and dev-container scripts (needs the image that includes shellcheck) |

The agent loads `AGENTS.md` for repository rules and `.agents/skills/flipper-fap-build/` for the build procedure.

## Limits

- The container builds; it does not flash. `ufbt flash` and `ufbt launch` need `/dev/ttyACM*` from the host, which a Dev Container does not receive by default.
- The Unleashed SDK is a third-party index, so its API can lag official firmware. Artifacts are named after the deployed firmware version, the build prints both version and API, and the nightly drift workflow records either kind of change.
