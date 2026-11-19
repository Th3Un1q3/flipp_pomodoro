# Agentic development environment

The repository builds inside a Dev Container that carries the C toolchain, ufbt, both Flipper SDK channels, and OpenCode.

## What the image provides

| Layer | Contents |
|---|---|
| Base | `mcr.microsoft.com/devcontainers/base:noble`, user `vscode` |
| OS packages | `build-essential`, `file`, `jq`, `libusb-1.0-0`, `pipx`, `python3`, `unzip`, `xz-utils` |
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

`UFBT_HOME` defaults to the standard directory, so a bare `ufbt` builds the standard channel. `tools/dev/build.sh` sets the directory per channel.

Updating the Unleashed directory without `--index-url` replaces it with the official SDK. Use `just sdk-unleashed`, which passes the URL.

## Commands

| Command | Result |
|---|---|
| `just verify` | tool, SDK, and clean-build gate |
| `just build` | standard FAP in `dist/` |
| `just build unleashed` | Unleashed FAP in `dist/` |
| `just build-all` | both channels |
| `just sdk-unleashed` | download the Unleashed SDK only |

The agent loads `AGENTS.md` for repository rules and `.agents/skills/flipper-fap-build/` for the build procedure.

## Limits

- The container builds; it does not flash. `ufbt flash` and `ufbt launch` need `/dev/ttyACM*` from the host, which a Dev Container does not receive by default.
- The Unleashed SDK is a third-party index. Its API can lag the submodule the project used before; the build prints the API version it linked against.
