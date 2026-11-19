# Environment baseline

Captured before retiring the legacy firmware submodules. One row per channel.

| Channel | Submodule commit (gitlink in `HEAD`) | API version | Released `.fap` | `sha256sum` of release artifact |
|---|---|---|---|---|
| standard | `9e810dc399e027e48ce07751df72168abbb19065` | 88.2 | `flipp_pomodoro_standard_87.1.fap` | `71a46b898a7501a3b1371fcfb816a83a364eef3f380633342d3bcd5e1920dd58` |
| unleashed | `9c6327764c18fc8376b5941aa8bde79fe681fe8f` | 88.26 | `flipp_pomodoro_unleashed_88.9.fap` | `a8dea4978c00d335306b60ebad5b76196a9e477f4a8fc455da775d699748d5df` |

The API version comes from `targets/f7/api_symbols.csv` in the submodule commit recorded by the superproject (`git ls-tree HEAD .<channel>-firmware`), not from a build. The numbers therefore stay available after the submodules are removed.

The checked-out submodule working trees are stale: they sit at `8622f1a` and `7ccce3e`, both reporting API 87.1. Their `HEAD` is not the recorded gitlink, so reading it would understate the baseline.

The release columns come from the latest published release (`v2.5.15`) metadata; no artifact was downloaded. Their API suffixes differ from the current submodule commits because that release predates the latest submodule bumps.

## SDK baseline

The machine-readable source of truth for drift detection is `tools/dev/sdk-baseline.json`. This document is human-facing background only; no workflow parses it. Each channel in that JSON records:

- `baseline_api`: the immutable API captured from the retired gitlink above.
- `firmware_version` and `api`: the last release/API pair observed by `tools/dev/check-sdk-drift.sh`.

Artifact suffixes are the **firmware version** from the deployed SDK (`~/.ufbt/<channel>/current/ufbt_state.json`), not the API version. The API is recorded alongside it so a firmware release that changes only the API is still detected.

The standard SDK is the `release` channel of `https://update.flipperzero.one/firmware/directory.json`; version 1.5.1 is the current stable release there (the development build `a8a24de3` and RC `1.5.1-rc` are deliberately excluded). The Unleashed SDK comes from `https://up.unleashedflip.com/directory.json`, version `unlshd-095`.

Refreshing the Unleashed SDK with `just sdk-unleashed` and rebuilding leaves the artifact byte-for-byte unchanged (`sha256 0eb7148d2637ff760e3b1db69511dc47ef3c0020bd2a01d74ce416ec66d4d771`). Artifact sizes are intentionally not recorded in the baseline because `dist/` is regenerated on every run.
