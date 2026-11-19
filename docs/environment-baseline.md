# Environment baseline

Captured before retiring the legacy firmware submodules. One row per channel.

| Channel | Submodule commit (gitlink in `HEAD`) | API version | Released `.fap` | `sha256sum` of release artifact |
|---|---|---|---|---|
| standard | `9e810dc399e027e48ce07751df72168abbb19065` | 88.2 | `flipp_pomodoro_standard_87.1.fap` | `71a46b898a7501a3b1371fcfb816a83a364eef3f380633342d3bcd5e1920dd58` |
| unleashed | `9c6327764c18fc8376b5941aa8bde79fe681fe8f` | 88.26 | `flipp_pomodoro_unleashed_88.9.fap` | `a8dea4978c00d335306b60ebad5b76196a9e477f4a8fc455da775d699748d5df` |

The API version comes from `targets/f7/api_symbols.csv` in the submodule commit recorded by the superproject (`git ls-tree HEAD .<channel>-firmware`), not from a build. The numbers therefore stay available after the submodules are removed.

The checked-out submodule working trees are stale: they sit at `8622f1a` and `7ccce3e`, both reporting API 87.1. Their `HEAD` is not the recorded gitlink, so reading it would understate the baseline.

The release columns come from the latest published release (`v2.5.15`) metadata; no artifact was downloaded. Their API suffixes differ from the current submodule commits because that release predates the latest submodule bumps.

## Container parity

| Channel | Baseline API | ufbt API | Delta | Artifact |
|---|---|---|---|---|
| standard | 88.2 | 88.2 | 0 | `dist/flipp_pomodoro_standard_88.2.fap` (32,976 bytes) |
